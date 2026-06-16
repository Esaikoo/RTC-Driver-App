#!/usr/bin/env bash
set -euo pipefail

RTC_DEV="${1:-/dev/rtc0}"

echo "[1/5] build user app"
make

echo "[2/5] list RTC devices"
ls -l /dev/rtc* 2>/dev/null || true

echo "[3/5] read RTC by user app"
./rtc_check --device "$RTC_DEV" --compare --max-drift 300 --samples 3 --interval 1 --proc

echo "[3b/5] collect JSON evidence"
./rtc_check --device "$RTC_DEV" --compare --max-drift 300 --samples 3 --interval 1 --json

echo "[4/5] read RTC by system tool if available"
if command -v hwclock >/dev/null 2>&1; then
    hwclock -r -f "$RTC_DEV" || true
else
    echo "hwclock not found, skipped"
fi

echo "[5/5] check kernel log"
dmesg | tail -80 | grep -iE 'rtc|rv8803|atlas-rtc-demo' || true
