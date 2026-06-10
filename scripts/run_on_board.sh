#!/usr/bin/env bash
set -euo pipefail

RTC_DEV="${1:-/dev/rtc0}"

echo "[1/5] build user app"
make

echo "[2/5] list RTC devices"
ls -l /dev/rtc* 2>/dev/null || true

echo "[3/5] read RTC by user app"
./rtc_check "$RTC_DEV"

echo "[4/5] read RTC by system tool if available"
if command -v hwclock >/dev/null 2>&1; then
    hwclock -r -f "$RTC_DEV" || true
else
    echo "hwclock not found, skipped"
fi

echo "[5/5] check kernel log"
dmesg | tail -80 | grep -iE 'rtc|rv8803|atlas-rtc-demo' || true

