#ifndef _PROTOCOL_SEV3X_
#define _PROTOCOL_SEV3X_

#include <stdio.h>
#include <string.h>

//=============================================================================
// 帧的字节顺序
//=============================================================================
#define HEAD_FIRST   0
#define HEAD_SECOND  1
#define MSG_ID       2
#define LENGTH_HIGH  3
#define LENGTH_LOW   4
#define DATA_START   5
#define PARITY_CHECK 6

#define CVI_MID_GET_HARDWARE  0xEE
#define CVI_MID_WRITE_LICENSE 0xEF
#define CVI_MID_CAPTURE_IMAGE 0xE3

//=============================================================================
#define VERSION          0x00  // 协议版本号
#define PROTOCOL_HEAD    0x06  // 固定协议头长度
#define FIRM_UPDATA_SIZE 256   // 升级包大小

#define FRAME_FIRST  0xEF
#define FRAME_SECOND 0xAA

/******************************************************************************/
#define DATALINK_QUEUE_LMT    256    // 数据接收队列大小,如MCU的RAM不够,可缩小
#define PROTOCOL_RECV_BUF_LMT 256    // 根据用户DP数据大小量定,必须大于24
#define PROTOCOL_SEND_BUF_LMT 65535  // 根据用户DP数据大小量定,必须大于24

//=============================================================================
// 数据帧类型
//=============================================================================
#define MID_REPLY               0x00
#define MID_NOTE                0x01
#define MID_IMAGE               0x02
#define MID_RESET               0x10
#define MID_GETSTATUS           0x11
#define MID_VERIFY              0x12
#define MID_ENROLL              0x13
#define MID_SNAPIMAGE           0x16
#define MID_GETSAVEDIMAGE       0x17
#define MID_UPLOADIMAGE         0x18
#define MID_ENROLL_SINGLE       0x1D
#define MID_DELUSER             0x20
#define MID_DELALL              0x21
#define MID_GETUSERINFO         0x22
#define MID_FACERESET           0x23
#define MID_GET_ALL_USERID      0x24
#define MID_ENROLL_ITG          0x26
#define MID_NOTIFY_WIFI_INIT    0x28
#define MID_GET_VERSION         0x30
#define MID_START_OTA           0x40
#define MID_STOP_OTA            0x41
#define MID_GET_OTA_STATUS      0x42
#define MID_OTA_HEADER          0x43
#define MID_OTA_PACKET          0x44
#define MID_INIT_ENCRYPTION     0x50
#define MID_CONFIG_BAUDRATE     0x51
#define MID_SET_RELEASE_ENC_KEY 0x52
#define MID_SET_DEBUG_ENC_KEY   0x53
#define MID_GET_LOGFILE         0x60
#define MID_UPLOAD_LOGFILE      0x61
#define MID_SET_THRESHOLD_LEVEL 0xD4
#define MID_POWERDOWN           0xED
#define MID_DEBUG_MODE          0xF0
#define MID_GET_DEBUG_INFO      0xF1
#define MID_UPLOAD_DEBUG_INFO   0xF2
#define MID_GETLIBRARY_VERSION  0xF3
#define MID_DEMOMODE            0xFE

//=============================================================================
/*定义常量*/
//=============================================================================
#ifndef TRUE
#define TRUE 1
#endif
//
#ifndef FALSE
#define FALSE 0
#endif
//
#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef SUCCESS
#define SUCCESS 1
#endif

#ifndef ERROR
#define ERROR 0
#endif

#ifndef INVALID
#define INVALID 0xFF
#endif

#ifndef ENABLE
#define ENABLE 1
#endif
//
#ifndef DISABLE
#define DISABLE 0
#endif

#define USER_NAME_SIZE           32
#define VERSION_INFO_BUFFER_SIZE 32

typedef enum _MS_STATUS_E {
    MS_STANDBY = 0,
    MS_BUSY,
    MS_ERROR,
    MS_INVALID,
    MS_FORMAT_MAX
} MS_STATUS_E;

/* REPLY消息的Result枚举 */
typedef enum _MR_STATUS_E {
    MR_SUCCESS               = 0,
    MR_REJECTED              = 1,
    MR_ABORTED               = 2,
    MR_FAILED4_CAMERA        = 4,
    MR_FAILED4_UNKNOWNREASON = 5,
    MR_FAILED4_INVALIDPARAM  = 6,
    MR_FAILED4_NOMEMORY      = 7,
    MR_FAILED4_UNKNOWNUSER   = 8,
    MR_FAILED4_MAXUSER       = 9,
    MR_FAILED4_FACEENROLLED  = 10,
    MR_FAILED4_LIVENESSCHECK = 12,
    MR_FAILED4_TIMEOUT       = 13,
    MR_FAILED4_AUTHORIZATION = 14,
    MR_FAILED4_READ_FILE     = 19,
    MR_FAILED4_WRITE_FILE    = 20,
    MR_FAILED4_NO_ENCRYPT    = 21,
} MR_STATUS_E;

// NOTE消息的人脸状态枚举
typedef enum _FACE_STATE_E {
    FACE_STATE_NORMAL,
    FACE_STATE_NOFACE,
    FACE_STATE_TOOUP,
    FACE_STATE_TOODOWN,
    FACE_STATE_TOOLEFT,
    FACE_STATE_TOORIGHT,
    FACE_STATE_FAR,
    FACE_STATE_CLOSE,
    FACE_STATE_EYEBROW_OCCLUSION,
    FACE_STATE_EYE_OCCLUSION,
    FACE_STATE_FACE_OCCLUSION,
    FACE_STATE_DIRECTION_ERROR
} FACE_STATE_E;

typedef enum _NID_STATUS_E {
    NID_READY = 0,
    NID_FACE_STATE,
    NID_UNKNOWNERROR,
    NID_OTA_DONE,
    NID_AUTHORIZATION = 8
} NID_STATUS_E;

typedef enum _s_face_dir {
    FACE_DIRECTION_UP     = 0x10,
    FACE_DIRECTION_DOWN   = 0x08,
    FACE_DIRECTION_LEFT   = 0x04,
    FACE_DIRECTION_RIGHT  = 0x02,
    FACE_DIRECTION_MIDDLE = 0x01,
} s_face_dir;

typedef struct _s_msg_reply_getstatus_data {
    uint8_t status;
} s_msg_reply_getstatus_data;

typedef struct _s_pixface_contorl {
    uint8_t facial_recognition;
    uint8_t save_faceinfo;
} s_pixface_contorl;

typedef struct _s_msg_init_encryption_data_v2 {
    uint8_t seed[4];
    uint8_t mode;
    uint8_t crttime[4];
    uint8_t power_mode;
    uint8_t reserved[2];
} s_msg_init_encryption_data_v2;

typedef struct _s_msg_enc_key_number_data {
    uint8_t enc_key_number[16];  // 00 ~ 1F
} s_msg_enc_key_number_data;

typedef struct _s_msg_upload_image_data {
    /* 表示该次上传图像的偏移量 */
    uint8_t upload_image_offset[4];
    /* 表示该次上传图像的大小，最大不超过4000字节 */
    uint8_t upload_image_size[4];
} s_msg_upload_image_data;

typedef struct _s_msg_snap_image_data {
    /* 抓拍图像的数量 */
    uint8_t image_counts;
    /* 抓取图像的名称编号 */
    uint8_t start_number;
} s_msg_snap_image_data;

typedef struct _s_msg_get_saved_image_data {
    /* 指定要获取的图像的编号 */
    uint8_t image_number;
} s_msg_get_saved_image_data;

typedef struct _s_msg_reply_get_saved_image_data {
    /* image size int -> [s1, s2, s3, s4] */
    uint8_t image_size[4];
} s_msg_reply_get_saved_image_data;

typedef struct _s_msg_reply_get_log_data {
    uint8_t log_size[4];  // log size int -> [s1, s2, s3, s4]
} s_msg_reply_get_log_data;

typedef struct _s_msg_upload_logfile_data {
    uint8_t upload_logfile_offset[4];  // upload logfile offset, int -> [o1 o2 o3 o4]
    uint8_t upload_logfile_size[4];    // uploade logfile size, int -> [s1 s2 s3 s4]
} s_msg_upload_logfile_data;

typedef struct _s_msg_reply_library_version_data {
    uint8_t library_version_info[20];
} s_msg_reply_library_version_data;

typedef struct _s_msg_reply_verify_data {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t user_name[USER_NAME_SIZE];  // 32Bytes
    uint8_t admin;
    uint8_t unlockStatus;
} s_msg_reply_verify_data;

typedef struct _s_msg_deluser_data {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
} s_msg_deluser_data;

typedef struct _s_msg_getuserinfo_data {
    uint8_t user_id_heb;  // high eight bits of user_id to get info
    uint8_t user_id_leb;  // low eight bits of user_id to get info
} s_msg_getuserinfo_data;

typedef struct _s_msg_reply_getuserinfo_data {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t user_name[USER_NAME_SIZE];  // 32 Bytes
    uint8_t admin;
} s_msg_reply_getuserinfo_data;

typedef struct _s_msg_reply_all_userid_data {
    uint8_t user_counts;
    uint8_t user_id[100];
} s_msg_reply_all_userid_data;

typedef struct _s_msg_reply_version_data {
    uint8_t version_info[VERSION_INFO_BUFFER_SIZE];  // 32 Bytes
} s_msg_reply_version_data;

typedef struct _s_msg_reply_enroll_data {
    uint8_t user_id_heb;
    uint8_t user_id_leb;
    uint8_t face_direction;  // depleted, user ignore this field
} s_msg_reply_enroll_data;

typedef struct _s_msg_reply_get_hardware_data {
    uint8_t info[32];
} s_msg_reply_get_hardware_data;

typedef struct _s_msg_init_encryption_data_v3 {
    /* 随机数，用于生成加密密钥使用 */
    uint8_t seed[4];
    /* 加密模式。AES加密：01H, 简单加密模式：02H */
    uint8_t mode;
    /* 用于同步模组和主控的系统时间，该数组存储的内容代表当前时区的秒数(1970年至今) */
    uint8_t crttime[4];
    /* 预留参数 */
    uint8_t power_mode;
    /* 预留参数 */
    uint8_t reserved[2];
} s_msg_init_encryption_data_v3;

typedef struct _s_msg_reply_init_encryption_data {
    uint8_t device_id[20];
} s_msg_reply_init_encryption_data;

typedef struct _s_msg_verify_data {
    /* 表示是否在解锁后立刻关机(预留参数)，默认设00 */
    uint8_t pd_rightaway;
    /* timeout为解锁超时时间(单位s)，默认10s，超时时间用户可自行设定，最大不超过255s。主控等待模组应答包超时时间应大于该参数设置值
     */
    uint8_t timeout;
} s_msg_verify_data;

typedef struct _s_note_data_face {
    /* 表示当前人脸状态 */
    int16_t state;
    /* 人脸的在图像中的位置 */
    int16_t left;
    int16_t top;
    int16_t right;
    int16_t bottom;
    /* 偏航角 */
    int16_t yaw;
    /* 俯仰角 */
    int16_t pitch;
    /* 翻滚角 */
    int16_t roll;
} s_note_data_face;

typedef struct _s_msg_enroll_data {
    /* 设置改录入的人为管理员 */
    uint8_t admin;
    /* 录入用户的用户名 */
    uint8_t user_name[32];
    /* 当前录入的人脸方向, 低5位从高到低分别表示人脸方向的朝上、朝下、朝左、朝右和正向 */
    s_face_dir face_direction;
    /* 录入过程的超时时间(单位s)，默认为10s，用户可以随意设置，最大不超过255s。主控等待模组录入应答的超时时间应大于此参数设置值
     */
    uint8_t timeout;
} s_msg_enroll_data;

typedef struct _s_msg_enroll_itg {
    /* 控制该录入用户是否为管理员用户 */
    uint8_t admin;
    /* 录入用户的用户名 */
    uint8_t user_name[USER_NAME_SIZE];
    /* 采用的录入方式，0表示交互式上中下左右5个方向的录入方式、1表示单帧录入方式 */
    uint8_t enroll_type;
    /* 采用单帧录入时，参数无效，可以使用0；采用交互式录入方式可参考交互式录入MID_ENROLL指令中的说明
     */
    s_face_dir face_direction;
    /* 0表示同一个人不能重复录入，1表示同一个人脸能够多次录入；username都可以重复 */
    uint8_t enable_duplicate;
    /* 录入过程的超时时间(单位s)，默认为10s，用户可以随意设置，最大不超过255s。主控等待模组录入应答的超时时间应大于等于此参数设置值
     */
    uint8_t timeout;
    uint8_t reserved[3];
} s_msg_enroll_itg;

typedef struct _s_msg_enroll {
    /* 控制该录入用户是否为管理员用户 */
    uint8_t admin;
    /* 录入用户的用户名 */
    uint8_t user_name[USER_NAME_SIZE];
    /* 采用单帧录入时，参数无效，可以使用0；采用交互式录入方式可参考交互式录入MID_ENROLL指令中的说明
     */
    s_face_dir face_direction;
    /* 录入过程的超时时间(单位s)，默认为10s，用户可以随意设置，最大不超过255s。主控等待模组录入应答的超时时间应大于等于此参数设置值
     */
    uint8_t timeout;
} s_msg_enroll;

typedef struct _s_msg_algo_threshold_level {
    /* level 0~4, safety from low to high, default 2 */
    uint8_t verify_threshold_level;
    /* level 0~4, safety from low to high, default 2 */
    uint8_t liveness_threshold_level;
} s_msg_algo_threshold_level;

typedef struct __protocol_handles_t {
    /* 返回为result 表1-7, pdata中存放验证结果 */
    /**
     * @brief 模组reset，返回时代表模组已经reset
     * @return 成功：MR_SUCCESS
     */
    MR_STATUS_E (*reset)(void);
    /**
     * @brief 返回模组的状态
     * @param pres 存放返回的状态
     * @return 成功：MR_SUCCESS
     */
    MR_STATUS_E (*get_status)(s_msg_reply_getstatus_data* pres);
    /**
     * @brief 模组识别人脸进行比对
     * @param preq 主控的request信息
     * @param pres 需要恢复主控的信息
     * @return 成功：MR_SUCCESS
     */
    MR_STATUS_E (*verify)(s_msg_verify_data* preq, s_msg_reply_verify_data* pres);
    /**
     * @brief 在verify和enroll_single期间，需要实时更新人脸的状态信息，协议层通过该接口获取
     * @param pres 状态信息
     */
    void (*get_face_state)(s_note_data_face* pres);
    /**
     * @brief 单帧录入，仅录入正向人脸即可
     * @param preq
     * @param pres
     * @return
     */
    MR_STATUS_E (*enroll_single)(s_msg_enroll_data* preq, s_msg_reply_enroll_data* pres);
    /**
     * @brief 交互式录入，完成上中下左右5个方向的录入，最终生成一个人脸ID
     * @param preq
     * @param pres
     * @return
     */
    MR_STATUS_E (*enroll)(s_msg_enroll* preq, s_msg_reply_enroll_data* pres);
    /**
     * @brief
     * 该指令是集成了单帧录入指令MID_ENROLL_SINGLE、交互录入指令MID_ENROLL等多种录入方式，录入方式可以在参数结构中指定，并可控制同一张人脸是否能够重复录入
     * @param preq
     * @param pres
     * @return
     */
    MR_STATUS_E (*enroll_itg)(s_msg_enroll_itg* preq, s_msg_reply_enroll_data* pres);
    /**
     * @brief 图片抓拍
     * @param preq 将抓拍到的图片按照起始编号依次向后命名
     * @return 00H(MR_SUCCESS) 抓取图像成功, 04H(MR_FAILED4_CAMERA)
     * Camera启动失败,05H(MR_FAILED4_UNKNOWNREASON) 未知错误 06H(MR_FAILED4_INVALIDPARAM)
     * 指令包传参错误, 14H(MR_FAILED4_WRITE_FILE) 写文件失败
     */
    MR_STATUS_E (*snap_image)(s_msg_snap_image_data* preq);
    /**
     * @brief 获取抓拍图像大小
     * @param preq
     * @return 00H(MR_SUCCESS) 获取图像大小成功, 13H(MR_FAILED4_READ_FILE) 读取图像文件失败
     */
    MR_STATUS_E(*get_saved_iamge)
    (s_msg_get_saved_image_data* preq, s_msg_reply_get_saved_image_data* pres);
    /**
     * @brief 上传抓拍图像数据
     * @param preq
     */
    MR_STATUS_E (*upload_image)(s_msg_upload_image_data* preq);
    /**
     * @brief 获取已注册用户的ID
     * @param pres
     */
    MR_STATUS_E (*get_all_userid)(s_msg_reply_all_userid_data* pres);
    /**
     * @brief 获取算法库版本
     *
     */
    void (*get_library_version)(s_msg_reply_library_version_data* pres);
    /**
     * @brief 删除单个用户
     * @return
     */
    MR_STATUS_E (*del_user)(s_msg_deluser_data* preq);
    /**
     * @brief 删除所有已注册的用户
     * @return
     */
    MR_STATUS_E (*del_all)(void);
    /**
     * @brief 获取指定ID的用户信息
     * @param preq 存放用户信息
     * @return 00H(MR_SUCCESS) 获取用户信息成功, 05H(MR_FAILED4_UNKNOWNREASON) 未知错误,
     * 08H(MR_FAILED4_UNKNOWNUSER) 该ID不存在
     */
    MR_STATUS_E (*get_user_info)(s_msg_getuserinfo_data* preq, s_msg_reply_getuserinfo_data* pres);
    /**
     * @brief 清除人脸录入状态
     * @note
     * 功能说明：重置SDK算法状态，如果正在进行交互式录入MID_ENROLL，发送该指令会重置录入的状态，已录入的方向全部被清空
     * @return
     */
    MR_STATUS_E (*face_reset)(void);
    /**
     * @brief 获取模组固件版本
     * @param pres 返回信息, 携带模组软件版本号的字符串
     * @return MR_SUCCESS 成功; MR_FAILED4_UNKNOWNREASON 失败
     */
    MR_STATUS_E (*get_version)(s_msg_reply_version_data* pres);
    /**
     * @brief 修改算法安全等级
     * @param pres 用于设置liveness和verify的安全等级，可设置四个等级
     * @return MR_SUCCESS 成功; MR_FAILED4_UNKNOWNREASON 失败
     */
    MR_STATUS_E (*set_threshold_level)(s_msg_algo_threshold_level* preq);
    /**
     * @brief 断电前发送该指令，模组收到该指令后，模组保存相应的log
     * @return MR_SUCCESS 成功; MR_FAILED4_UNKNOWNREASON 失败
     */
    MR_STATUS_E (*power_down)(void);
    /**
     * @brief
     * 演示模式仅用于演示使用，安全性较低，任何人都可以解锁。掉电不保存，重启后模组恢复到正常模式。
     * @param mode 01H进入演示模式; 00H退出演示模式
     * @return MR_SUCCESS 成功; MR_FAILED4_UNKNOWNREASON 失败
     */
    MR_STATUS_E (*demo_mode)(uint8_t mode);
    /**
     * @brief 获取硬件信息
     * @return MR_SUCCESS 成功; MR_FAILED4_UNKNOWNREASON 失败
     */
    MR_STATUS_E (*get_hardware)(s_msg_reply_get_hardware_data* pres);
    /**
     * @brief 写入算法认证密钥
     * @return
     */
    MR_STATUS_E (*write_license)(uint8_t* auth_key, uint16_t auth_key_len);
    /**
     * @brief 设置开机抓图RGB和IR的数量
     * @return
     */
    MR_STATUS_E (*set_image_num)(uint8_t rgb_num, uint8_t ir_num);
} protocol_handles_t;

int32_t protocol_init(void);
void register_protocol_handles(protocol_handles_t* handles);

#endif
