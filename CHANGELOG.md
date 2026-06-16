# Changelog

## v1.5 - RTC system evidence version

- Add project version header `include/project_version.h`.
- Upgrade `rtc_check` from a minimal reader to a board-side verification tool.
- Add `--device`, `--compare`, `--proc`, `--version`, and `--help` options.
- Update board run script to collect RTC time, system-time comparison, `/proc/driver/rtc`, and kernel logs in one run.
- This version better supports course acceptance because it gives observable evidence across user space, RTC subsystem, and kernel log.

## v1.4 - Report and acceptance notes

- Add experiment report and Git/board operation steps.
- Organize acceptance commands: `dmesg`, `lsmod`, `hwclock`, `/proc/driver/rtc`.

## v1.3 - Board scripts

- Add scripts for loading `rtc-rv8803.ko` on the Atlas board.
- Add script for building and running the board-side user program.

## v1.2 - Driver patch script

- Add `patch_rtc_rv8803.py` to insert `[atlas-rtc-demo]` logs into `rtc-rv8803.c`.
- Add host-side helper script for patching the Atlas source tree.

## v1.1 - User-space RTC read test

- Add `src/rtc_check.c`.
- Read `/dev/rtc0` with `ioctl(RTC_RD_TIME)`.

## v1.0 - Initial repository

- Create repository structure.
- Add README and experiment background.
