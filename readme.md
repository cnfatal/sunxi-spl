# sunxi-spl

全志H3 MCU，从0开始，手动启动并使能uart0。非uboot，用于研究学习硬件初始化逻辑。

[BROM](https://linux-sunxi.org/BROM)

## 从 SD 卡启动

为了被 BROM 识别，SPL 需要写入 SD 卡上的某个位置，并有一个带有正确校验和的特殊标头。可以使用 `mksunxiboot` 工具将此类特殊头文件添加到二进制文件中。

SPL 的大小必须是 [NAND 中 8 KiB 的倍数和 SD 卡上 512 字节的倍数](https://source.denx.de/u-boot/u-boot/commit/1f6f61fe4c7c9637e2c8b2960a08f106fbe01134)

对于 `Allwinner H3`,栈顶指针`sp=0x0F7DC`,SPL 加载地址为 `0x000000`,块大小最小应该为 MMC `512byte`或者 NAND`8kib`,为了兼容两者可以设置块大小为`0x2000`也就是`8192kib`,编译出来的 SPL,正好为 32kb 即可。如果超过了 32kb，则需要将 SPL 拆分为两部分，一部分为 32kb，一部分为其他，使用 32kb 部分初始化 DRAM 后，再重新加载 32kb 和其他部分，然后才可继续引导。BROM 拒绝大于 32 KiB 的大小。正好 32 KiB 就可以了，通过在 SPL 末尾写入特殊模式(pattern)并在 SRAM 中检查它来验证。
