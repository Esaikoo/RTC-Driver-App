#include <errno.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include "project_version.h"

struct options {
    const char *device;
    int show_proc;
    int compare_system_time;
    int json_output;
    int samples;
    unsigned int interval_sec;
    long max_drift_sec;
};

static void print_usage(const char *prog)
{
    printf("%s %s\n", ATLAS_RTC_DEMO_NAME, ATLAS_RTC_DEMO_VERSION);
    printf("Usage: %s [--device /dev/rtcN] [--compare] [--proc] [--json]\n", prog);
    printf("\n");
    printf("Options:\n");
    printf("  -d, --device PATH      RTC device path, default /dev/rtc0\n");
    printf("      --compare          Compare RTC time with current system time\n");
    printf("      --max-drift SEC    Fail when absolute RTC/System drift exceeds SEC\n");
    printf("      --samples N        Read RTC N times, default 1\n");
    printf("      --interval SEC     Delay between samples, default 1\n");
    printf("      --json             Print machine-readable JSON evidence\n");
    printf("      --proc             Print /proc/driver/rtc summary if available\n");
    printf("  -v, --version          Print project version\n");
    printf("  -h, --help             Show this help message\n");
}

static int parse_positive_int(const char *text, int *value)
{
    char *end = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno || end == text || *end != '\0' || parsed < 1 || parsed > INT_MAX) {
        return -1;
    }

    *value = (int)parsed;
    return 0;
}

static int parse_non_negative_long(const char *text, long *value)
{
    char *end = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno || end == text || *end != '\0' || parsed < 0) {
        return -1;
    }

    *value = parsed;
    return 0;
}

static int parse_args(int argc, char **argv, struct options *opts)
{
    int i;

    opts->device = "/dev/rtc0";
    opts->show_proc = 0;
    opts->compare_system_time = 0;
    opts->json_output = 0;
    opts->samples = 1;
    opts->interval_sec = 1;
    opts->max_drift_sec = -1;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--device") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "%s requires a path\n", argv[i]);
                return -1;
            }
            opts->device = argv[++i];
        } else if (strcmp(argv[i], "--compare") == 0) {
            opts->compare_system_time = 1;
        } else if (strcmp(argv[i], "--json") == 0) {
            opts->json_output = 1;
        } else if (strcmp(argv[i], "--samples") == 0) {
            if (i + 1 >= argc || parse_positive_int(argv[++i], &opts->samples) < 0) {
                fprintf(stderr, "--samples requires a positive integer\n");
                return -1;
            }
        } else if (strcmp(argv[i], "--interval") == 0) {
            int parsed_interval;

            if (i + 1 >= argc || parse_positive_int(argv[++i], &parsed_interval) < 0) {
                fprintf(stderr, "--interval requires a positive integer\n");
                return -1;
            }
            opts->interval_sec = (unsigned int)parsed_interval;
        } else if (strcmp(argv[i], "--max-drift") == 0) {
            if (i + 1 >= argc || parse_non_negative_long(argv[++i], &opts->max_drift_sec) < 0) {
                fprintf(stderr, "--max-drift requires a non-negative integer\n");
                return -1;
            }
            opts->compare_system_time = 1;
        } else if (strcmp(argv[i], "--proc") == 0) {
            opts->show_proc = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("%s %s\n", ATLAS_RTC_DEMO_NAME, ATLAS_RTC_DEMO_VERSION);
            exit(0);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            exit(0);
        } else {
            fprintf(stderr, "unknown option: %s\n", argv[i]);
            return -1;
        }
    }

    return 0;
}

static time_t rtc_time_to_epoch(const struct rtc_time *rtc_tm)
{
    struct tm copy;

    memset(&copy, 0, sizeof(copy));
    copy.tm_sec = rtc_tm->tm_sec;
    copy.tm_min = rtc_tm->tm_min;
    copy.tm_hour = rtc_tm->tm_hour;
    copy.tm_mday = rtc_tm->tm_mday;
    copy.tm_mon = rtc_tm->tm_mon;
    copy.tm_year = rtc_tm->tm_year;
    copy.tm_isdst = -1;

    return mktime(&copy);
}

static int read_rtc_time(const char *dev, struct rtc_time *tm)
{
    int fd;

    fd = open(dev, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open %s failed: %s\n", dev, strerror(errno));
        return -1;
    }

    memset(tm, 0, sizeof(*tm));
    if (ioctl(fd, RTC_RD_TIME, tm) < 0) {
        fprintf(stderr, "ioctl RTC_RD_TIME failed: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static void print_rtc_time(const char *dev, const struct rtc_time *tm)
{
    printf("RTC demo version: %s\n", ATLAS_RTC_DEMO_VERSION);
    printf("RTC device      : %s\n", dev);
    printf("RTC time        : %04d-%02d-%02d %02d:%02d:%02d\n",
           tm->tm_year + 1900,
           tm->tm_mon + 1,
           tm->tm_mday,
           tm->tm_hour,
           tm->tm_min,
           tm->tm_sec);
}

static long absolute_long(long value)
{
    return value < 0 ? -value : value;
}

static int compare_with_system_time(const struct rtc_time *rtc_tm, long max_drift_sec, long *delta_out)
{
    time_t rtc_epoch = rtc_time_to_epoch(rtc_tm);
    time_t sys_epoch = time(NULL);
    long delta;

    if (rtc_epoch == (time_t)-1 || sys_epoch == (time_t)-1) {
        printf("System compare  : unavailable\n");
        return 0;
    }

    delta = (long)difftime(sys_epoch, rtc_epoch);
    if (delta_out) {
        *delta_out = delta;
    }

    printf("System time     : %ld\n", (long)sys_epoch);
    printf("RTC/System diff : %ld seconds\n", delta);
    if (max_drift_sec >= 0) {
        printf("Drift threshold : %ld seconds\n", max_drift_sec);
        printf("Drift status    : %s\n",
               absolute_long(delta) <= max_drift_sec ? "PASS" : "FAIL");
    }

    return max_drift_sec >= 0 && absolute_long(delta) > max_drift_sec ? -1 : 0;
}

static void print_proc_driver_rtc(void)
{
    FILE *fp;
    char line[256];
    int count = 0;

    fp = fopen("/proc/driver/rtc", "r");
    if (!fp) {
        printf("/proc/driver/rtc: unavailable: %s\n", strerror(errno));
        return;
    }

    printf("/proc/driver/rtc summary:\n");
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "rtc_time") ||
            strstr(line, "rtc_date") ||
            strstr(line, "alrm_time") ||
            strstr(line, "alrm_date") ||
            strstr(line, "alarm_IRQ") ||
            strstr(line, "alrm_pending")) {
            printf("  %s", line);
            count++;
        }
    }

    if (count == 0) {
        printf("  no common RTC summary fields found\n");
    }

    fclose(fp);
}

static int print_json_sample(int index, const char *dev, const struct rtc_time *tm,
                             int compare_system_time, long max_drift_sec)
{
    time_t rtc_epoch = rtc_time_to_epoch(tm);
    time_t sys_epoch = compare_system_time ? time(NULL) : (time_t)-1;
    long delta = 0;
    const char *status = "unchecked";
    int failed = 0;

    if (compare_system_time && rtc_epoch != (time_t)-1 && sys_epoch != (time_t)-1) {
        delta = (long)difftime(sys_epoch, rtc_epoch);
        failed = max_drift_sec >= 0 && absolute_long(delta) > max_drift_sec;
        status = failed ? "fail" : "pass";
    }

    printf("{\"sample\":%d,\"version\":\"%s\",\"device\":\"%s\","
           "\"rtc_time\":\"%04d-%02d-%02d %02d:%02d:%02d\","
           "\"rtc_epoch\":%ld",
           index,
           ATLAS_RTC_DEMO_VERSION,
           dev,
           tm->tm_year + 1900,
           tm->tm_mon + 1,
           tm->tm_mday,
           tm->tm_hour,
           tm->tm_min,
           tm->tm_sec,
           (long)rtc_epoch);

    if (compare_system_time) {
        printf(",\"system_epoch\":%ld,\"drift_seconds\":%ld,"
               "\"max_drift_seconds\":%ld,\"status\":\"%s\"",
               (long)sys_epoch,
               delta,
               max_drift_sec,
               status);
    }

    printf("}\n");
    return failed ? -1 : 0;
}

int main(int argc, char **argv)
{
    struct options opts;
    struct rtc_time tm;
    int i;
    int failed = 0;

    if (parse_args(argc, argv, &opts) < 0) {
        print_usage(argv[0]);
        return 2;
    }

    for (i = 0; i < opts.samples; i++) {
        long delta = 0;

        if (read_rtc_time(opts.device, &tm) < 0) {
            return 1;
        }

        if (opts.json_output) {
            if (print_json_sample(i + 1, opts.device, &tm,
                                  opts.compare_system_time, opts.max_drift_sec) < 0) {
                failed = 1;
            }
        } else {
            if (opts.samples > 1) {
                printf("Sample %d/%d\n", i + 1, opts.samples);
            }
            print_rtc_time(opts.device, &tm);

            if (opts.compare_system_time &&
                compare_with_system_time(&tm, opts.max_drift_sec, &delta) < 0) {
                failed = 1;
            }
        }

        if (i + 1 < opts.samples) {
            sleep(opts.interval_sec);
        }
    }

    if (opts.show_proc && !opts.json_output) {
        print_proc_driver_rtc();
    }

    return failed ? 3 : 0;
}
