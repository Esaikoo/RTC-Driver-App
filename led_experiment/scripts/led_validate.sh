#!/usr/bin/env bash
set -euo pipefail

LED_NAME="${1:-led0}"
LED_PATH="/sys/class/leds/${LED_NAME}"

echo "== LED driver log =="
dmesg | grep -i led || true

echo
echo "== LED class nodes =="
ls /sys/class/leds || true

echo
echo "== LED node check =="
if [ -d "${LED_PATH}" ]; then
  cat "${LED_PATH}/brightness" || true
  cat "${LED_PATH}/trigger" || true
else
  echo "LED path not found: ${LED_PATH}"
fi

echo
echo "== LED on/off test =="
echo 1 > "${LED_PATH}/brightness"
sleep 1
echo 0 > "${LED_PATH}/brightness"
cat "${LED_PATH}/brightness" || true
