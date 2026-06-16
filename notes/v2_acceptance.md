# v2.0 RTC acceptance notes

## Goal

Version 2.0 turns the RTC demo from a single read test into a repeatable
acceptance workflow. The board-side program now records multiple RTC samples,
compares them with Linux system time, checks a configurable drift threshold,
and can emit JSON lines for Git-tracked experiment evidence.

## Suggested collaborative workflow

1. The repository owner creates the initial GitHub repository and pushes v1.x.
2. A collaborator creates a branch such as `feature/rtc-v2-evidence`.
3. The collaborator modifies `rtc_check`, README, changelog, and acceptance notes.
4. The Atlas board clones the repository and runs the v2.0 command set.
5. The generated `rtc_evidence_v2.jsonl` and terminal screenshots are attached to
   the course report or committed under a dated experiment folder.
6. The branch is merged after another teammate reviews the command output.

## Board commands

```bash
git clone https://github.com/Esaikoo/RTC-Driver-App.git
cd RTC-Driver-App
make
sudo bash scripts/run_on_board.sh /dev/rtc0
```

Manual evidence collection:

```bash
sudo ./rtc_check --device /dev/rtc0 --compare --max-drift 300 --samples 3 --interval 1 --proc
sudo ./rtc_check --device /dev/rtc0 --compare --max-drift 300 --samples 3 --interval 1 --json | tee rtc_evidence_v2.jsonl
dmesg | tail -80 | grep -iE 'rtc|rv8803|atlas-rtc-demo'
```

## Acceptance criteria

- `/dev/rtc0` exists and can be opened by the user-space program.
- `RTC_RD_TIME` returns a valid time.
- The user-space time is close to system time within the configured threshold.
- Kernel logs show the modified `rtc-rv8803.c` path when the custom module is used.
- JSON evidence can be preserved and reviewed by other team members.
