#include "capture_image_util.h"
#include <aos/cli.h>
#include <aos/kernel.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "cvi_util.h"
#include "md5.h"

static int32_t s32_image_num     = 0;
static int32_t s32_image_rgb_num = 0;
static int32_t s32_image_ir_num  = 0;

/**
 * @brief   the number of images to be stored.
 * @note
 * @param u8_rgb_num: rgb image number.
 * @param u8_rgb_num: ir image number.
 *
 * @retval  None
 */
void cvi_capture_image_set_num(uint8_t u8_rgb_num, uint8_t u8_ir_num)
{
    s32_image_rgb_num = u8_rgb_num;
    s32_image_ir_num  = u8_ir_num;
    s32_image_num     = s32_image_rgb_num + s32_image_ir_num;
    LOG_UTIL(UTIL_DBUG, "image_rgb_num = %d, image_ir_num = %d, image_num = %d\n",
             s32_image_rgb_num, s32_image_ir_num, s32_image_num);
}

/**
 * @brief   buffer to save image data.
 * @note
 * @retval  -1 if image number is zero, otherwise return 0
 */
int32_t cvi_capture_image_init()
{
    int32_t s32_ret                      = CVI_FAILURE;
    cvi_tcp_frm_info_t* pst_tcp_frm_info = cvi_tcp_get_frm_info();
    if (pst_tcp_frm_info == NULL) {
        LOG_UTIL(UTIL_DBUG, "tcp_frm_info is null\n");
        s32_ret = CVI_FAILURE;
        return s32_ret;
    }
    LOG_UTIL(UTIL_DBUG, "image_num %d\n", s32_image_num);
    if (s32_image_num > 0) {
        s32_ret = CVI_SUCCESS;
    } else {
        return s32_ret;
    }

    pst_tcp_frm_info->s32_image_num = s32_image_num;
    pst_tcp_frm_info->s32_frm_idx   = 0;
    pst_tcp_frm_info->s32_frm_state = 0;
    for (int i = 0; i < TCP_IMG_NUM_MAX; i++) {
        pst_tcp_frm_info->u8_img_type[i] = 0;
    }
    pst_tcp_frm_info->pu8_frm_buf = (uint8_t*)malloc(TCP_IMG_SIZE * s32_image_num);
    return s32_ret;
}

/**
 * @brief   the image data in the buffer.
 * @note
 * @param   e_img_type: image type include ir、rgb or none.
 * @param   pu8_buf: save image data.
 * @param   u32_len: image buffer size.
 * @retval  -1 if can't save image data, otherwise return 0
 */
int32_t cvi_capture_image_set_buf(cvi_tcp_img_type_e e_img_type, const uint8_t* pu8_buf,
                                  const uint32_t u32_len)
{
    int32_t s32_ret                      = CVI_FAILURE;
    cvi_tcp_frm_info_t* pst_tcp_frm_info = cvi_tcp_get_frm_info();
    if (pst_tcp_frm_info == NULL) {
        LOG_UTIL(UTIL_DBUG, "tcp_frm_info is null\n");
        return s32_ret;
    }

    if (!pst_tcp_frm_info->pu8_frm_buf || pst_tcp_frm_info->s32_image_num == 0) {
        return s32_ret;
    }
    LOG_UTIL(UTIL_DBUG, "frm_state  %d\n", pst_tcp_frm_info->s32_frm_state);
    LOG_UTIL(UTIL_DBUG, "frm_idx %d\n", pst_tcp_frm_info->s32_frm_idx);
    LOG_UTIL(UTIL_DBUG, "image_rgb_num = %d, image_ir_num = %d, image_num = %d\n",
             s32_image_rgb_num, s32_image_ir_num, s32_image_num);

    if (e_img_type == TCP_IMG_RGB || e_img_type == TCP_IMG_IR) {
        if (s32_image_ir_num == 0 && e_img_type == TCP_IMG_IR) {
            return s32_ret;
        }

        if (s32_image_rgb_num == 0 && e_img_type == TCP_IMG_RGB) {
            return s32_ret;
        }

        size_t frm_offset = pst_tcp_frm_info->s32_frm_idx * TCP_IMG_SIZE;

        CVI_U8 au8Md5Value[MD5_STRING_LEN] = {0};
        calcute_md5_value(pu8_buf, TCP_IMG_SIZE, au8Md5Value);
        if (e_img_type == TCP_IMG_RGB) {
            LOG_UTIL(UTIL_INFO, "dump_RGB md5 is: %s\n", au8Md5Value);
        } else if (e_img_type == TCP_IMG_IR) {
            LOG_UTIL(UTIL_INFO, "dump_IR md5 is: %s\n", au8Md5Value);
        }

        memcpy(pst_tcp_frm_info->pu8_frm_buf + frm_offset, pu8_buf, TCP_IMG_SIZE);
        pst_tcp_frm_info->u8_img_type[pst_tcp_frm_info->s32_frm_idx] = e_img_type;

        pst_tcp_frm_info->s32_frm_state++;
        // 判断当前buf是否已满
        if (pst_tcp_frm_info->s32_frm_state >= s32_image_num) {
            pst_tcp_frm_info->s32_frm_state = pst_tcp_frm_info->s32_image_num;
        }

        if (pst_tcp_frm_info->s32_frm_state >= s32_image_num) {
            if (pst_tcp_frm_info->s32_frm_idx >= s32_image_num - 1) {
                pst_tcp_frm_info->s32_frm_idx = 0;
            } else {
                pst_tcp_frm_info->s32_frm_idx++;
            }
        } else {
            pst_tcp_frm_info->s32_frm_idx++;
        }
    }
    return CVI_SUCCESS;
}
