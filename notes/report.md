# 实验报告要点

## 题目

基于 Atlas 200I DK A2 的 Linux RTC 驱动修改与验证。

## 背景

RTC 是 Real-Time Clock，负责为系统提供掉电后仍可保持的硬件时间。Linux 中用户程序通过 `/dev/rtc0`、`ioctl(RTC_RD_TIME)` 等接口访问 RTC。调用会进入 Linux RTC 子系统，再由具体芯片驱动完成硬件访问。

Atlas 200I DK A2 文档中给出的 RTC 设备树节点为：

```dts
&i2c5_ao {
    rtc@32 {
        compatible = "epson,rx8900";
        reg = <0x32>;
    };
};
```

说明 RTC 芯片挂在 I2C5 总线上，设备地址为 `0x32`，驱动对应 `drivers/rtc/rtc-rv8803.c`。

## 修改内容

本实验在 `rtc-rv8803.c` 中增加调试日志：

- `probe`：驱动匹配到设备时打印日志。
- `read_time`：用户读取 RTC 时间时打印日志。
- `set_time`：用户设置 RTC 时间时打印日志。

日志统一带有：

```text
[atlas-rtc-demo]
```

便于通过 `dmesg` 验证修改是否生效。

## 验证流程

1. 在编译主机上修改驱动源码。
2. 使用 Atlas SDK 编译 kernel module。
3. 将 `rtc-rv8803.ko` 上传到开发板。
4. 在开发板上加载模块。
5. 运行用户态程序读取 `/dev/rtc0`。
6. 通过 `dmesg` 查看新增日志。

## 系统链路说明

用户态读取 `/dev/rtc0` 时，会经过 Linux RTC 子系统进入具体 RTC 驱动 `rtc-rv8803.c`。该驱动根据设备树中 `compatible = "epson,rx8900"` 和 I2C 地址 `0x32` 匹配 RTC 芯片，并通过 I2C 访问硬件时间寄存器。因此，本实验的验证重点是观察“用户态应用 -> RTC 子系统 -> RTC 驱动 -> I2C 硬件设备”这一链路是否被实际触发。

## 答辩说明

本实验采用 Git 管理驱动修改和测试程序。开发者在仓库中提交修改脚本、测试应用和实验说明。开发板通过 `git clone` 拉取项目后运行测试程序，能够读取 RTC 时间，并在内核日志中看到新增的 `[atlas-rtc-demo]` 日志，说明修改后的 RTC 驱动已经生效。
