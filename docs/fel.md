# FEL

FEL 是 Allwinner 设备上的 BORM 中包含的低级子程序。它用于使用 USB 进行设备的初始编程和恢复。

> 因此，您的设备需要通过 USB 电缆连接到 "host"（您的 PC），并连接到 sunxi 设备，sunxi 设备将在该端口上将自己显示为 USB "slave"(即在设备模式下).

## 进入 EFL 模式

首先需要完全关闭设备，断电，如果有任何外部的连接线建议全部重新连接。

### 通过 FEL 按钮

如果有 FEL 按钮，也被称为`recovery`,`uboot`,`fel`。按住按钮，再开机。

### 通过按住标准按钮

这通常是标准的数位板按钮之一，例如 VOL +键或其他按钮。
以下似乎有效：

1. 按住可能被用作 FEL 的按钮。
2. 按住电源键 2 秒
3. 放开电源键，立即按至少 3 下电源键。

> boot1 使用这种模式

### 通过串口

如果能够连接 UART，可以在上电后发送字符 `1`（个别设备为`2`）到串口。

或者部分 uboot 支持 `efex` 命令，如果没有该命令，可以试试直接使用 `go` 命令和 FEL 地址。

```sh
=> go 0xffff0020
```

### 通过特殊的 SD 卡镜像

这个镜像仅是为了跳转至 FEL 模式

```sh
wget https://github.com/linux-sunxi/sunxi-tools/raw/master/bin/fel-sdboot.sunxi
dd if=fel-sdboot.sunxi of=/dev/sdX bs=1024 seek=8
```

### 没有有效的启动镜像

如果 BORM 没有有效的启动镜像，会自动进入 FEL 模式。如果没有板载 NAND 或者 eMMC 的板，只需要取出 SD 卡即可。

## 验证 FEL 模式

### 会出现一个新的 USB 设备

如果运行 lsusb

```sh
Bus 001 Device 074: ID 1f3a:efe8
```

### 运行 sunxi-fel 工具

```sh
> ./sunxi-fel version
AWUSBFEX soc=00162500(A13) 00000001 ver=0001 44 08 scratchpad=00007e00 00000000 00000000
Serial output
```

### 串口输出

如果您选择的方法初始化了 boot1，那么您应该会在串行上看到类似以下内容的内容：

```sh
HELLO! BOOT0 is starting!
boot0 version : .3.0
dram size =1024
Succeed in opening nand flash.
Succeed in reading Boot1 file head.
The size of Boot1 is 0x00036000.
The file stored in 0X00000000 of block 2 is perfect.
Check is correct.
Ready to disable icache.
Succeed in loading Boot1.
Jump to Boot1.
[       0.145] boot1 version : 1.3.1a
[       0.145] pmu type = 3
[       0.145] bat vol = 4117
[       0.176] axi:ahb:apb=3:2:2
[       0.176] set dcdc2=1400, clock=1008 successed
[       0.178] key
[       2.486] you can unclench the key to update now
[       2.486] key found, jump to fel
```

## 从 FEL 模式启动

[FEL/USBBoot](http://linux-sunxi.org/FEL/USBBoot)

编译 uboot

```sh
git clone -b next git://git.denx.de/u-boot-sunxi.git
cd u-boot-sunxi
make CROSS_COMPILE=aarch64-linux-gnu- <your-board>_defconfig
make CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc) BL31=/path/to/trusted-firmware/build/sun50i-a64/debug/bl31.bin SCP=/dev/null
```

从 USB 启动 uboot

```sh
sunxi-fel uboot u-boot-sunxi-with-spl.bin
```

从 USB 启动整个系统

这需要与 U-Boot `v2015.10` 或更高版本一起使用（在被 sunxi-fel 上传到 RAM 之后，它可以在 RAM 中自动找到"boot.scr" blob）。

```sh
sunxi-fel -v uboot u-boot-sunxi-with-spl.bin \
             write 0x42000000 uImage \
             write 0x43000000 sun7i-a20-cubietruck.dtb \
             write 0x43100000 boot.scr \
             write 0x43300000 rootfs.cpio.lzma.uboot
```

64bits SoC

```sh
sunxi-fel -v uboot u-boot-sunxi-with-spl.bin \
             write 0x40200000 Image \
             write 0x4fa00000 sun50i-a64-pine64-lts.dtb \
             write 0x4fc00000 boot.scr \
             write 0x4ff00000 rootfs.cpio.lzma.uboot

```

这会通过 USB 启动 U-Boot。在 U-Boot 取得控制权之后，它将在各种默认位置开始扫描 boot.scr 文件，以便引导系统的其余部分。

## sunxi-fel `uboot` 命令实现

在 FEL 模式下有两个堆栈：

- `sp_irq`是 IRQ 堆栈指针寄存器的值。IRQ 堆栈通常为空，这意味着`sp_irq`同时指向该堆栈的顶部和底部（除非处理器当前正在处理 IRQ)
- `sp`是常规堆栈指针寄存器的值，当通过`sunxi-fel exe`命令执行该代码时，该指针也可用于您的代码。该堆栈通常包含`sp`和 `0x7000` 之间的数据（由 BROM 的 FEL 代码写入该数据）

  通过`sunxi-fel write`命令上传数据时，请确保不要覆盖这些堆栈（请勿触碰`sp_irq`以下和`sp`以下的 SRAM 中的数据）

Allwinner 正在以特殊方式处理通过 USB 引导，不幸的是，此行为已硬编码在 BROM 中。从 MMC/NAND 引导和从 USB 引导之间的主要区别是：

- 为了从 SD/NAND 进行引导，BROM 代码正在可引导媒体(boot media)上搜索特殊的 `eGON` 签名，并将高达 32K（实际上比这还少一点）的初始代码加载到 SRAM 中的地址 `0x0`（通常是 SRAM A1 part），然后执行它。此初始代码（在 U-Boot 中称为`SPL`或在 Allwinner 的 Bootloader 中称为`boot0`）将配置(初始化) DRAM 以获得更多的可访问存储空间，然后将 Bootloader 的主要部分以及系统的其余部分加载到那里。
- 对于通过 FEL 进行引导，Allwinner 的想法是，我们应该仅在 SRAM 中的地址 `0x2000` 后上传最多约 15K 的代码，然后执行它。

这是不一致的，至少是不利的。uboot 需要特殊优化的，以用于从 FEL 引导，需要将启动地址从 `0x0` 更改到 `0x2000`，这带来了额外的配置。
由于代码大小的限制，意味着需要禁用许多功能。但是，由`sunxi-fel`工具实现的`spl`和`uboot`命令可以通过将 U-Boot SPL 的普通 MMC 或 NAND 变种移动(smuggling)到 SRAM 中来解决此限制。下面提供了更多技术细节。

下图说明了通过 FEL 引导时我们的代码大小限制为〜15K 的原因。当我们以 FEL 模式启动设备时，会在 BROM 中激活特殊代码，并开始使用 FEL 协议通过 USB 进行通信。
来自 BROM 的 USB 驱动程序代码在 SRAM 的前 32K 内相当不方便的位置分配了两个堆栈。
IRQ 处理程序堆栈设置在地址 0x2000 处并向下扩展。
普通的应用程序堆栈设置在地址 0x7000 处，并且也向下扩展。
这些堆栈使 SRAM 空间碎片化，并且最大可用的连续〜15K 区域夹在这两个堆栈之间。
通过`sunxi-fel write`覆盖这两个堆栈中的任何一个都会使设备崩溃，并且它停止响应其他 FEL 命令。

![FEL-uboot-memory-map](FEL-uboot-memory-map.dio.png)

那么，我们如何解决这个问题呢？Allwinner 设备通常具有超过 32K 的 SRAM（在 Allwinner A13 中，所有设备中最小的 SRAM 总量为 48K）。而且，我们可以将额外的 SRAM 位置用作 FEL 堆栈的备份存储（在上图中显示为“备份区域 1”和“备份区域 2”）。我们还上传了一个特殊的 thunk 代码，该代码负责在跳到地址 0x0 之前将 FEL 堆栈的内容与这些备份区域的内容交换。现在，为了从 U-Boot 执行完整的 SPL，我们只需要将 SPL 拆分为多个块并将其上传到 SRAM，将应该与 FEL 堆栈重叠的部分写入备份区域。执行重排代码会将 FEL 堆栈保存到备份区域，

为什么我们需要备份原始的 FEL 堆栈？原因是仅上载和执行 SPL 不足以引导系统。SPL 代码非常小，其主要任务是设置时钟并初始化 DRAM。初始化 DRAM 后，所有存储空间问题均已解决，我们希望将主 U-Boot 代码加载到设备中。为此，我们仍然需要激活 BROM FEL 代码并重新获得控制权，以便它仍可以通过 USB 与“ sunxi-fel”工具进行对话并执行 FEL 命令。因此，SPL 将控制权返回到 thunk 代码。重排代码再次将 FEL 堆栈与备份区域交换，最后将控制权交还给 BROM 中的 FEL 代码，该程序能够愉快地恢复其工作，因为它已将所有原始数据重新存储在堆栈中。
