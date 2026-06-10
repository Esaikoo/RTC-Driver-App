#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "usage: sudo bash scripts/build_kernel_module_on_host.sh /path/to/Ascend310B-source"
    exit 2
fi

SOURCE_ROOT="$1"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ ! -d "$SOURCE_ROOT" ]; then
    echo "source root not found: $SOURCE_ROOT"
    exit 1
fi

python3 "$SCRIPT_DIR/patch_rtc_rv8803.py" "$SOURCE_ROOT"

cat <<'EOF'

Patch finished.

Next step:
  cd <Ascend310B-source>
  bash build.sh kernel

In menuconfig, choose:
  Device Drivers
    -> Real Time Clock
      -> Micro Crystal RV8803, Epson RX8900

Recommended:
  CONFIG_RTC_DRV_RV8803=m

After build, check:
  output/kernel_modules/rtc-rv8803.ko

EOF

