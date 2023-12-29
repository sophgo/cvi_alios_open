#include <drv/rtc.h>
#include <aos/cli.h>
#include <ulog/ulog.h>
#include <aos/kernel.h>
static void rtc_callback(csi_rtc_t *rtc, void *arg)
{
    *((uint8_t *)arg) = 1;
    aos_cli_printf("alarm callback\r\n");
}

static void print_usage(void)
{
    printf("usage:\n\trtc init <idx>"
            "\n\trtc uinit"
            "\n\trtc settime <\"year-mon-day hour:min:sec\">"
            "\n\trtc gettime"
            "\n\trtc setalarm <\"year-mon-day hour:min:sec\">"
            "\n\trtc cancelalarm\n");
}
static csi_rtc_t test_rtc_handler;


void cvi_rtc_test(int argc, char **argv)
{
    uint32_t idx;
    //uint32_t device_addr;
    //uint32_t reg_addr;
    //uint32_t data;
    //uint32_t reg_width;
    //uint32_t data_width;
    char *cmd = NULL;
    char *para3;
    int ret;
    volatile uint8_t rtc_flag = 0;
    csi_rtc_time_t tm_test = {.tm_year = 120,
                              .tm_mon = 11,
                              .tm_mday = 31,
                              .tm_hour = 23,
                              .tm_min = 59,
                              .tm_sec = 59
                             };
    if (argc < 2) {
        print_usage();
        return;
    }

    cmd = argv[1];

    if (!strcmp(cmd, "init") && argc == 3) {

        idx = strtol(argv[2], NULL, 16);
        ret = csi_rtc_init(&test_rtc_handler, idx);
        if (ret != CSI_OK)
        {
            aos_cli_printf("rtc init failed\r\n");
            return;
        }
    } else if (!strcmp(cmd, "uinit") && argc == 2) {
        
        csi_rtc_uninit(&test_rtc_handler);
    } else if (!strcmp(cmd, "settime") && argc == 3) {
        para3 = argv[2];
        ret = sscanf(para3, "%d-%d-%d %d:%d:%d", &tm_test.tm_year, &tm_test.tm_mon, &tm_test.tm_mday, \
         &tm_test.tm_hour, &tm_test.tm_min, &tm_test.tm_sec);
        if (ret != 6){
            aos_cli_printf("Time invalid format\r\n"); 
        }
        aos_cli_printf("set time: %d-%d-%d %d:%d:%d\r\n",tm_test.tm_year, tm_test.tm_mon, tm_test.tm_mday, \
            tm_test.tm_hour, tm_test.tm_min, tm_test.tm_sec);
        tm_test.tm_year = tm_test.tm_year - 1900;
        tm_test.tm_mon = tm_test.tm_mon - 1;   
        if (tm_test.tm_year < 70 || tm_test.tm_year > 199 || \
            tm_test.tm_mon < 0 || tm_test.tm_mon > 11     || \
            tm_test.tm_mday < 1 || tm_test.tm_mday > 31   || \
            tm_test.tm_hour < 0 || tm_test.tm_hour > 23   || \
            tm_test.tm_min < 0 || tm_test.tm_min > 59   || \
            tm_test.tm_sec < 0 || tm_test.tm_sec > 59
        ) {
            aos_cli_printf("Passed an invalid rtc time\r\n");
        } else {
            ret = csi_rtc_set_time(&test_rtc_handler, &tm_test);
            if (ret != CSI_OK)
            {
                aos_cli_printf("Rtc settime fail, maybe time invalid\r\n");
                return;
            }
        }
    } else if (!strcmp(cmd, "gettime") && argc == 2) {

        ret = csi_rtc_get_time(&test_rtc_handler, &tm_test);
        if (ret != CSI_OK)
        {
            aos_cli_printf("Rtc gettime fail, maybe time invalid\r\n"); 
            return;
        }
        aos_cli_printf("%d-%02d-%02d %02d:%02d:%02d\r\n",tm_test.tm_year + 1900, tm_test.tm_mon + 1, tm_test.tm_mday, \
        tm_test.tm_hour, tm_test.tm_min, tm_test.tm_sec);
        
    } else if (!strcmp(cmd, "setalarm") && argc == 3) {

        para3 = argv[2];
        ret = sscanf(para3, "%d-%d-%d %d:%d:%d", &tm_test.tm_year, &tm_test.tm_mon, &tm_test.tm_mday, \
         &tm_test.tm_hour, &tm_test.tm_min, &tm_test.tm_sec);
        if (ret != 6){
            aos_cli_printf("Rtc time invalid format\r\n"); 
        }
        aos_cli_printf("set alarm: %d-%d-%d %d:%d:%d\r\n",tm_test.tm_year, tm_test.tm_mon, tm_test.tm_mday, \
            tm_test.tm_hour, tm_test.tm_min, tm_test.tm_sec);
        tm_test.tm_year = tm_test.tm_year - 1900;
        tm_test.tm_mon = tm_test.tm_mon - 1;   
        if (tm_test.tm_year < 70 || tm_test.tm_year > 199 || \
            tm_test.tm_mon < 0 || tm_test.tm_mon > 11     || \
            tm_test.tm_mday < 1 || tm_test.tm_mday > 31   || \
            tm_test.tm_hour < 0 || tm_test.tm_hour > 23   || \
            tm_test.tm_min < 0 || tm_test.tm_min > 59   || \
            tm_test.tm_sec < 0 || tm_test.tm_sec > 59
        ) {
            aos_cli_printf("Passed an invalid RTC time\r\n");
        } else {
            ret = csi_rtc_set_alarm(&test_rtc_handler, &tm_test, rtc_callback, (void *)&rtc_flag);
            if (ret != CSI_OK)
            {
                aos_cli_printf("Rtc setalarm fail, maybe time invalid\r\n");
                return;
            }
        }

    } else if (!strcmp(cmd, "cancelalarm") && argc == 2) {
        ret =  csi_rtc_cancel_alarm(&test_rtc_handler);
        if (ret != CSI_OK)
        {
            aos_cli_printf("RTC cancelalarm failed\r\n");
            return;
        }
    
    }else {
        print_usage();
    }
}

ALIOS_CLI_CMD_REGISTER(cvi_rtc_test, rtc, rtc init/uinit/settime/gettime/setalarm/cancelalarm);