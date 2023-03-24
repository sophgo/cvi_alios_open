#ifndef _GLOBAL_SLEEP_FLAG_H_
#define _GLOBAL_SLEEP_FLAG_H_

#ifdef __cplusplus
extern "C" {
#endif

void   sleep_init_flag();

void   set_tf_ota_flag(int state);
int    get_tf_ota_flag();
void   set_cloud_ota_flag(int state);
int    get_cloud_ota_flag();
void   set_dowload_file_flag(int state);
int    get_dowload_file_flag();
void   set_tf_weight_flag(int state);
int    get_tf_weight_flag();
void   set_ready_play_flag();
void   set_snap_flag();
int    get_snap_flag();

void   set_lv_flag(int state);
void   set_snap_timerwait();

int    get_living_flag();



//void   cli_reg_cmd_notsleep(void);






#ifdef __cplusplus
}
#endif

#endif

