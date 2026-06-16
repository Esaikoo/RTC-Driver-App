#!/usr/bin/env bash
set -euo pipefail

LED_NAME="${1:-led0}"
LED_PATH="/sys/class/leds/${LED_NAME}"

echo timer > "${LED_PATH}/trigger"
echo 500 > "${LED_PATH}/delay_on"
echo 500 > "${LED_PATH}/delay_off"
sleep 5
echo none > "${LED_PATH}/trigger"
echo 0 > "${LED_PATH}/brightness"
