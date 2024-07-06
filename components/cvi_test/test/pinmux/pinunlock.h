#define RTC_IO_BASE 0x05027000

struct pinunlock {
char name[24];
unsigned int offset;
} pinunlock_st;


struct pinunlock pin_unlock[] = {
{ "CTRL_selphy_PWR_SEQ1", 0x078 },
{ "CTRL_selphy_PWR_SEQ2", 0x07c },
{ "CTRL_selphy_PWR_SEQ3", 0x080 },
{ "CTRL_selphy_PWR_BUTTON1", 0x084 },
{ "CTRL_selphy_PWR_ON", 0x088 },
{ "CTRL_selphy_PWR_WAKEUP0", 0x08c },
{ "CTRL_selphy_PWR_WAKEUP1", 0x090 },
{ "CTRL_selphy_CLK25M", 0x094 },
{ "CTRL_selphy_CLK32K", 0x098 },
{ "CTRL_selphy_IIC2_SCL", 0x09c },
{ "CTRL_selphy_IIC2_SDA", 0x0a0},
{ "CTRL_selphy_PWR_GPIO0", 0x0a4 },
{ "CTRL_selphy_PWR_GPIO1", 0x0a8 },
{ "CTRL_selphy_PWR_GPIO2", 0x0ac },
{ "CTRL_selphy_SD1_CLK", 0x0b0 },
{ "CTRL_selphy_SD1_CMD", 0x0b4 },
{ "CTRL_selphy_SD1_D0", 0x0b8 },
{ "CTRL_selphy_SD1_D1 ", 0x0bc },
{ "CTRL_selphy_SD1_D2", 0x0c0},
{ "CTRL_selphy_SD1_D3", 0x0c4},
{ "CTRL_selphy3_UART2_CTS ", 0x0c8 },
{ "CTRL_selphy3_UART2_RTS ", 0x0cc},
{ "CTRL_selphy2_UART2_RX", 0x0d0},
{ "CTRL_selphy2_UART2_TX ", 0x0d4 },
{ "CTRL_selphy2_GPIO_ZQ", 0x0dc}
};
