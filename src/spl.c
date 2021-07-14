#define set_wbit(addr, v) (*((volatile unsigned long *)(addr)) |= (unsigned long)(v))
#define readl(addr) (*((volatile unsigned long *)(addr)))
#define writel(v, addr) (*((volatile unsigned long *)(addr)) = (unsigned long)(v))

#define SUNXI_UART0_BASE 0x01C28000
#define SUNXI_PIO_BASE 0x01C20800
#define AW_CCM_BASE 0x01c20000
#define AW_SRAMCTRL_BASE 0x01c00000

typedef unsigned int u32;

struct sunxi_gpio
{
    u32 cfg[4];
    u32 dat;
    u32 drv[2];
    u32 pull[2];
};

struct sunxi_gpio_reg
{
    struct sunxi_gpio gpio_bank[10];
};

#define UART0_LSR (SUNXI_UART0_BASE + 0x14) /* line status register */
#define UART0_THR (SUNXI_UART0_BASE + 0x0)  /* transmit holding register */

void uart0_putc(char c)
{
    while (!(readl(UART0_LSR) & (1 << 6)))
    {
    }
    writel(c, UART0_THR);
}

void uart0_puts(const char *s)
{
    while (*s)
    {
        if (*s == '\n')
            uart0_putc('\r');
        uart0_putc(*s++);
    }
}

#define GPIO_BANK(pin) ((pin) >> 5)
#define GPIO_NUM(pin) ((pin)&0x1F)

#define GPIO_CFG_INDEX(pin) (((pin)&0x1F) >> 3)
#define GPIO_CFG_OFFSET(pin) ((((pin)&0x1F) & 0x7) << 2)

int sunxi_gpio_set_cfgpin(u32 pin, u32 val)
{
    u32 cfg;
    u32 bank = GPIO_BANK(pin);
    u32 index = GPIO_CFG_INDEX(pin);
    u32 offset = GPIO_CFG_OFFSET(pin);
    struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];
    cfg = readl(&pio->cfg[0] + index);
    cfg &= ~(0xf << offset);
    cfg |= val << offset;
    writel(cfg, &pio->cfg[0] + index);
    return 0;
}

#define GPIO_PULL_INDEX(pin) (((pin)&0x1f) >> 4)
#define GPIO_PULL_OFFSET(pin) ((((pin)&0x1f) & 0xf) << 1)

int sunxi_gpio_set_pull(u32 pin, u32 val)
{
    u32 cfg;
    u32 bank = GPIO_BANK(pin);
    u32 index = GPIO_PULL_INDEX(pin);
    u32 offset = GPIO_PULL_OFFSET(pin);
    struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];
    cfg = readl(&pio->pull[0] + index);
    cfg &= ~(0x3 << offset);
    cfg |= val << offset;
    writel(cfg, &pio->pull[0] + index);
    return 0;
}

#define SUNXI_GPA(n) (0 + (n))

/* A workaround for https://patchwork.ozlabs.org/patch/622173 */
void __attribute__((section(".start"))) __attribute__((naked)) start(void)
{
    asm volatile("b     main             \n"
                 ".long 0xffffffff       \n"
                 ".long 0xffffffff       \n"
                 ".long 0xffffffff       \n");
}

#define UART0_DLL (SUNXI_UART0_BASE + 0x0) /* divisor latch low register */
#define UART0_DLH (SUNXI_UART0_BASE + 0x4) /* divisor latch high register */
#define UART0_LCR (SUNXI_UART0_BASE + 0xc) /* line control register */

#define BAUD_115200 (0xD) /* 24 * 1000 * 1000 / 16 / 115200 = 13 */
#define LC_8_N_1 (0 << 3 | 0 << 2 | 3)

#define AW_CCM_BASE 0x01c20000
#define APB2_GATE (AW_CCM_BASE + 0x06C)
#define APB2_RESET (AW_CCM_BASE + 0x2D8)
void main(void)
{
    sunxi_gpio_set_cfgpin(SUNXI_GPA(4), 2);
    sunxi_gpio_set_cfgpin(SUNXI_GPA(5), 2);
    sunxi_gpio_set_pull(SUNXI_GPA(5), 1);

    /* Open the clock gate for UART0 */
    set_wbit(APB2_GATE, 1 << 16);
    /* Deassert UART0 reset (only needed on A31/A64/H3) */
    set_wbit(APB2_RESET, 1 << 16);

    /* select dll dlh */
    writel(0x80, UART0_LCR);
    /* set baudrate */
    writel(0, UART0_DLH);
    writel(BAUD_115200, UART0_DLL);
    readl(UART0_LCR) &= (~0x80);
    /* set line control */
    writel(LC_8_N_1, UART0_LCR);

    uart0_putc('h');
    uart0_putc('e');
    uart0_putc('l');
    uart0_putc('l');
    uart0_putc('0');

    uart0_puts("Hello from Allwinner H3\n");
}