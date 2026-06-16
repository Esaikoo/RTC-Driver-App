# LED 系统层验收步骤

## 1. 驱动加载

```bash
dmesg | grep -i led
```

预期：看到 LED 设备匹配、注册或初始化信息。

## 2. 节点检查

```bash
ls /sys/class/leds
```

预期：存在目标 LED 节点。

## 3. 亮灭控制

```bash
cat /sys/class/leds/led0/brightness
echo 1 > /sys/class/leds/led0/brightness
echo 0 > /sys/class/leds/led0/brightness
```

预期：LED 可以点亮和熄灭。

## 4. 触发机制

```bash
cat /sys/class/leds/led0/trigger
echo timer > /sys/class/leds/led0/trigger
echo 500 > /sys/class/leds/led0/delay_on
echo 500 > /sys/class/leds/led0/delay_off
echo none > /sys/class/leds/led0/trigger
```

预期：LED 能按设定周期闪烁并恢复普通模式。

## 5. 稳定性验证

```bash
reboot
ls /sys/class/leds
```

预期：重启后节点重新建立，控制仍然有效。
