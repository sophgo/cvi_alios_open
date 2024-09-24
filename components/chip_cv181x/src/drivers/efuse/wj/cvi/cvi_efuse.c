#include "drv/common.h"
#include "mmio.h"
#include "cvi_efuse.h"

#define EFUSE_BASE      0x03050000
#define EFUSE_STATUS    0x010
#define EFUSE_MODE      0x000
#define EFUSE_ADR       0x004
#define EFUSE_RD_DATA   0x00c
#define EFUSE_FTSN0     0x100
#define EFUSE_FTSN1     0x104
#define EFUSE_SW_INFO   0x12c

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

csi_error_t cvi_efuse_disable_uart_dl()
{
    csi_error_t ret = CSI_OK;

    ret = cvi_efuse_program_bit(0xb, 27);

    return ret;
}

csi_error_t cvi_efuse_is_uart_dl_enable()
{
    csi_error_t ret = CSI_OK;
    uint32_t data = 0;

    ret = cvi_efuse_read_word_from_shadow(0xb, &data);
    if (ret != CSI_OK)  {
        printf("cvi_efuse_read_word_from_shadow failed\n");
        return CSI_ERROR;
    }
    
    if ((data & (1 << 27)) == 0) {
        return CSI_ERROR;
    }

    return ret;
}

csi_error_t cvi_efuse_set_sd_dl_button()
{
    csi_error_t ret = CSI_OK;
    uint32_t data = 0;

    ret = cvi_efuse_read_word_from_shadow(0xb, &data);
    if (ret != CSI_OK)  {
        printf("cvi_efuse_read_word_from_shadow failed\n");
        return CSI_ERROR;
    }

    if ((data & (1 << 23)) != 0) {
        return CSI_ERROR;
    }

    ret = cvi_efuse_program_bit(0xb, 22);

    return ret;
}

csi_error_t cvi_efuse_is_sd_dl_button()
{
    csi_error_t ret = CSI_OK;
    uint32_t data = 0;

    ret = cvi_efuse_read_word_from_shadow(0xb, &data);
    if (ret != CSI_OK) {
        printf("cvi_efuse_read_word_from_shadow failed\n");
        return CSI_ERROR;
    }

    if ((data & (1 << 23)) != 0 || (data & (1 << 22)) == 0) {
        return CSI_ERROR;
    }

    return ret;
}

CVI_S32 cvi_efuse_get_chip_sn(void *data, uint32_t *size)
{
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(size, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    uint32_t uid_3 = 0xDEAFBEEF;
    uint32_t uid_4 = 0xDEAFBEEF;

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
