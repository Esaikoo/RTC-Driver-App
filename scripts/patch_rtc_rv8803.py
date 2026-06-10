#!/usr/bin/env python3
import shutil
import sys
from pathlib import Path

MARK = "atlas-rtc-demo"


def insert_in_function(text: str, signature: str, line: str) -> str:
    if line.strip() in text:
        return text

    pos = text.find(signature)
    if pos < 0:
        raise RuntimeError(f"cannot find function: {signature!r}")

    brace = text.find("{", pos)
    if brace < 0:
        raise RuntimeError(f"cannot find function body: {signature!r}")

    line_end = text.find("\n", brace)
    if line_end < 0:
        raise RuntimeError(f"cannot find function body line end: {signature!r}")

    return text[: line_end + 1] + line + text[line_end + 1 :]


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: patch_rtc_rv8803.py /path/to/Ascend310B-source")
        return 2

    source_root = Path(sys.argv[1]).resolve()
    driver = source_root / "drivers" / "rtc" / "rtc-rv8803.c"
    if not driver.exists():
        print(f"not found: {driver}")
        return 1

    text = driver.read_text(encoding="utf-8", errors="ignore")
    if MARK in text:
        print(f"already patched: {driver}")
        return 0

    backup = driver.with_suffix(driver.suffix + ".atlas-demo.bak")
    if not backup.exists():
        shutil.copy2(driver, backup)

    text = insert_in_function(
        text,
        "static int rv8803_get_time(struct device *dev, struct rtc_time *tm)",
        '\tdev_info(dev, "[atlas-rtc-demo] read_time called\\n");\n',
    )

    set_sig = "static int rv8803_set_time(struct device *dev, struct rtc_time *tm)"
    if set_sig in text:
        text = insert_in_function(
            text,
            set_sig,
            '\tdev_info(dev, "[atlas-rtc-demo] set_time called\\n");\n',
        )

    probe_sig = "static int rv8803_probe("
    if probe_sig in text:
        text = insert_in_function(
            text,
            probe_sig,
            '\tdev_info(&client->dev, "[atlas-rtc-demo] rv8803 probe: compatible rtc device detected\\n");\n',
        )

    driver.write_text(text, encoding="utf-8")
    print(f"patched: {driver}")
    print(f"backup : {backup}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
