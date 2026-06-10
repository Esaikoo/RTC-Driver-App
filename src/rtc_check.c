#include <errno.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    const char *dev = "/dev/rtc0";
    struct rtc_time tm;
    int fd;

    if (argc > 1) {
        dev = argv[1];
    }

    fd = open(dev, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open %s failed: %s\n", dev, strerror(errno));
        return 1;
    }

    memset(&tm, 0, sizeof(tm));
    if (ioctl(fd, RTC_RD_TIME, &tm) < 0) {
        fprintf(stderr, "ioctl RTC_RD_TIME failed: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    printf("RTC device: %s\n", dev);
    printf("RTC time: %04d-%02d-%02d %02d:%02d:%02d\n",
           tm.tm_year + 1900,
           tm.tm_mon + 1,
           tm.tm_mday,
           tm.tm_hour,
           tm.tm_min,
           tm.tm_sec);

    close(fd);
    return 0;
}

