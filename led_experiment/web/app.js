const stateText = document.querySelector("#stateText");
const buttons = [...document.querySelectorAll("[data-action]")];

const labels = {
  idle: "已连接",
  on: "已点亮",
  off: "已熄灭",
  blink: "闪烁中",
  stop: "已停止",
  error: "连接失败",
};

function setState(state) {
  stateText.textContent = labels[state] || state;
  buttons.forEach((button) => {
    button.classList.toggle("active", button.dataset.action === state);
  });
}

function setBusy(isBusy) {
  buttons.forEach((button) => {
    button.disabled = isBusy;
  });
}

async function requestLed(action) {
  setBusy(true);
  try {
    const response = await fetch("/api/led", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ action }),
    });
    const result = await response.json();
    if (!response.ok || !result.ok) {
      throw new Error(result.error || "request failed");
    }
    setState(result.state);
  } catch (error) {
    setState("error");
  } finally {
    setBusy(false);
  }
}

async function loadStatus() {
  try {
    const response = await fetch("/api/status");
    const result = await response.json();
    setState(result.ok ? result.state : "error");
  } catch (error) {
    setState("error");
  }
}

buttons.forEach((button) => {
  button.addEventListener("click", () => requestLed(button.dataset.action));
});

loadStatus();
