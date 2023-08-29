#include <drv/pin.h>
#include "cvi_pin.h"
#include "aos/cli.h"

csi_error_t csi_pin_set_mux(pin_name_t pin_name, pin_func_t pin_func)
{
	/* please usb marco PINMUX_COMFIG replace this function */
	return CSI_UNSUPPORTED;
}

pin_func_t csi_pin_get_mux(pin_name_t pin_name)
{
    return cvi_pin_get_mux(pin_name);
}

csi_error_t csi_pin_mode(pin_name_t pin_name, csi_pin_mode_t mode)
{
	return cvi_pin_set_mode(pin_name, mode);
}

csi_error_t csi_pin_speed(pin_name_t pin_name, csi_pin_speed_t speed)
{
    csi_error_t ret;
    switch (speed)
    {
        case PIN_SPEED_LV0:
            ret = cvi_pin_set_speed(pin_name, CVI_PIN_SPEED_SLOW);
            break;
        case PIN_SPEED_LV1:
            ret = cvi_pin_set_speed(pin_name, CVI_PIN_SPEED_FAST);
            break;
        default:
            aos_cli_printf("pin name %d is unsupported to set speed %d\n", pin_name, speed);
            ret = CSI_UNSUPPORTED;
    }
	return ret;
}

csi_error_t csi_pin_drive(pin_name_t pin_name, csi_pin_drive_t drive)
{
    csi_error_t ret;
    switch (drive)
    {
        case PIN_DRIVE_LV0:
            ret = cvi_pin_set_drive(pin_name, PIN_DRIVE_STRENGTH0);
            break;
        case PIN_DRIVE_LV1:
            ret = cvi_pin_set_drive(pin_name, PIN_DRIVE_STRENGTH1);
            break;
        default:
            aos_cli_printf("pin name %d is unsupported to set drive %d\n", pin_name, drive);
            ret = CSI_UNSUPPORTED;
    }
    return ret;
}

