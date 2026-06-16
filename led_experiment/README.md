# Atlas LED Git Demo

本仓库用于课程报告 `B. 代码版本控制与系统验证`，主题为：

> 使用 Git 管理 Atlas 平台 LED 控制链路相关代码修改，并从系统层验证 GPIO、LED 驱动、sysfs 节点和用户态控制命令。

本仓库是课程实验模板/示例工程，可用于展示实验设计、版本控制流程、代码修改点和验证步骤。若后续连接真实 Atlas 板，可据此替换为真实日志和截图。

## 仓库结构

```text
atlas-led-git-demo/
  driver/
    atlas_led_demo.c      # LED 驱动修改示例
    Makefile              # 内核模块构建示例
  scripts/
    led_validate.sh       # LED 系统链路验证脚本
    led_blink_demo.sh     # LED 闪烁验证脚本
    led_expected_output.txt
  docs/
    experiment_record.md  # 实验记录
    validation.md         # 验收步骤
  screenshots/
    README.md
```

## 实验主线

```text
LED 硬件 -> GPIO 控制器 -> LED 驱动 -> Linux LED 子系统 -> /sys/class/leds -> 用户态控制命令
```

主要验证点：

- `dmesg` 中能看到 LED 驱动匹配或注册信息。
- `/sys/class/leds` 下存在对应节点。
- `brightness` 写入 `0/1` 能控制 LED 熄灭和点亮。
- `trigger` 节点支持 `timer` 等模式。
- 重启或重新加载后，LED 节点能够恢复并继续控制。

## Git 使用

```bash
git log --oneline --decorate --graph
git status
git diff
```

建议保留四个阶段的提交：

1. 初始化 LED 实验仓库。
2. 增加 LED 驱动示例。
3. 增加用户态验证脚本。
4. 补充实验记录与验收说明。

## 注意

`driver/atlas_led_demo.c` 是教学用途的简化示例，不应直接替换真实内核源码。真实 Atlas 环境中应结合平台设备树、GPIO 编号、有效电平配置和已有 LED 驱动框架调整。
