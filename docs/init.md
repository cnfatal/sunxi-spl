# 初始化

## 初始化 CPU

1. CPU 已经在 BORM 中进行了初始化
2. 时钟已经在 BORM 进行了初始化

## 初始化 UART

### 使能 uart 时钟

参见 datasheet `4.3.5.17.Bus Clock Gating Register3 (Default Value: 0x00000000)`

uart0 时钟位使能由 `BUS_CLK_GATING_REG3` - `16` bit 控制

- CCU RegisterBase: `0x01C20000`
- Register Name: `BUS_CLK_GATING_REG3`
- Offset: `0x006C`

| BitR/W | Default/Hex | Description                                         |
| ------ | ----------- | --------------------------------------------------- |
| 19R/W  | 0x0         | UART3_GATING.Gating Clock For UART3;0: Mask,1: Pass |
| 18R/W  | 0x0         | UART2_GATING.Gating Clock For UART2;0: Mask,1: Pass |
| 17R/W  | 0x0         | UART2_GATING.Gating Clock For UART1;0: Mask,1: Pass |
| 16R/W  | 0x0         | UART2_GATING.Gating Clock For UART0;0: Mask,1: Pass |

```c
reg = readwl(BUS_CLK_GATING_REG3)
/* 设置bit 16 为1 */
reg|=(1<<16)
```

### 设置 GPIO 为 uart 模式

参见 datasheet `4.22.2.1.PA Configure Register 0 (Default Value: 0x77777777)`

`PIO_REG_BASE`:`0x01C20800`

设置 CPU GPIO 选择 uart 模式由 `PA_CFG0_REG` Offset: `0x00`

- `PA4/UART0_TX/PA_EINT4`(F5):UART0_TX
- `PA5/UART0_RX/PWM0/PA_EINT5`(H6):UART0_RX

| BitR/W   | Default/Hex | Description |
| -------- | ----------- | ----------- |
| 22:20R/W | 0x7         | PA5_SELECT  |
| 18:16R/W | 0x7         | PA4_SELECT  |

选择寄存器位`PA5_SELECT`值与功能对照表：

| value | function   |
| ----- | ---------- |
| 000   | Input      |
| 001   | Output     |
| 010   | UART0_RX   |
| 011   | Reserved   |
| 100   | Reserved   |
| 101   | Reserved   |
| 110   | PA_EINT5   |
| 111   | IO Disable |

选择寄存器位`PA4_SELECT`值与功能对照表：

| value | function   |
| ----- | ---------- |
| 000   | Input      |
| 001   | Output     |
| 010   | UART0_TX   |
| 011   | Reserved   |
| 100   | Reserved   |
| 101   | Reserved   |
| 110   | PA_EINT4   |
| 111   | IO Disable |

根据 datasheet, PA_CFG0_REG 默认值为 `0x77777777`,`PA5_SELECT` `PA4_SELECT` 均默认为 `0x7`

```c
uint32_t tmp
tmp = readl(PA_CFG0_REG)
tmp &=(0xff<<16)
tmp |=(0x22<<16)
writel(PA_CFG0_REG,tmp)
```

### 设置 uart

设置波特率，通过将适当的时钟分频值写入分频锁存寄存器（DLL 和 DLH）来设置所需的波特率。

输入 uart 的时钟频率为 24Mhz(注意倍频器是否对时钟频率进行了更改)，目标波特率为 115200，则计算结果为 13.02083333.

$$
Divisor= \frac{UART Input Clock Frequency}{Desired Baud Rate * 16}
$$

$$
~13.02083333333333333333 = \frac{24*1000*1000}{115200 * 16}
$$

根据结果需要设置 DLL 和 DLH

```c
uint16_t divisor;
uint32_t apb_freq = 24 * 1000 * 1000 ;

divisor = apb_freq / UART_BAUD / 16
UART_REG_DLH(port) = divisor >> 8;
UART_REG_DLL(port) = divisor & 0x00ff;
```

halt

```c

lcr = UART_REG_LCR(port);
/* DLAB 位设置为 1 以访问 DLL、DLH */
UART_REG_LCR(port) = lcr | 0x80;

/ * 设置 DLH DLL */
UART_REG_DLH(port) = divisor >> 8;
UART_REG_DLL(port) = divisor & 0x00ff;

/* DLAB 位设置为 0 以访问 RBR、THR  */
UART_REG_LCR(port) =  lcr & (~0x80);



/* 配置 LCR  */
// 禁用奇偶检验 使用0个停止位 字长为0x3 8bits
UART_REG_LCR(port) = ((PARITY&0x03)<<3) | ((STOP&0x01)<<2) | (DLEN&0x03);

// 禁用 fifo
UART_REG_FCR(port) = 0x06;
```

写入

```c
#define SERIAL_WRITE_READY()	( UART_REG_LSR(port) & ( 1 << 6 ) )
#define SERIAL_WRITE_CHAR(c)	( ( UART_REG_THR(port) ) = ( c ) )

void UART_putchar(char c)
{
	while (!SERIAL_WRITE_READY());                       /* nothing */
	    SERIAL_WRITE_CHAR(c);
}
```

## 硬件

H3 有四个可用 uart 接口，其中位于 `pin29(CP_Rx)` `pin30(CP_Tx)` 的 `uart0` 被引出, 连接到了 `CP2102` `pin25(CP_Rx)` `pin26(CP_Tx)`

## 备注

PLL

- 实际应用中，除 PLL_CPUX 外，其他 PLL 不支持动态频率缩放；
- PLL_DDR 频率改变后，PLL_DDR 控制寄存器的 20 位要写 1 才有效；

BUS

- 设置 BUS 时钟时，应先设置分频系数，等分频系数生效后，切换时钟源。
- 时钟源将在至少三个时钟周期后切换；
- 在大多数应用程序中不应动态更改 BUS 时钟

时钟切换

- 在时钟源切换前确保时钟源输出有效，然后设置合适的分频比；在除法因子生效后，切换时钟源

Gating and reset

- 确保在释放模块时钟门控之前已释放复位信号；
