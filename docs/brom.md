# BORM

[Allwinner H3](http://linux-sunxi.org/H3)-[BORM](http://linux-sunxi.org/BROM)

上电后，SoC 从一个集成的不可更换的 32KiB ROM （Boot ROM 或 BROM）芯片启动，这可以被认为是 PPL(primary program-loader).
SoC 从 `0xffff0000` 开始执行指令。

BROM 分为两部分：第一部分是 [FEL 模式](fel.md)(位于 `0xffff0000`),第二部分是 eGON.BRM(位于 `0xffff4000`).

## eGON

### 复位向量

复位向量位于 FEL 模式最开始的位置：在地址为 `0xffff0000`,复位时，它跳转到 `0xffff0028`，在其中将 `0xffff4000`（eGON.BRM）加载到程序计数器中，然后再执行。

### eGON 启动

该 eGON 引导 ROM 执行一些任务：

1. 执行一些协处理器设置（c15，（虚拟）系统控制协处理器）。
1. 禁用看门狗定时器
1. 设置 CPU，AXI，AHB 和 APB0 时钟
1. 启用 AHB 门控
1. 启用 APB0 门控
1. 将堆栈指针设置为 32K
1. 然后跳转到“启动”这立即跳转到 check_uboot
1. check_uboot 设置一些寄存器，然后检查状态引脚（通常称为 FEL 引脚，BSP 引脚或 uboot）
   1. 如果引脚为低电平（连接到 GND），则在 0xffff0020 处执行 FEL 模式。
   2. 如果该引脚为高电平，它将继续尝试从以下媒体引导，如果出现故障，则继续按顺序继续下一个引导。
      1. SD 卡 0 也称为 MMC0
      2. 内部 NAND 闪存也称为 NAND
      3. SD 卡 2 也称为 MMC2
      4. SPI 连接的 NOR 闪存也称为 SPI
      5. 如果全部失败，则从 0xffff0020 执行 FEL / USB 引导模式

可以看出，A10/A20 有几种启动方式，进入 FEL 模式之前，很多事情都需要出错或"失败"
如果 NAND 闪存中有有效的标头，则这一点尤其重要。显然，这可能会被破坏，因为它会破坏标头并因此导致失败。如果没有其他引导选项可用，则 FEL 模式应为最终结果。

作为旁路机制，A10 具有所谓的引导选择引脚（BSP）。该引脚通常在内部由 50KΩ 电阻上拉。如果将引脚拉低至 GND，则 A10 将尝试启动进入 FEL 模式。否则，将尝试上述引导顺序。
