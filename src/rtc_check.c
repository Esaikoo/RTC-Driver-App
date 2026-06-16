#include <errno.h>
#include <fcntl.h>
#include <linux/rtc.h>
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
};

static void print_usage(const char *prog)
{
    printf("%s %s\n", ATLAS_RTC_DEMO_NAME, ATLAS_RTC_DEMO_VERSION);
    printf("Usage: %s [--device /dev/rtcN] [--compare] [--proc]\n", prog);
    printf("\n");
    printf("Options:\n");
    printf("  -d, --device PATH   RTC device path, default /dev/rtc0\n");
    printf("      --compare       Compare RTC time with current system time\n");
    printf("      --proc          Print /proc/driver/rtc summary if available\n");
    printf("  -v, --version       Print project version\n");
    printf("  -h, --help          Show this help message\n");
}

static int parse_args(int argc, char **argv, struct options *opts)
{
    int i;

    opts->device = "/dev/rtc0";
    opts->show_proc = 0;
    opts->compare_system_time = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--device") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "%s requires a path\n", argv[i]);
                return -1;
            }
            opts->device = argv[++i];
        } else if (strcmp(argv[i], "--compare") == 0) {
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

static void compare_with_system_time(const struct rtc_time *rtc_tm)
{
    struct tm copy;
    time_t rtc_epoch;
    time_t sys_epoch;
    long delta;

    memset(&copy, 0, sizeof(copy));
    copy.tm_sec = rtc_tm->tm_sec;
    copy.tm_min = rtc_tm->tm_min;
    copy.tm_hour = rtc_tm->tm_hour;
    copy.tm_mday = rtc_tm->tm_mday;
    copy.tm_mon = rtc_tm->tm_mon;
    copy.tm_year = rtc_tm->tm_year;
    copy.tm_isdst = -1;

    rtc_epoch = mktime(&copy);
    sys_epoch = time(NULL);
    if (rtc_epoch == (time_t)-1 || sys_epoch == (time_t)-1) {
        printf("System compare  : unavailable\n");
        return;
    }

    delta = (long)difftime(sys_epoch, rtc_epoch);
    printf("System time     : %ld\n", (long)sys_epoch);
    printf("RTC/System diff : %ld seconds\n", delta);
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

int main(int argc, char **argv)
{
    struct options opts;
    struct rtc_time tm;

    if (parse_args(argc, argv, &opts) < 0) {
        print_usage(argv[0]);
        return 2;
    }

    if (read_rtc_time(opts.device, &tm) < 0) {
        return 1;
    }

    print_rtc_time(opts.device, &tm);

    if (opts.compare_system_time) {
        compare_with_system_time(&tm);
    }

    if (opts.show_proc) {
        print_proc_driver_rtc();
    }

    return 0;
}
