from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import json
import os
import subprocess
import tempfile
import threading
import time


HOST = os.environ.get("ATLAS_HOST", "192.168.137.100")
USER = os.environ.get("ATLAS_USER", "root")
PASSWORD = os.environ.get("ATLAS_PASS", "")
LED_NAME = os.environ.get("ATLAS_LED", "mmc0::")
PORT = int(os.environ.get("LED_WEB_PORT", "8080"))
WEB_DIR = Path(__file__).resolve().parent

state_lock = threading.Lock()
blink_stop = threading.Event()
blink_thread = None
current_state = "idle"


def run_remote(command):
    if not PASSWORD:
        raise RuntimeError("ATLAS_PASS is not set")

    askpass = Path(tempfile.gettempdir()) / "atlas_led_askpass.cmd"
    askpass.write_text(f"@echo off\r\necho {PASSWORD}\r\n", encoding="ascii")
    env = os.environ.copy()
    env["SSH_ASKPASS"] = str(askpass)
    env["SSH_ASKPASS_REQUIRE"] = "force"
    env["DISPLAY"] = "none"

    ssh_command = [
        "ssh",
        "-o",
        "BatchMode=no",
        "-o",
        "ConnectTimeout=5",
        "-o",
        "StrictHostKeyChecking=accept-new",
        f"{USER}@{HOST}",
        command,
    ]
    try:
        completed = subprocess.run(
            ssh_command,
            env=env,
            text=True,
            capture_output=True,
            timeout=12,
            check=False,
        )
    finally:
        askpass.unlink(missing_ok=True)

    if completed.returncode != 0:
        detail = completed.stderr.strip() or completed.stdout.strip()
        raise RuntimeError(detail or "ssh command failed")
    return completed.stdout.strip()


def led_path():
    return f"/sys/class/leds/{LED_NAME}"


def set_led(value):
    path = led_path()
    return run_remote(
        f'P="{path}"; echo none > "$P/trigger"; echo {value} > "$P/brightness"; cat "$P/brightness"'
    )


def set_state(state):
    global current_state
    with state_lock:
        current_state = state


def stop_blink():
    global blink_thread
    blink_stop.set()
    if blink_thread and blink_thread.is_alive():
        blink_thread.join(timeout=2)
    blink_thread = None
    blink_stop.clear()


def blink_loop():
    while not blink_stop.is_set():
        set_led(1)
        if blink_stop.wait(0.4):
            break
        set_led(0)
        blink_stop.wait(0.4)
    set_led(0)


def start_blink():
    global blink_thread
    stop_blink()
    blink_thread = threading.Thread(target=blink_loop, daemon=True)
    blink_thread.start()
    set_state("blink")


def read_status():
    path = led_path()
    output = run_remote(
        f'P="{path}"; test -d "$P" && printf "brightness=" && cat "$P/brightness"'
    )
    with state_lock:
        state = current_state
    return {"ok": True, "state": state, "output": output}


class LedHandler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(WEB_DIR), **kwargs)

    def send_json(self, status, payload):
        data = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if self.path == "/api/status":
            try:
                self.send_json(200, read_status())
            except Exception as error:
                self.send_json(500, {"ok": False, "state": "error", "error": str(error)})
            return
        super().do_GET()

    def do_POST(self):
        if self.path != "/api/led":
            self.send_json(404, {"ok": False, "error": "not found"})
            return

        length = int(self.headers.get("Content-Length", "0"))
        body = self.rfile.read(length) if length else b"{}"
        try:
            action = json.loads(body.decode("utf-8")).get("action")
            if action == "on":
                stop_blink()
                set_led(1)
                set_state("on")
            elif action == "off":
                stop_blink()
                set_led(0)
                set_state("off")
            elif action == "blink":
                start_blink()
            elif action == "stop":
                stop_blink()
                set_led(0)
                set_state("stop")
            else:
                raise ValueError("unknown action")
            with state_lock:
                state = current_state
            self.send_json(200, {"ok": True, "state": state})
        except Exception as error:
            self.send_json(500, {"ok": False, "state": "error", "error": str(error)})


def main():
    os.chdir(WEB_DIR)
    address = ("127.0.0.1", PORT)
    print(f"LED web panel: http://{address[0]}:{address[1]}")
    print(f"Atlas target: {USER}@{HOST}, LED={LED_NAME}")
    ThreadingHTTPServer(address, LedHandler).serve_forever()


if __name__ == "__main__":
    main()
