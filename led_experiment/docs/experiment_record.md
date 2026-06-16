# LED 控制链路实验记录

## 实验目标

使用 Git 管理 Atlas 平台 LED 控制链路相关代码修改，并验证 LED 从硬件到用户态的系统路径：

```text
LED 硬件 -> GPIO 控制器 -> LED 驱动 -> Linux LED 子系统 -> /sys/class/leds -> 用户态控制命令
```

## 修改内容

1. 核查 LED 对应 GPIO 编号、引脚复用和有效电平。
2. 分析 `probe` 和 `brightness_set` 控制路径。
3. 检查 LED 类设备注册及 sysfs 节点建立。
4. 编写用户态亮灭与闪烁验证脚本。
5. 给出重启或重新加载后的稳定性验收思路。

## Git 记录方式

```bash
git log --oneline --decorate --graph
git diff
git status
```

## 系统验证结果记录

真实实验时可填写：

| 验证项 | 命令 | 结果 |
|---|---|---|
| 驱动日志 | `dmesg | grep -i led` | 待填写 |
| LED 节点 | `ls /sys/class/leds` | 待填写 |
| 亮灭控制 | `echo 1 > brightness` / `echo 0 > brightness` | 待填写 |
| 触发机制 | `cat trigger` / `echo timer > trigger` | 待填写 |
| 闪烁控制 | `delay_on` / `delay_off` | 待填写 |
| 稳定性 | 重启或重新加载后再次验证 | 待填写 |

## 异常定位

- 无 LED 日志：检查设备树、compatible、GPIO 描述和驱动匹配关系。
- 无 `/sys/class/leds` 节点：检查 LED 类设备注册是否成功。
- 节点存在但 LED 不亮：检查 GPIO 方向、有效电平定义和硬件连接。
- 亮灭方向相反：检查 `active-low` 属性和驱动电平解释。
- 亮灭正常但 `trigger` 无效：检查 LED 触发器支持情况。
