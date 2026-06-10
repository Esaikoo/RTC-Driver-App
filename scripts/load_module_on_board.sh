#!/usr/bin/env bash
set -euo pipefail

KO_PATH="${1:-/run/rtc-rv8803.ko}"

if [ ! -f "$KO_PATH" ]; then
    echo "module not found: $KO_PATH"
    exit 1
fi

echo "[1/4] current RTC devices"
ls -l /dev/rtc* 2>/dev/null || true

echo "[2/4] unload old module if possible"
if lsmod | grep -q '^rtc_rv8803'; then
    rmmod rtc_rv8803 || true
fi
if lsmod | grep -q '^rtc-rv8803'; then
    rmmod rtc-rv8803 || true
fi

echo "[3/4] load new module"
insmod "$KO_PATH"

echo "[4/4] verify"
lsmod | grep -E 'rtc|rv8803' || true
dmesg | tail -50 | grep -iE 'rtc|rv8803|atlas-rtc-demo' || true
ls -l /dev/rtc* 2>/dev/null || true

