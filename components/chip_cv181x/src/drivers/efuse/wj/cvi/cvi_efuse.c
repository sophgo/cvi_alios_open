#include "drv/common.h"
#include "mmio.h"
#include "cvi_efuse.h"

#define EFUSE_BASE      0x03050000
#define EFUSE_SHADOW_REG (EFUSE_BASE + 0x100)
#define SEC_EFUSE_SHADOW_REG (0x020C0100)
#define EFUSE_STATUS    0x010
#define EFUSE_MODE      0x000
#define EFUSE_ADR       0x004
#define EFUSE_RD_DATA   0x00c
#define EFUSE_FTSN0     0x100
#define EFUSE_FTSN1     0x104
#define EFUSE_SW_INFO   0x12c

#define EFUSE_SIZE      0x100

#define CVI_EFUSE_LOCK_ADDR 0xF8

#define ARRAY_SIZE(a)  (sizeof(a)/sizeof(a[0]))

static struct _CVI_EFUSE_AREA_S {
    CVI_U32 addr;
    CVI_U32 size;
} cvi_efuse_area[] = { [CVI_EFUSE_AREA_USER] = { 0x40, 40 },
                       [CVI_EFUSE_AREA_DEVICE_ID] = { 0x8c, 8 },
                       [CVI_EFUSE_AREA_HASH0_PUBLIC] = { 0xA8, 32 },
                       [CVI_EFUSE_AREA_LOADER_EK] = { 0xD8, 16 },
                       [CVI_EFUSE_AREA_DEVICE_EK] = { 0xE8, 16 },
                       [CVI_EFUSE_AREA_CHIP_SN] = { 0x0C, 8 } };

static struct _CVI_EFUSE_AREA_USER_S {
    CVI_U32 addr;
    CVI_U32 size;
} cvi_efuse_area_user[] = {
    { 0x40, 4 },
    { 0x48, 4 },
    { 0x50, 4 },
    { 0x58, 4 },
    { 0x60, 4 },
    { 0x68, 4 },
    { 0x70, 4 },
    { 0x78, 4 },
    { 0x80, 4 },
    { 0x88, 4 },
};

static struct _CVI_EFUSE_LOCK_S {
    CVI_U32 wlock_shift;
    CVI_U32 rlock_shift;
} cvi_efuse_lock[] = { [CVI_EFUSE_LOCK_HASH0_PUBLIC] = { 0, 8 },
                       [CVI_EFUSE_LOCK_LOADER_EK] = { 4, 12 },
                       [CVI_EFUSE_LOCK_DEVICE_EK] = { 6, 14 } };


csi_error_t cvi_efuse_wait_idle()
{
    uint32_t status;
    uint32_t time_count = 0;
    csi_error_t ret = CSI_OK;

    do {
        status = mmio_read_32(EFUSE_BASE + EFUSE_STATUS);
        time_count++;

        if (time_count > 0x1000) {
            printf("wait idle timeout\n");
            return CSI_ERROR;
        }
    } while ((status & 0x1) != 0);

    return ret;
}

csi_error_t cvi_efuse_pwr_on()
{
    csi_error_t ret = CSI_OK;

    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_wait_idle failed\n");
        return CSI_ERROR;
    }

    mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x10);

    return ret;
}

csi_error_t cvi_efuse_refresh_shadow()
{
    csi_error_t ret = CSI_OK;

    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_wait_idle failed\n");
        return CSI_ERROR;
    }

    mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x30);

    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_wait_idle failed\n");
        return CSI_ERROR;
    }

    return ret;
}

// read one line (4 bytes) from efuse
csi_error_t cvi_efuse_read_word_from_efuse(uint32_t addr, uint32_t *data)
{
    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_wait_idle failed\n");
        return CSI_ERROR;
    }

    mmio_write_32(EFUSE_BASE + EFUSE_ADR, (addr << 1));
    mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x12);

    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_wait_idle failed\n");
        return CSI_ERROR;
    }

    *data = mmio_read_32(EFUSE_BASE + EFUSE_RD_DATA);
    return CSI_OK;
}

// read one line (4 bytes) from shadow
csi_error_t cvi_efuse_read_word_from_shadow(uint32_t addr, uint32_t *data)
{
    *data = mmio_read_32(EFUSE_BASE + EFUSE_FTSN0 + (addr << 2));
    return CSI_OK;
}

static CVI_S32 cvi_efuse_read_from_sec_shadow(CVI_U32 addr, CVI_U32 *data)
{
    if (addr >= EFUSE_SIZE)
        return CVI_FAILURE_ILLEGAL_PARAM;

    if (addr % 4 != 0)
        return CVI_FAILURE_ILLEGAL_PARAM;

    *data = mmio_read_32(SEC_EFUSE_SHADOW_REG + addr);

    return CVI_SUCCESS;
}

// program 4 bytes
csi_error_t cvi_efuse_program_word(uint32_t addr, const uint32_t data)
{
    int i;
    uint32_t exist_data = 0;

    cvi_efuse_read_word_from_shadow(addr, &exist_data);
    exist_data = (~exist_data) & data; 

    if (exist_data == 0) {
        printf("no data need write\n");
        return CSI_OK;        
    }

    for (i = 0; i < 32; i++) {
        int bit = exist_data & (1 << i);

        if (bit) {
            if (cvi_efuse_wait_idle() != CSI_OK) {
                printf("cvi_efuse_wait_idle failed\n");
                return CSI_ERROR;
            }

            uint16_t w_addr = (i << 7) | ((addr & 0x3F) << 1);
            mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
            mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x14);

            if (cvi_efuse_wait_idle() != CSI_OK) {
                printf("cvi_efuse_wait_idle failed\n");
                return CSI_ERROR;
            }

            w_addr |= 0x1;
            mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
            mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x14);
        }
    }

    cvi_efuse_refresh_shadow();

    return CSI_OK;
}

// program 1 bit
csi_error_t cvi_efuse_program_bit(uint32_t addr, const uint32_t bit)
{
    uint32_t exist_data = 0;

    cvi_efuse_read_word_from_shadow(addr, &exist_data);

    if (exist_data & (1 << bit)) {
        // printf("no bit need write\n");
        return CSI_OK;        
    }

    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_wait_idle failed\n");
        return CSI_ERROR;
    }

    uint16_t w_addr = (bit << 7) | ((addr & 0x3F) << 1);
    mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
    mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x14);

    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_wait_idle failed\n");
        return CSI_ERROR;
    }

    w_addr |= 0x1;
    mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
    mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x14);

    cvi_efuse_refresh_shadow();

    return CSI_OK;
}

csi_error_t cvi_efuse_pwr_off()
{
    csi_error_t ret = CSI_OK;

    if (cvi_efuse_wait_idle() != CSI_OK) {
        printf("cvi_efuse_wait_idle failed\n");
        return CSI_ERROR;
    }

    mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x18);

    return ret;
}

csi_error_t cvi_efuse_sd_dl_config(enum SD_USB_UART_DL_MODE_E mode)
{
    csi_error_t ret = CSI_OK;
    uint32_t data = 0;

    ret = cvi_efuse_read_word_from_shadow(0xb, &data);
    if (ret != CSI_OK) {
        printf("cvi_efuse_read_word_from_shadow failed\n");
        return ret;
    }

    data = (data & (0x3 << EFUSE_SW_INFO_SD_DL_SHIFT)) >> EFUSE_SW_INFO_SD_DL_SHIFT;
    switch (mode) {
    case SD_USB_UART_DL_ENABLE:
        switch (data) {
        case 0x0:
            break;
        case 0x1:
            ret = cvi_efuse_program_bit(0xb, 23);
            break;
        case 0x2:
            ret = cvi_efuse_program_bit(0xb, 22);
            break;
        case 0x3:
            break;
        default:
            ret = CSI_ERROR;
            break;
        }
        break;
    case SD_USB_UART_DL_FASTBOOT:
        switch (data) {
        case 0x0:
            ret = cvi_efuse_program_bit(0xb, 22);
            break;
        case 0x1:
            break;
        case 0x2:
            ret = CSI_ERROR;
            break;
        case 0x3:
            ret = CSI_ERROR;
            break;
        default:
            ret = CSI_ERROR;
            break;
        }
        break;
    case SD_USB_UART_DL_DISABLE:
        switch (data) {
        case 0x0:
            ret = cvi_efuse_program_bit(0xb, 23);
            break;
        case 0x1:
            ret = CSI_ERROR;
            break;
        case 0x2:
            break;
        case 0x3:
            ret = CSI_ERROR;
            break;
        default:
            ret = CSI_ERROR;
            break;
        }
        break;
    default:
        break;
    }

    return ret;
}

uint32_t cvi_efuse_get_sd_dl_config()
{
    csi_error_t ret = CSI_OK;
    uint32_t data = 0;

    ret = cvi_efuse_read_word_from_shadow(0xb, &data);
    if (ret != CSI_OK) {
        printf("cvi_efuse_read_word_from_shadow failed\n");
        return ret;
    }

    data = (data & (0x3 << EFUSE_SW_INFO_SD_DL_SHIFT)) >> EFUSE_SW_INFO_SD_DL_SHIFT;
    return data;
}

uint32_t cvi_efuse_get_usb_dl_config()
{
    csi_error_t ret = CSI_OK;
    uint32_t data = 0;

    ret = cvi_efuse_read_word_from_shadow(0xb, &data);
    if (ret != CSI_OK) {
        printf("cvi_efuse_read_word_from_shadow failed\n");
        return ret;
    }

    data = (data & (0x3 << EFUSE_SW_INFO_USB_DL_SHIFT)) >> EFUSE_SW_INFO_USB_DL_SHIFT;
    return data;
}

uint32_t cvi_efuse_get_uart_dl_config()
{
    csi_error_t ret = CSI_OK;
    uint32_t data = 0;

    ret = cvi_efuse_read_word_from_shadow(0xb, &data);
    if (ret != CSI_OK) {
        printf("cvi_efuse_read_word_from_shadow failed\n");
        return ret;
    }

    data = (data & (0x3 << EFUSE_SW_INFO_UART_DL_SHIFT)) >> EFUSE_SW_INFO_UART_DL_SHIFT;
    return data;
}

csi_error_t cvi_efuse_usb_dl_config(enum SD_USB_UART_DL_MODE_E mode)
{
    csi_error_t ret = CSI_OK;
    uint32_t data = 0;

    ret = cvi_efuse_read_word_from_shadow(0xb, &data);
    if (ret != CSI_OK) {
        printf("cvi_efuse_read_word_from_shadow failed\n");
        return ret;
    }

    data = (data & (0x3 << EFUSE_SW_INFO_USB_DL_SHIFT)) >> EFUSE_SW_INFO_USB_DL_SHIFT;
    switch (mode) {
    case SD_USB_UART_DL_ENABLE:
        switch (data) {
        case 0x0:
            break;
        case 0x1:
            ret = cvi_efuse_program_bit(0xb, 25);
            break;
        case 0x2:
            ret = cvi_efuse_program_bit(0xb, 24);
            break;
        case 0x3:
            break;
        default:
            ret = CSI_ERROR;
            break;
        }
        break;
    case SD_USB_UART_DL_FASTBOOT:
        switch (data) {
        case 0x0:
            ret = cvi_efuse_program_bit(0xb, 24);
            break;
        case 0x1:
            break;
        case 0x2:
            ret = CSI_ERROR;
            break;
        case 0x3:
            ret = CSI_ERROR;
            break;
        default:
            ret = CSI_ERROR;
            break;
        }
        break;
    case SD_USB_UART_DL_DISABLE:
        switch (data) {
        case 0x0:
            ret = cvi_efuse_program_bit(0xb, 25);
            break;
        case 0x1:
            ret = CSI_ERROR;
            break;
        case 0x2:
            break;
        case 0x3:
            ret = CSI_ERROR;
            break;
        default:
            ret = CSI_ERROR;
            break;
        }
        break;
    default:
        break;
    }

    return ret;
}

csi_error_t cvi_efuse_uart_dl_config(enum SD_USB_UART_DL_MODE_E mode)
{
    csi_error_t ret = CSI_OK;
    uint32_t data = 0;

    ret = cvi_efuse_read_word_from_shadow(0xb, &data);
    if (ret != CSI_OK) {
        printf("cvi_efuse_read_word_from_shadow failed\n");
        return ret;
    }

    data = (data & (0x3 << EFUSE_SW_INFO_UART_DL_SHIFT)) >> EFUSE_SW_INFO_UART_DL_SHIFT;
    switch (mode) {
    case SD_USB_UART_DL_ENABLE:
        switch (data) {
        case 0x0:
            break;
        case 0x1:
            ret = cvi_efuse_program_bit(0xb, 27);
            break;
        case 0x2:
            ret = cvi_efuse_program_bit(0xb, 26);
            break;
        case 0x3:
            break;
        default:
            ret = CSI_ERROR;
            break;
        }
        break;
    case SD_USB_UART_DL_FASTBOOT:
        switch (data) {
        case 0x0:
            ret = cvi_efuse_program_bit(0xb, 26);
            break;
        case 0x1:
            break;
        case 0x2:
            ret = CSI_ERROR;
            break;
        case 0x3:
            ret = CSI_ERROR;
            break;
        default:
            ret = CSI_ERROR;
            break;
        }
        break;
    case SD_USB_UART_DL_DISABLE:
        switch (data) {
        case 0x0:
            ret = cvi_efuse_program_bit(0xb, 27);
            break;
        case 0x1:
            ret = CSI_ERROR;
            break;
        case 0x2:
            break;
        case 0x3:
            ret = CSI_ERROR;
            break;
        default:
            ret = CSI_ERROR;
            break;
        }
        break;
    default:
        break;
    }

    return ret;
}

CVI_S32 cvi_efuse_get_chip_sn(void *data, CVI_U32 *size)
{
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    CVI_U32 uid_3 = 0xDEAFBEEF;
    CVI_U32 uid_4 = 0xDEAFBEEF;

    ret = cvi_efuse_read_word_from_shadow(0x3, &uid_3);
    ret |= cvi_efuse_read_word_from_shadow(0x4, &uid_4);
    if (ret != CSI_OK) {
        printf("cvi_efuse_read_word_from_shadow failed\n");
        return CSI_ERROR;
    }

    ((uint32_t *)data)[0] = uid_3;
    ((uint32_t *)data)[1] = uid_4;
    *size = 8;

    return *size;
}

CVI_S32 _CVI_EFUSE_Read(u32 addr, void *buf, size_t buf_size)
{
    CVI_S32 ret = -1;
    CVI_U32 data;
    int i;

    if (!buf)
        return CVI_FAILURE_ILLEGAL_PARAM;

    if (buf_size > EFUSE_SIZE)
        buf_size = EFUSE_SIZE;

    memset(buf, 0, buf_size);

    for (i = 0; i < buf_size; i += 4) {
        ret = cvi_efuse_read_from_sec_shadow(addr + i, &data);
        if (ret < 0)
            return ret;

        memcpy(buf + i, &data, sizeof(data));
    }

    return buf_size;
}

CVI_S32 _CVI_EFUSE_Write(u32 addr, const CVI_U8 *buf, size_t buf_size)
{
    CVI_S32 ret = -1;
    int i;

    if (!buf)
        return CVI_FAILURE_ILLEGAL_PARAM;

    if (buf_size > EFUSE_SIZE)
        buf_size = EFUSE_SIZE;

    for (i = 0; i < buf_size; i += 4) {
        ret = cvi_efuse_program_word((addr + i)/4, *(CVI_U32 *)(buf + i));
        if (ret < 0)
            return ret;
    }

    return CVI_SUCCESS;
}

CVI_S32 CVI_EFUSE_GetSize(CVI_EFUSE_AREA_E area, CVI_U32 *size)
{
    if (area >= ARRAY_SIZE(cvi_efuse_area) ||
        cvi_efuse_area[area].size == 0) {
        printf("area (%d) is not found\n", area);
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    if (size)
        *size = cvi_efuse_area[area].size;

    return 0;
}

CVI_S32 CVI_EFUSE_Read(CVI_EFUSE_AREA_E area, CVI_U8 *buf, CVI_U32 buf_size)
{
    CVI_U32 user_size = cvi_efuse_area[CVI_EFUSE_AREA_USER].size;
    CVI_U8 user[user_size], *p;
    CVI_S32 ret;
    size_t i;

    if (area >= ARRAY_SIZE(cvi_efuse_area) ||
        cvi_efuse_area[area].size == 0) {
        printf("area (%d) is not found\n", area);
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    if (!buf)
        return CVI_FAILURE_ILLEGAL_PARAM;

    memset(buf, 0, buf_size);

    if (buf_size > cvi_efuse_area[area].size)
        buf_size = cvi_efuse_area[area].size;

    if (area != CVI_EFUSE_AREA_USER)
        return _CVI_EFUSE_Read(cvi_efuse_area[area].addr, buf, buf_size);

    memset(user, 0, user_size);

    p = user;
    for (i = 0; i < ARRAY_SIZE(cvi_efuse_area_user); i++) {
        ret = _CVI_EFUSE_Read(cvi_efuse_area_user[i].addr, p,
                                cvi_efuse_area_user[i].size);
        if (ret < 0)
                return ret;
        p += cvi_efuse_area_user[i].size;
    }

    memcpy(buf, user, buf_size);

    return CVI_SUCCESS;
}

CVI_S32 CVI_EFUSE_Write(CVI_EFUSE_AREA_E area, const CVI_U8 *buf,
                        CVI_U32 buf_size)
{
    CVI_U32 user_size = cvi_efuse_area[CVI_EFUSE_AREA_USER].size;
    CVI_U8 user[user_size], *p;
    CVI_S32 ret;
    size_t i;

    if (area >= ARRAY_SIZE(cvi_efuse_area) ||
        cvi_efuse_area[area].size == 0) {
        printf("area (%d) is not found\n", area);
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    if (!buf)
        return CVI_FAILURE_ILLEGAL_PARAM;

    if (buf_size > cvi_efuse_area[area].size)
        buf_size = cvi_efuse_area[area].size;

    if (area != CVI_EFUSE_AREA_USER) {
        return _CVI_EFUSE_Write(cvi_efuse_area[area].addr, buf,
                                buf_size);
    }

    memset(user, 0, user_size);
    memcpy(user, buf, buf_size);

    p = user;
    for (i = 0; i < ARRAY_SIZE(cvi_efuse_area_user); i++) {
        ret = _CVI_EFUSE_Write(cvi_efuse_area_user[i].addr, p,
                                cvi_efuse_area_user[i].size);
        if (ret < 0)
            return ret;

        p += cvi_efuse_area_user[i].size;
    }

    return CVI_SUCCESS;
}

CVI_S32 CVI_EFUSE_Lock(CVI_EFUSE_LOCK_E lock)
{
    CVI_U32 value = 0;
    CVI_S32 ret = 0;

    if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
        printf("lock (%d) is not found\n", lock);
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    value = 0x3 << cvi_efuse_lock[lock].wlock_shift;
    value |= 0x3 << cvi_efuse_lock[lock].rlock_shift;
    ret = _CVI_EFUSE_Write(CVI_EFUSE_LOCK_ADDR, (CVI_U8 *)&value, sizeof(value));

    return ret;
}

CVI_S32 CVI_EFUSE_IsLocked(CVI_EFUSE_LOCK_E lock)
{
    CVI_S32 ret = 0;
    CVI_U32 value = 0, lock_value = 0;

    if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
        printf("lock (%d) is not found\n", lock);
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    ret = _CVI_EFUSE_Read(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
    //printf("ret=%d value=%u\n", ret, value);
    if (ret < 0)
        return ret;

    lock_value = 0x3 << cvi_efuse_lock[lock].wlock_shift;
    lock_value |= 0x3 << cvi_efuse_lock[lock].rlock_shift;

    return ((value & lock_value) == lock_value)?1:0;
}

CVI_S32 CVI_EFUSE_LockWrite(CVI_EFUSE_LOCK_E lock)
{
    CVI_U32 value = 0;
    CVI_S32 ret = 0;

    if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
        printf("lock (%d) is not found\n", lock);
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    value = 0x3 << cvi_efuse_lock[lock].wlock_shift;
    ret = _CVI_EFUSE_Write(CVI_EFUSE_LOCK_ADDR, (CVI_U8 *)&value, sizeof(value));
    return ret;
}

CVI_S32 CVI_EFUSE_IsWriteLocked(CVI_EFUSE_LOCK_E lock)
{
    CVI_S32 ret = 0;
    CVI_U32 value = 0;

    if (lock >= ARRAY_SIZE(cvi_efuse_lock)) {
        printf("lock (%d) is not found\n", lock);
        return CVI_FAILURE_ILLEGAL_PARAM;
    }

    ret = _CVI_EFUSE_Read(CVI_EFUSE_LOCK_ADDR, &value, sizeof(value));
    //CVI_TRACE_SYS(CVI_DBG_DEBUG, "ret=%d value=%u\n", ret, value);
    if (ret < 0)
        return ret;

    value &= 0x3 << cvi_efuse_lock[lock].wlock_shift;
    return !!value;
}
