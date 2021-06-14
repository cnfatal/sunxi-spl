# eGON

[eGON](https://linux-sunxi.org/EGON)

如在多个地方提到的,Allwinner A10,A13,A20 和 A31 引导,BROM 是引导的第一步,并被烧写到芯片本身中.从 BR​​OM 开始,Allwinner 从 NAND 引导启动 boot0 和 boot1.AllWinner 引导加载程序在各个地方的魔术值为'eGON',因此,引导加载程序也应这样称呼.

## eGON.BRM

BROM 引导加载程序已从芯片中提取,可以在 [hno/Allwinner-Info](https://github.com/hno/Allwinner-Info/tree/master/BROM) 的存储库中找到.魔术签名是"eGON.BRM".BROM 从 `0x4000` 开始.如果 BROM 在 NAND 中加载了 boot0,则将其加载并执行.

## eGON.BT0

### Header

head: [sunxi-tools/bootinfo.c](https://github.com/linux-sunxi/sunxi-tools/blob/master/bootinfo.c)

public header

- u32, Jump to address
- 8 \* u8, Magic "eGON.BT0" (no \n)
- 32u, checksum for boot0
- 32u, length for boot0
- 32u, header size of boot0
- 4 \* u8, header version
- 4 \* u8, Boot_vsn
- 4 \* u8, eGON_vsn
- 8 \* u8, platform information

private header

- u32, header size
- 4 \* u8, header version
- boot_dram_para_t, DRAM parameters
- s32, uart port
- 2 \* normal_gpio_cfg,
- s32, enable_jtag (0 off, 1 on)
- 5 \* normal_gpio_cfg, jtag_gpio
- 32 \* normal_gpio_cfg, storage_gpio
- u8 _ 512 - (32 _ sizeof(normal_gpio_cfg)), storage_data

### 从 MMC 加载

brom 从 `nand` 或 `mmc` 加载 `boot0`,然后从 `mmc` 链式加载 `boot1`.检查汇编源码后发现:

- SD 驱动程序与 `u-boot` 的 `sunxi-mmc` 驱动程序非常相似.因此,非常幸运的是,它们是完全相同的
- 已选择 SD 卡并检查其是否为有效的卡槽
- SD/MMC 读取器已经被初始化
- 读取从 SD 上的扇区 `38192` 开始的 2(1kiB)个扇区,并根据魔术值`eGON.BT1`进行检查（没有`\n`）
- 读取存储在 `header` 中的长度,并检查其长度是否正确（以 512 为单位对齐）
- 使用找到的长度从 SD 卡读取 boot1
- 验证 boot1 的校验和
- 设置 `eGON_vsn`（即使校验和失败）
- 关闭 SD 卡

## eGON.BT1

public header

- u32, Jump to address
- 8 \* u8, Magic "eGON.BT1" (no \n)
- 32u, checksum for boot0
- 32u, length for boot0
- 32u, header size of boot0
- 4 \* u8, header version
- 4 \* u8, Boot_vsn
- 4 \* u8, eGON_vsn
- 8 \* u8, platform information

private header

- u32, header size
- 4 \* u8, header version
- s32, uart port
- 2 \* normal_gpio_cfg, UART gpio config
- boot_dram_para_t, DRAM parameters
- 32k * u8, script*buf (fex)
- boot_core_para_t, boot core parameters
- s32, twi_port
- 2 * normal*gpio_cfg, twi gpio config
- s32, debug enable (0 off, 1 on)
- s32, hold_key_min
- s32, hold_key_max
- u32, work_mode
- u32, storage_type (0 = nand, 1 = sdcard, 2 = SPI-NOR
- 32 * normal*gpio_cfg, storage_gpio
- u8 \* 512 - (32 \* sizeof(normal_gpio_cfg)), storage_data
