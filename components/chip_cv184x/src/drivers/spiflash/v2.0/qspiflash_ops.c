/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     qspi_device.c
 * @brief
 * @version
 * @date     2020-03-18
 ******************************************************************************/
#include <string.h>

#include <drv/spiflash.h>
#include <drv/cvi_irq.h>
#include <drv/tick.h>

#include "spiflash_vendor.h"

/* communication parameters */
#define DEFUALT_QSPI_DEVICE_TX_RX_TIMEOUT        0x100000U
#define DEFUALT_QSPI_DEVICE_CLOCK         6250000
#define DEFUALT_QSPI_DEVICE_PHASE         QSPI_CLOCK_MODE_0
#define DEFUALT_QSPI_DEVICE_SALVE_IDX     0U

#define MAX_TRANSFER_SIZE_ONCE            8U

#define FLASH_CMD_WRITE_STATUS       0x01U
#define FLASH_CMD_READ_STATUS        0x05U
#define FLASH_CMD_WRITE_ENABLE       0x06U
#define FLASH_CMD_READ_JEDEC_ID      0x9FU
#define FLASH_CMD_SECTOR_ERASE       0x20U
#define FLASH_CMD_CHIP_ERASE         0x60U
#define FLASH_CMD_PAGE_PROGRAM       0x02U
#define FLASH_CMD_READ_DATA          0x03U
#define FLASH_CMD_FAST_READ_DUAL_IO  0xbbU
#define FLASH_CMD_FAST_READ_QUAL_IO  0xebU
#define FLASH_CMD_SECTOR_ERASE       0x20U
#define FALSH_CMD_POWER_DOWN         0xB9U
#define FALSH_CMD_RELEASE_POWER_DOWN 0xABU

#if defined(CONFIG_XIP)
#define ATTRIBUTE_DATA __attribute__((section(".ram.code")))
#else
#define ATTRIBUTE_DATA
#endif

static csi_qspi_command_t command;
#define FLASH_PAGE_MAX_SZ            (0x100U)
static uint8_t g_page_buf[FLASH_PAGE_MAX_SZ]__attribute__((aligned(64)));

ATTRIBUTE_DATA int32_t get_flash_status(csi_qspi_t *qspi, uint8_t *status)
{
    int32_t ret = 0;

    /* transmit READ_STATUS command */
    command.instruction.value     = FLASH_CMD_READ_STATUS;
    command.instruction.bus_width = QSPI_CFG_BUS_SINGLE;
    command.instruction.disabled  = (bool)false;
    command.alt.disabled          = (bool)true;
    command.address.disabled      = (bool)true;
    command.data.bus_width        = QSPI_CFG_BUS_SINGLE;

    /* receive status */
    ret = csi_qspi_receive(qspi, &command,  status, 1U, DEFUALT_QSPI_DEVICE_TX_RX_TIMEOUT);

    if (ret != 1) {
        ret = -1;
    } else {
        ret = 0;
    }

    return ret;
}

ATTRIBUTE_DATA int32_t flash_wait_ready(csi_qspi_t *qspi)
{
    uint32_t timeout = DEFUALT_QSPI_DEVICE_TX_RX_TIMEOUT;
    uint8_t  status;
    int32_t  ret = -1;

    while (timeout-- > 0U) {
        ret = get_flash_status(qspi, &status);

        if ((ret == 0) && ((status & 0x01U) == 0U)) {
            ret = 0;
            break;
        }
    }

    return ret;
}

ATTRIBUTE_DATA int32_t qspi_send_and_wait(csi_qspi_t *qspi, csi_qspi_command_t *command, const void *data, uint32_t size)
{
    int32_t  ret;
    uint32_t count = 0U;
    uint32_t result;

    do {
        result = csi_irq_save();
        ret = csi_qspi_send(qspi, command, data, size, DEFUALT_QSPI_DEVICE_TX_RX_TIMEOUT);

        if (ret < 0) {
            break;
        }

        count = (uint32_t)ret;

        ret = flash_wait_ready(qspi);
        csi_irq_restore(result);

    } while (0);

    if (ret >= 0) {
        ret = (int32_t)count;
    }

    return ret;
}

static int32_t send(void *spiflash, uint8_t cmd, uint32_t addr, uint32_t addr_size, const void *data, uint32_t size)
{
    int32_t  ret = 0;
    csi_spiflash_t *handle;
    uint32_t count = 0U;

    handle = (csi_spiflash_t *)spiflash;
    handle->spi_cs_callback(GPIO_PIN_LOW);

    do {
        command.instruction.value     = cmd;
        command.instruction.bus_width = QSPI_CFG_BUS_SINGLE;
        command.instruction.disabled  = (bool)false;
        command.alt.disabled          = (bool)true;
        command.address.disabled      = (addr_size > 0U) ? (bool)false : (bool)true;
        command.address.value         = addr;
        command.address.size          = addr_size;

        ret = csi_qspi_send(&handle->spi_qspi.qspi, &command, data, size, DEFUALT_QSPI_DEVICE_TX_RX_TIMEOUT);
        if(ret < 0){
            break;
        }
        count = ret;
    } while (0);
    handle->spi_cs_callback(GPIO_PIN_HIGH);

    // return actual count if count != 0
    if (count != 0U) {
        ret = (int32_t)count;
    }

    return ret;
}

static int32_t receive(void *spiflash, uint8_t cmd, uint32_t addr, uint32_t addr_size, void *data, uint32_t size)
{
    int32_t  ret = 0;
    csi_spiflash_t *handle;

    handle = (csi_spiflash_t *)spiflash;
    handle->spi_cs_callback(GPIO_PIN_LOW);

    command.instruction.value     = cmd;
    command.instruction.bus_width = QSPI_CFG_BUS_SINGLE;
    command.instruction.disabled  = (bool)false;
    command.alt.disabled          = (bool)true;
    command.address.disabled      = (addr_size > 0U) ? (bool)false : (bool)true;
    command.address.value         = addr;
    command.address.size          = addr_size;
    ret = csi_qspi_receive(&handle->spi_qspi.qspi, &command, data, size, DEFUALT_QSPI_DEVICE_TX_RX_TIMEOUT);
    handle->spi_cs_callback(GPIO_PIN_HIGH);

    return ret;
}

static csi_error_t set_cmd(void *spiflash, csi_spiflash_cmd_t *cmd){
    command.address.bus_width     = cmd->addr.buswidth;
    command.data.bus_width        = cmd->data.buswidth;
    command.dummy_count           = cmd->dummy.nbytes;
    return CSI_OK;
}

csi_error_t csi_spiflash_qspi_init(csi_spiflash_t *spiflash, uint32_t qspi_idx, void *qspi_cs_callback)
{
    CSI_PARAM_CHK(spiflash, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    uint32_t    flash_id = 0U;
    uint32_t    addr, addr_size, data_size;
    uint32_t    data = 0U;
    uint8_t     cmd;
    csi_spiflash_cmd_t   cfg;
    csi_spiflash_param_t *param;

    spiflash->spi_cs_callback = qspi_cs_callback;
    spiflash->set_cmd         = set_cmd;
    spiflash->spi_send        = send;
    spiflash->spi_receive     = receive;
    memset(&command, (int)0, sizeof(csi_qspi_command_t));
    spiflash->spi_cs_callback(GPIO_PIN_HIGH);

    do {
        ret = csi_qspi_init(&spiflash->spi_qspi.qspi, qspi_idx);

        if (ret != CSI_OK) {
            ret = CSI_ERROR;
            break;
        }

        /* set spi mode 0 */
        ret = csi_qspi_mode(&spiflash->spi_qspi.qspi, DEFUALT_QSPI_DEVICE_PHASE);
        if(ret){
            return ret;
        }  

        ret = csi_qspi_frequence(&spiflash->spi_qspi.qspi, DEFUALT_QSPI_DEVICE_CLOCK);
        if(!ret){
            return ret;
        }

#ifdef CONFIG_PM
        ret = csi_spiflash_release_power_down(spiflash);

        if (ret != CSI_OK) {
            ret = CSI_ERROR;
            break;
        }

#endif

        cmd       = FLASH_CMD_READ_JEDEC_ID;
        addr      = 0U;
        addr_size = 0U;
        data_size = 3U;

        cfg.data.buswidth=QSPI_CFG_BUS_SINGLE;
        cfg.addr.buswidth=QSPI_CFG_BUS_SINGLE;
        cfg.dummy.nbytes=0;
        spiflash->set_cmd(spiflash, &cfg);
        ret = (csi_error_t)spiflash->spi_receive(spiflash, cmd, addr, addr_size, &data, data_size);

        if (ret < 0) {
            break;
        }

        flash_id  = ((data & 0x000000FFU) << 16) |
                    ((data & 0x0000FF00U))       |
                    ((data & 0x00FF0000U) >> 16);

        ret = get_spiflash_vendor_param(flash_id, &spiflash->flash_prv_info);

        if (ret != CSI_OK) {
            ret = CSI_UNSUPPORTED;
        }

        param = (csi_spiflash_param_t *)spiflash->flash_prv_info;
        spiflash->xfer.xfer_buf = g_page_buf;
        spiflash->xfer.xfer_buf_len = param->page_size;
		if(param->page_size < sizeof(g_page_buf)) {
			return CSI_ERROR;
		}
    } while (0);

    return ret;
}

void csi_spiflash_qspi_uninit(csi_spiflash_t *spiflash)
{
    CSI_PARAM_CHK_NORETVAL(spiflash);

#ifdef CONFIG_PM
    csi_spiflash_power_down(spiflash);
#endif

    csi_qspi_uninit(&spiflash->spi_qspi.qspi);
}

csi_error_t spiflash_config_data_line(csi_spiflash_t *spiflash, csi_spiflash_data_line_t line)
{
    return CSI_OK;
}
