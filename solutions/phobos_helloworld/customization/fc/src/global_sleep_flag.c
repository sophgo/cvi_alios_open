#include<stdio.h>
#include <aos/kernel.h>
#include <aos/cli.h>
#include <aos/kv.h>
#include "ulog/ulog.h"
#include "wifi_hostmsgprase.h"
#include "wifi_common.h"
#include "wifi_dispatch.h"

#include "global_sleep_flag.h"
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
#include "cx_func.h"
#endif


#define TAG "GLOBAL_SLEEP_FLAG"


#define FLAG_TF_OTA          "tf_ota_flag"
#define FLAG_CLOUD_OTA       "cloud_ota_flag"
#define FLAG_TF_WEIGHT       "tf_weight_flag"
#define FLAG_DOWLOAD_FILE    "dowload_file_flag"

typedef struct {
	int tf_ota_flag;
	int cloud_ota_flag;
	int deint_flag;
	int tf_weight_flag;
	int dowload_file_flag;
} Flag_Static_Deinit;


typedef struct {
	int ready_flag;
	int temp_ns_flag ;
	int lv_flag;
	int snap_cloud_flag ;
	int picture_task_flag ;
	long long ready_timer ;
	long long timer_snap ;
	long long force_stop_timer ;
} Flag_Dynamic_Deinit;


static Flag_Static_Deinit   g_staflag ;
static Flag_Dynamic_Deinit  g_dynflag ;


void set_lv_flag(int state)
{

	g_dynflag.lv_flag = state;
	if (0 == state) {
		g_dynflag.ready_flag = 0;
	} else {
		g_dynflag.force_stop_timer = aos_now_ms();
	}
}




void sleep_init_flag()
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	memset(&g_staflag, 0, sizeof(Flag_Static_Deinit));
	memset(&g_dynflag, 0, sizeof(Flag_Dynamic_Deinit));

	int ret = aos_kv_getint(FLAG_TF_OTA, &g_staflag.tf_ota_flag);
	if (ret != 0) {
		LOGD(TAG, "no tf ota key \r\n");
		g_staflag.tf_ota_flag = 0;
	}

	ret = aos_kv_getint(FLAG_CLOUD_OTA, &g_staflag.cloud_ota_flag);
	if (ret != 0) {
		LOGD(TAG, "no cloud_ota_flag key \r\n");
		g_staflag.cloud_ota_flag = 0;
	}

	ret = aos_kv_getint(FLAG_TF_WEIGHT, &g_staflag.tf_weight_flag);
	if (ret != 0) {

		LOGD(TAG, "no tf_weight_flag key \r\n");
		g_staflag.tf_weight_flag = 0;
	}

	ret = aos_kv_getint(FLAG_DOWLOAD_FILE, &g_staflag.dowload_file_flag);
	if (ret != 0) {

		LOGD(TAG, "no dowload_file_flag key \r\n");
		g_staflag.dowload_file_flag = 0;
	}
	cx_user_info_cb info_cb;
	info_cb.get_encode_state = set_lv_flag;
	cx_register_user_cb(&info_cb);
#endif
}


void set_tf_ota_flag(int state)
{
	aos_kv_setint(FLAG_TF_OTA, state);
	g_staflag.tf_ota_flag = state;
}

int get_tf_ota_flag()
{
	return g_staflag.tf_ota_flag;
}

void set_cloud_ota_flag(int state)
{
	aos_kv_setint(FLAG_CLOUD_OTA, state);
	g_staflag.cloud_ota_flag = state;
}


int get_cloud_ota_flag()
{
	return g_staflag.cloud_ota_flag;
}

void set_dowload_file_flag(int state)
{
	aos_kv_setint(FLAG_DOWLOAD_FILE, state);
	g_staflag.dowload_file_flag = state;
}

int get_dowload_file_flag()
{
	return g_staflag.dowload_file_flag;
}


void set_tf_weight_flag(int state)
{
	aos_kv_setint(FLAG_TF_WEIGHT, state);
	g_staflag.tf_weight_flag = state;
}


int get_tf_weight_flag()
{
	return g_staflag.tf_weight_flag;
}

/*
* some task will limit the time before sleep
*
*/

void set_ready_play_flag()
{
	g_dynflag.ready_flag = 1;
	g_dynflag.ready_timer = aos_now_ms();
}

int judge_ready_play_flag()
{
	long long temp = aos_now_ms();
	if ((temp - g_dynflag.ready_timer) / 1000 >= 10) {
		g_dynflag.ready_flag = 0;
		return 0;
	} else {
		return g_dynflag.ready_flag;
	}

	return 0;
}

void set_snap_flag()
{
	g_dynflag.snap_cloud_flag = 1;
}

int get_snap_flag()
{
	return g_dynflag.picture_task_flag;
}



void picture_task(void *paras)
{
	printf("picture_task time limit start  \r\n");
	long long temp = aos_now_ms();
	while (1) {
		temp = aos_now_ms();
		if ((temp - g_dynflag.timer_snap) / 1000 >= 10) {
			break;
		}
		aos_msleep(200);

	}
	g_dynflag.picture_task_flag = 0;


	WIFI_MSGINFO_S _Info = {0};
	_Info.HeartInfo.SleepEnable = 0x00;
	_Info.HeartInfo.SleepEnableLen = 1;
	Wifi_HostMsgUpLoad(H2W_HOST_CMD_HEART, &_Info);

	printf("picture_task time limit over  \r\n");
	aos_task_exit(0);

}


void set_snap_timerwait()
{
	if (1 == g_dynflag.picture_task_flag) {
		return;
	} else {
		g_dynflag.timer_snap = aos_now_ms();
		aos_task_t picture_success_get;
		g_dynflag.picture_task_flag = 1;
		aos_task_new_ext(&picture_success_get, "picture_task", picture_task, NULL, 4 * 1024, 32);
	}

}



static void cmd_temp_nosleep_func(int argc, char **argv)
{
	int item_count = argc;
	if (item_count == 2) {
		if (strcmp(argv[1], "0") == 0) {

			g_dynflag.temp_ns_flag = 0;
		} else if (strcmp(argv[1], "1") == 0) {

			g_dynflag.temp_ns_flag = 1;
		}
		return ;
	}

	printf("cli_cmd error! input like this  hcc_debug [0/1] \r\n");
	return;

}



/*
void cli_reg_cmd_notsleep(void)
{
    static const struct cli_command cmd_info = {
        "notsleep_temp",
        "notsleep_temp [0/1] 0 sleep 1 notsleep",
        cmd_temp_nosleep_func,
    };

    aos_cli_register_command(&cmd_info);
}*/

ALIOS_CLI_CMD_REGISTER(cmd_temp_nosleep_func, notsleep_temp, notsleep_temp [0 / 1] 0 sleep 1 notsleep);

int get_living_flag()
{
	long long temp_force_sleep = aos_now_ms();
	if (1 == g_dynflag.lv_flag) {
		if ((temp_force_sleep - g_dynflag.force_stop_timer) / 1000 >= 140) {
			return 0;
		}
	}

	if (g_staflag.tf_ota_flag == 0 && g_dynflag.lv_flag == 0 &&
	    g_staflag.cloud_ota_flag == 0 && g_staflag.tf_weight_flag == 0 &&
	    g_staflag.dowload_file_flag == 0 && g_dynflag.temp_ns_flag == 0 &&
	    g_dynflag.picture_task_flag == 0) {

		if (judge_ready_play_flag() == 0) { //&& snap_timer_judge()==0
			return 0;
		}

	}

	return 1;
}






