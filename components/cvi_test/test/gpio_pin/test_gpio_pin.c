#include <drv/gpio.h>
#include <drv/gpio_pin.h>
#include <stdio.h>
#include <drv/irq.h>
#include <drv/pin.h>
#include <unistd.h>
#include "aos/cli.h"
#include <soc.h>

static int enter_irq = 0;

static void gpio_pin_irq_test(void)
{
	aos_cli_printf("goto %s() \r\n", __func__);
    enter_irq = 1;
}

void test_gpio_pin(int32_t argc, char **argv)
{
    csi_error_t ret;
    csi_gpio_pin_t gpio_pin = {0};
    unsigned int gpio_value;

	if (argc < 3) {
        aos_cli_printf("\tplease run(example): testgpiopin {pin_name:1:IIC0_SCL 2:IIC0_SDA} {function} [value] \r\n");
		aos_cli_printf("\tplease run(output):  testgpiopin 1 write 1 \r\n");
        aos_cli_printf("\tplease run(input):   testgpiopin 2 read \r\n");
        aos_cli_printf("\tplease run(input):   testgpiopin 2 irq \r\n");
		return;
	}

    switch (atoi(argv[1]))
    {
    case 1:
        ret = csi_gpio_pin_init(&gpio_pin, IIC0_SCL);
        break;
    case 2:
        ret = csi_gpio_pin_init(&gpio_pin, IIC0_SDA);
        break;
    default:
        ret = CSI_ERROR;
        aos_cli_printf("invalid pin_name\r\n");
        break;
    }

    if (ret != CSI_OK)
    {
	    aos_cli_printf("csi_gpio_init failed\r\n");
		return;
    }

    if (strncmp(argv[2], "read", strlen(argv[2])) == 0) {
        csi_gpio_pin_dir(&gpio_pin, GPIO_DIRECTION_INPUT);
        if (csi_gpio_pin_read(&gpio_pin) != 0U) {
            gpio_value = 1;
        } else {
            gpio_value = 0;
        }
        aos_cli_printf("read gpio_value : %d \r\n",gpio_value);
    } else if (strncmp(argv[2], "write", strlen(argv[2])) == 0) {
        gpio_value = (argv[3] == NULL) ? 0 : atoi(argv[3]);
        csi_gpio_pin_dir(&gpio_pin, GPIO_DIRECTION_OUTPUT);
        csi_gpio_pin_write(&gpio_pin, gpio_value ? 1 : 0);
        aos_cli_printf("write gpio_value : %d \r\n", gpio_value ? 1 : 0);
    } else if (strncmp(argv[2], "irq", strlen(argv[2])) == 0) {
        enter_irq = 0;
        csi_gpio_pin_dir(&gpio_pin, GPIO_DIRECTION_INPUT);
        csi_gpio_pin_mode(&gpio_pin, GPIO_MODE_PULLUP);
        csi_gpio_pin_irq_mode(&gpio_pin, GPIO_IRQ_MODE_FALLING_EDGE);
        csi_gpio_pin_attach_callback(&gpio_pin, gpio_pin_irq_test, NULL);
        csi_gpio_pin_irq_enable(&gpio_pin, true);
        while(!enter_irq)
        {
            mdelay(1000);
        }
        csi_gpio_pin_irq_enable(&gpio_pin, false);
    } else {
        aos_cli_printf("invaild param: %s\r\n",argv[2]);
        return ;
    }

	csi_gpio_pin_uninit(&gpio_pin);
	// free(gpio.priv);

	// aos_cli_printf("test gpio success.\r\n");
}
ALIOS_CLI_CMD_REGISTER(test_gpio_pin, testgpiopin, test gpio pin function);
