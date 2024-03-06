
#include <stdint.h>

#include "sensor.h"

#if CONFIG_SENSOR_GCORE_GC2093
extern int gc2093_probe(uint8_t i2c_addr, uint8_t i2c_dev);
extern void gc2093_init();
extern void gc2093_exit();
#endif
#if CONFIG_SENSOR_SMS_SC201CS
extern int sc201cs_probe(uint8_t i2c_addr, uint8_t i2c_dev);
extern void sc201cs_init(uint8_t ViPipe);
extern void sc201cs_exit(uint8_t ViPipe);
#endif
#if CONFIG_SENSOR_SMS_SC201CS_SLAVE
extern int sc201cs_slave_probe(uint8_t i2c_addr, uint8_t i2c_dev);
extern void sc201cs_slave_init(uint8_t ViPipe);
extern void sc201cs_slave_exit(uint8_t ViPipe);
#endif
#if CONFIG_SENSOR_SMS_SC201CS_MULTI
extern int sc201cs_multi_probe(uint8_t i2c_addr, uint8_t i2c_dev);
extern void sc201cs_multi_init(uint8_t ViPipe);
extern void sc201cs_multi_exit(uint8_t ViPipe);
#endif
int sensor_init(void)
{
	int ret = 0;
	sensor_pinmux_init();
#if CONFIG_SENSOR_GCORE_GC2093
	ret = gc2093_probe(0x37, 2);
	gc2093_init();
	gc2093_exit();
#endif
#if CONFIG_SENSOR_SMS_SC201CS
	ret = sc201cs_probe(0x30, 2);
	sc201cs_init(0);
	sc201cs_exit(0);
#endif
#if CONFIG_SENSOR_SMS_SC201CS_SLAVE
	ret = sc201cs_slave_probe(0x32, 2);
	sc201cs_slave_init(0);
	sc201cs_slave_exit(0);
#endif
#if CONFIG_SENSOR_SMS_SC201CS_MULTI
	ret = sc201cs_multi_probe(0x30, 4);
	sc201cs_multi_init(0);
	sc201cs_multi_exit(0);
#endif

	return ret;
}

