# LED 前端控制面板

这是连接 Atlas 板子的本地 LED 控制面板。页面只保留四个操作按钮：

- 点亮
- 熄灭
- 闪烁
- 停止闪烁

后端通过 SSH 登录 Atlas 板子，并写入 `/sys/class/leds/mmc0::/brightness` 完成控制。真实板子的 LED trigger 列表中没有 `timer`，所以闪烁由后端周期性写入 `brightness` 实现。

## 运行方式

```powershell
$env:ATLAS_HOST="192.168.137.100"
$env:ATLAS_USER="root"
$env:ATLAS_PASS="你的板子密码"
$env:ATLAS_LED="mmc0::"
python server.py
```

然后打开：

```text
http://127.0.0.1:8080
```

如需改用其他 LED 节点，把 `ATLAS_LED` 改成 `/sys/class/leds` 下实际存在的名称。
