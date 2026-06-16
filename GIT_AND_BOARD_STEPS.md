# Git 上传与开发板运行步骤

## 1. 在电脑上创建并上传仓库

进入项目目录：

```bash
cd atlas-rtc-driver-demo
```

初始化 Git：

```bash
git init
git add .
git commit -m "Add Atlas RTC driver modification demo"
```

在 GitHub、Gitee 或学校 Git 平台创建一个空仓库，例如：

```text
https://gitee.com/你的用户名/atlas-rtc-driver-demo.git
```

绑定远端并推送：

```bash
git remote add origin https://gitee.com/你的用户名/atlas-rtc-driver-demo.git
git branch -M main
git push -u origin main
```

## 2. 在编译主机上修改 RTC 驱动

假设 Atlas 源码在：

```text
/opt/Ascend310B-source
```

拉取项目：

```bash
git clone https://gitee.com/你的用户名/atlas-rtc-driver-demo.git
cd atlas-rtc-driver-demo
```

给脚本加执行权限：

```bash
chmod +x scripts/*.sh scripts/*.py
```

打补丁：

```bash
sudo bash scripts/build_kernel_module_on_host.sh /opt/Ascend310B-source
```

进入源码目录编译 kernel：

```bash
cd /opt/Ascend310B-source
sudo bash build.sh kernel
```

在 menuconfig 中选择：

```text
Device Drivers
  -> Real Time Clock
    -> Micro Crystal RV8803, Epson RX8900
```

建议选成模块：

```text
M
```

编译成功后找到：

```text
/opt/Ascend310B-source/output/kernel_modules/rtc-rv8803.ko
```

## 3. 把模块放到开发板

可以用 `scp`：

```bash
scp /opt/Ascend310B-source/output/kernel_modules/rtc-rv8803.ko root@开发板IP:/run/
```

也可以用 WinSCP 上传到开发板：

```text
/run/rtc-rv8803.ko
```

## 4. 开发板拉取项目并运行

登录开发板：

```bash
ssh root@开发板IP
```

拉取项目：

```bash
cd ~
git clone https://gitee.com/你的用户名/atlas-rtc-driver-demo.git
cd atlas-rtc-driver-demo
chmod +x scripts/*.sh
```

加载修改后的驱动模块：

```bash
sudo bash scripts/load_module_on_board.sh /run/rtc-rv8803.ko
```

编译并运行用户态测试程序：

```bash
sudo bash scripts/run_on_board.sh /dev/rtc0
```

也可以直接运行 v1.5 用户态验证工具：

```bash
make
sudo ./rtc_check --device /dev/rtc0 --compare --proc
```

## 5. 验收截图建议

建议截图保留这些命令结果：

```bash
git log --oneline -3
ls -l /dev/rtc*
lsmod | grep -E 'rtc|rv8803'
./rtc_check /dev/rtc0
./rtc_check --device /dev/rtc0 --compare --proc
dmesg | grep atlas-rtc-demo
cat /proc/driver/rtc
```

## 6. 答辩说明

可以这样讲：

> 我使用 Git 管理 Atlas RTC 驱动修改项目。仓库中包含驱动补丁脚本、开发板端测试应用和实验说明。驱动修改点位于 Linux RTC 芯片驱动 `drivers/rtc/rtc-rv8803.c`，增加了带有 `[atlas-rtc-demo]` 标记的日志。开发板运行用户态程序读取 `/dev/rtc0` 后，会通过 RTC 子系统调用驱动的 `read_time`，此时 `dmesg` 中可以看到新增日志，说明修改后的 RTC 驱动路径已经被实际触发。
