#ifndef __DRVSPI_PLATFORM_H__
#define __DRVSPI_PLATFORM_H__

#define DRVSPI_PLATFORM_IRQ_NUM 19
#define DRVSPI_PLATFORM_CS_NUM 18

typedef void (*drvspi_platform_irq_call)(void *);
int drvspi_platform_init(void);
int drv_spi_platfrom_write(unsigned char *wbuff, unsigned int len);
int drv_spi_platfrom_read(unsigned char *rbuff, unsigned int len);
int drv_spi_platform_gpio_value(int gpionum);
void drv_spi_platform_irq_enable(int irqnum);
void drv_spi_platform_irq_disable(int irqnum);
int drv_spi_platform_irq_req(char *irqname, int irqnum, int irqmode,
    drvspi_platform_irq_call irqcall, void *args);

#endif