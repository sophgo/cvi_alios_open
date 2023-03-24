#include <drv/uart.h>
#include <drv/baud_calc.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>

typedef int (*case_func)(void *args);

typedef struct {
    char *cml_name;
    case_func case_name;
} baudcalc_map;

typedef struct {
    uint32_t uart_idx;
    uint32_t baudrate;
    uint8_t stop_bits;
    uint8_t data_bits;
    uint8_t parity;
    uint8_t flowctrl;
    uint32_t times;
} test_baudcalc_uart_t;

typedef struct {
    uint32_t pwm_idx;
    uint32_t channel;
    uint32_t baudrate_min;
    uint32_t baudrate_max;
} test_baudcalc_calc_t;

typedef struct {
    uint32_t pwm_idx;
    uint32_t channel;
    uint32_t baudrate_expect;
} test_baudcalc_adjust_t;

int test_baudcalc_calculateBaudrate(void *args);
int test_baudcalc_adjustBaudrate(void *args);
int test_baudcalc_outputBaudrate(void *args);