/*
* Copyright (C) Cvitek Co., Ltd. 2019-2029. All rights reserved.
*/
/******************************************************************************
 * @file     dw_mmc.c
 * @brief    CSI Source File for mmc Driver
 * @version  V1.0
 * @date     29. July 2022
 ******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <csi_config.h>
#include <soc.h>
#include <assert.h>
#include <drv/cvi_irq.h>
#include <drv/list.h>
#include <drv/sdif.h>
#include <dw_mmc.h>
#include <dw_sdmmc.h>
#include <sys_clk.h>
#include "top_reg.h"

/* LOG_LEVEL: 0: Err; 1: Err&Warn; 2: Err&Warn&Info; 3: Err&Warn&Info&Debug */
#define LOG_LEVEL 0
#include <syslog.h>
#include <errno.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ERR_SDIF(errno) (CSI_DRV_ERRNO_SDIF_BASE | errno)

typedef struct  {
    uint32_t base;
    uint32_t irq;
} dw_sdif_priv_t;

/*
 * brief sdif handle
 *
 * Defines the structure to save the sdif state information and callback function. The detail interrupt status when
 * send command or transfer data can be obtained from interrupt_flags field by using mask defined in
 * sdif_interrupt_flag_t;
 * note All the fields except interrupt_flags and transfered_words must be allocated by the user.
 */
typedef struct {
    /* Transfer parameter */
    sdif_data_t *volatile       data;                   ///< Data to transfer
    sdif_command_t *volatile    command;                ///< Command to send

    /* Transfer status */
    volatile uint32_t           interrupt_flags;        ///< Interrupt flags of last transaction
    volatile uint32_t           dma_interrupt_flags;    ///< DMA interrupt flags of last transaction
    volatile uint32_t           transfered_words;       ///< Words transferred by polling way
    volatile uint32_t           flags;
    volatile uint32_t           error;
    /* Callback functions */
    sdif_callback_t    callback;               ///< Callback function
    void                       *user_data;              ///< Parameter for transfer complete callback
} sdif_state_t;

sdif_state_t g_sdifState[CONFIG_SDIO_NUM];

/*! @brief SDIF internal handle pointer array */
static sdif_state_t *s_sdifState[CONFIG_SDIO_NUM];

/*! @brief SDIF base pointer array */
static SDIF_TYPE *s_sdifBase[CONFIG_SDIO_NUM];
static sdif_type_t param[CONFIG_SDIO_NUM];
/*! @brief SDIF IRQ name array */
static int s_sdifIRQ[CONFIG_SDIO_NUM];

static void *s_sdifIRQEntry[CONFIG_SDIO_NUM];
static csi_dev_t s_sdif_dev[CONFIG_SDIO_NUM];

/* Typedef for interrupt handler. */
typedef void (*sdif_isr_t)(SDIF_TYPE *base, sdif_state_t *handle);

/* SDIF ISR for transactional APIs. */
static sdif_isr_t s_sdifIsr;

/*! @brief define the controller support sd/sdio card version 2.0 */
#define SDIF_SUPPORT_SD_VERSION (0x20)
/*! @brief define the controller support mmc card version 4.4 */
#define SDIF_SUPPORT_MMC_VERSION (0x44)
/*! @brief define the timeout counter */
#define SDIF_TIMEOUT_VALUE (6553500U)
/*! @brief this value can be any value */
#define SDIF_POLL_DEMAND_VALUE (0xFFU)
/*! @brief DMA descriptor buffer1 size */
#define SDIF_DMA_DESCRIPTOR_BUFFER1_SIZE(x) (x & 0x1FFFU)
/*! @brief DMA descriptor buffer2 size */
#define SDIF_DMA_DESCRIPTOR_BUFFER2_SIZE(x) ((x & 0x1FFFU) << 13U)
/*! @brief RX water mark value */
#define SDIF_RX_WATERMARK (15U)
/*! @brief TX water mark value */
#define SDIF_TX_WATERMARK (16U)
extern int32_t target_sdio_init(int32_t idx, uint32_t *base, uint32_t *irq, void **handler);

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*
* @brief driver IRQ handler
* @param SDIF base address
* @param sdif handle
*/
static void SDIF_TransferHandleIRQ(unsigned int irqn, void *priv);

/*
* @brief sdif wait for cmd complete
* This function will wait for complete and clear status
* @param SDIF base address
* @param SDIF data
*/
static void SDIF_WaitCmdComplete(uint32_t sdif);

//static void SDIF_IRQEnable(int id, void *handle);

/*
* @brief sdif host init irq
* This function will init irq status
* @param SDIF base address
* @param SDIF data
*/
static void SDIF_HostInitForIrq(uint32_t sdif);



/*******************************************************************************
 * Code
 ******************************************************************************/

void sdhci_register_dump(void *base)
{
	uintptr_t BASE = (uintptr_t)base;

	printf("============ SDHCI REGISTER DUMP ===========\n");

	printf("Sys addr:  0x%08x | Version:  0x%08x\n",
		   mmio_read_32(BASE + SDIF_DMA_ADDRESS),
		   mmio_read_16(BASE + SDIF_HOST_VERSION));
	printf("Blk size:  0x%08x | Blk cnt:  0x%08x\n",
		   mmio_read_16(BASE + SDIF_BLOCK_SIZE),
		   mmio_read_16(BASE + SDIF_BLOCK_COUNT));
	printf("Argument:  0x%08x | Trn mode: 0x%08x\n",
		   mmio_read_32(BASE + SDIF_ARGUMENT),
		   mmio_read_16(BASE + SDIF_TRANSFER_MODE));
	printf("Present:   0x%08x | Host ctl: 0x%08x\n",
		   mmio_read_32(BASE + SDIF_PRESENT_STATE),
		   mmio_read_8(BASE + SDIF_HOST_CONTROL));
	printf("Power:     0x%08x | Blk gap:  0x%08x\n",
		   mmio_read_8(BASE + SDIF_PWR_CONTROL),
		   mmio_read_8(BASE + SDIF_BLOCK_GAP_CONTROL));
	printf("Wake-up:   0x%08x | Clock:    0x%08x\n",
		   mmio_read_8(BASE + SDIF_WAKE_UP_CONTROL),
		   mmio_read_16(BASE + SDIF_CLK_CTRL));
	printf("Timeout:   0x%08x | Int stat: 0x%08x\n",
		   mmio_read_8(BASE + SDIF_TOUT_CTRL),
		   mmio_read_32(BASE + SDIF_INT_STATUS));
	printf("Int enab:  0x%08x | Sig enab: 0x%08x\n",
		   mmio_read_32(BASE + SDIF_INT_ENABLE),
		   mmio_read_32(BASE + SDIF_SIGNAL_ENABLE));
	printf("ACmd stat: 0x%08x | Slot int: 0x%08x\n",
		   mmio_read_16(BASE + SDIF_AUTO_CMD_STATUS),
		   mmio_read_16(BASE + SDIF_SLOT_INT_STATUS));
	printf("Caps:      0x%08x | Caps_1:   0x%08x\n",
		   mmio_read_32(BASE + SDIF_CAPABILITIES),
		   mmio_read_32(BASE + SDIF_CAPABILITIES_1));
	printf("Cmd:       0x%08x | Max curr: 0x%08x\n",
		   mmio_read_16(BASE + SDIF_COMMAND),
		   mmio_read_32(BASE + SDIF_MAX_CURRENT));
	printf("Resp[0]:   0x%08x | Resp[1]:  0x%08x\n",
		   mmio_read_32(BASE + SDIF_RESPONSE),
		   mmio_read_32(BASE + SDIF_RESPONSE + 4));
	printf("Resp[2]:   0x%08x | Resp[3]:  0x%08x\n",
		   mmio_read_32(BASE + SDIF_RESPONSE + 8),
		   mmio_read_32(BASE + SDIF_RESPONSE + 12));
	printf("Host ctl2: 0x%08x\n",
		   mmio_read_16(BASE + SDIF_HOST_CONTROL2));
	printf("ADMA Err:  0x%08x | ADMA Ptr: 0x%08x%08x\n",
		   mmio_read_32(BASE + SDIF_ADMA_ERROR),
		   mmio_read_32(BASE + SDIF_ADMA_ADDRESS_HI),
		   mmio_read_32(BASE + SDIF_ADMA_ADDRESS));
	printf("============================================\n");
}

void sdhci_pinmux_dump(void)
{
	printf("============ SDIO0 PINMUX DUMP ===========\n");

	printf("SDIO0_CD  :  0x%08x | SDIO0_PWR :  0x%08x\n",
		mmio_read_8(REG_SDIO0_CD_PIO_REG),
		mmio_read_8(REG_SDIO0_PWR_EN_PIO_REG));
	printf("SDIO0_CLK :  0x%08x | SDIO0_CMD :  0x%08x\n",
		mmio_read_8(REG_SDIO0_CLK_PIO_REG),
		mmio_read_8(REG_SDIO0_CMD_PIO_REG));
	printf("SDIO0_DAT0:  0x%08x | SDIO0_DAT1:  0x%08x\n",
		mmio_read_8(REG_SDIO0_DAT0_PIO_REG),
		mmio_read_8(REG_SDIO0_DAT1_PIO_REG));
	printf("SDIO0_DAT2:  0x%08x | SDIO0_DAT3:  0x%08x\n",
		mmio_read_8(REG_SDIO0_DAT2_PIO_REG),
		mmio_read_8(REG_SDIO0_DAT3_PIO_REG));
	mdelay(10);
	printf("============ SDIO1 PINMUX DUMP ===========\n");

	printf("SDIO1_CLK :  0x%08x | SDIO1_CMD :  0x%08x\n",
		mmio_read_8(REG_SDIO1_CLK_PIO_REG),
		mmio_read_8(REG_SDIO1_CMD_PIO_REG));
	printf("SDIO1_DAT0:  0x%08x | SDIO1_DAT1:  0x%08x\n",
		mmio_read_8(REG_SDIO1_DAT0_PIO_REG),
		mmio_read_8(REG_SDIO1_DAT1_PIO_REG));
	printf("SDIO1_DAT2:  0x%08x | SDIO1_DAT3:  0x%08x\n",
		mmio_read_8(REG_SDIO1_DAT2_PIO_REG),
		mmio_read_8(REG_SDIO1_DAT3_PIO_REG));
	printf("SDIO1_MUTX:  0x%08x\n", mmio_read_32(0x03000294));
	mdelay(10);
	printf("=========== SDIO PINMUX DUMP END ===========\n");
}

static void SDIF_FinishCommand(SDIF_TYPE *base, uint32_t sdif)
{
	int i;
	sdif_command_t *command = param[sdif].command;
	uintptr_t BASE = (uintptr_t)base;

	if (command) {
		if (command->response_type == SDIF_CARD_RESPONSE_R2) {
			/* CRC is stripped so we need to do some shifting. */
			for (i = 0; i < 4; i++) {
				command->response[3-i] = mmio_read_32(BASE + SDIF_RESPONSE_01 + (3-i)*4) << 8;
				if (i != 3)
					command->response[3-i] |= mmio_read_8(BASE + SDIF_RESPONSE_01 + (3-i)*4-1);
			}
		} else {
			command->response[0] = mmio_read_32(BASE + SDIF_RESPONSE_01);
		}

		if ((command->resopnse_error_flags != 0U) &&
			((command->response_type == SDIF_CARD_RESPONSE_R1) || (command->response_type == SDIF_CARD_RESPONSE_R1b) ||
			 (command->response_type == SDIF_CARD_RESPONSE_R6) || (command->response_type == SDIF_CARD_RESPONSE_R5))) {
			if (((command->resopnse_error_flags) & (command->response[0U])) != 0U) {
				param[sdif].cmd_error = kStatus_SDIF_ResponseError;
				param[sdif].data_error = kStatus_SDIF_ResponseError;
			}
		}
	}
}

static void SDIF_CmdIrq(SDIF_TYPE *base, uint32_t intmask, uint32_t sdif)
{
	if (!param[sdif].command) {
		//printf("Got command interrupt 0x%08x even through no command operation was in progress.\n", intmask);
		return;
	}

	if (intmask & (SDIF_INT_TIMEOUT | SDIF_INT_CRC |
		       SDIF_INT_END_BIT | SDIF_INT_INDEX)) {
		if (intmask & SDIF_INT_TIMEOUT)
			param[sdif].cmd_error = -ETIMEDOUT;
		else
			param[sdif].cmd_error = -47;
		param[sdif].intmask = intmask;
		param[sdif].command = NULL;
		return;
	}

	if (intmask & SDIF_INT_RESPONSE)
		SDIF_FinishCommand(base, sdif);
}

static void SDIF_DataIrq(SDIF_TYPE *base, uint32_t intmask, uint32_t sdif)
{
	uintptr_t BASE = (uintptr_t)base;
	uint32_t command;

	/* CMD19 generates _only_ Buffer Read Ready interrupt */
	if (intmask & SDIF_INT_DATA_AVAIL) {
		command = SDIF_GET_CMD(mmio_read_16(BASE + SDIF_COMMAND));
		if (command == MMC_CMD19 ||
		    command == MMC_CMD21) {
			//host->tuning_done = 1;
			return;
		}
	}

	if (intmask & SDIF_INT_DATA_TIMEOUT)
		param[sdif].data_error = -ETIMEDOUT;
	else if (intmask & SDIF_INT_DATA_END_BIT)
		param[sdif].data_error = -45;
	else if (intmask & SDIF_INT_DATA_CRC)
		param[sdif].data_error = -47;
	else if (intmask & SDIF_INT_ADMA_ERROR)
		param[sdif].data_error = -EIO;


	if (param[sdif].data_error) {
		param[sdif].command = NULL;
		param[sdif].data = NULL;
	} else {
		if (intmask & SDIF_INT_DATA_END) {
			param[sdif].command = NULL;
			param[sdif].data = NULL;
			return;
		}

		if (intmask & SDIF_INT_DMA_END) {
			uint64_t dma_addr;
			dma_addr = mmio_read_32(BASE + SDIF_ADMA_SA_LOW);
			mmio_write_32(BASE + SDIF_ADMA_SA_LOW, dma_addr);
			mmio_write_32(BASE + SDIF_ADMA_SA_HIGH, 0);
		}
	}

	return;
}

static void SDIF_TransferHandleSDIOInterrupt(SDIF_TYPE *base)
{
    uint32_t sdif = csi_sdif_get_idx(base);

    if (s_sdifState[sdif]->callback.sdif_interrupt != NULL) {
        s_sdifState[sdif]->callback.sdif_interrupt(sdif, s_sdifState[sdif]->user_data);
    }
}

static void SDIF_TransferHandleIRQ(unsigned int irqn, void *priv)
{
	SDIF_TYPE *base = priv;
	uint32_t sdif = csi_sdif_get_idx(base);
	uintptr_t BASE = (uintptr_t)base;
	int max_loop             = 16;
	uint32_t intmask;
	uint32_t mask;
	uint32_t unexpected;

	intmask = mmio_read_32(BASE + SDIF_INT_STATUS);

	if (!intmask || intmask == 0xffffffff) {
		printf("never be here!\n");
		return;
	}

	//printf("got intmask: 0x%08x.\n", intmask);

	do {
		mask = intmask & (SDIF_INT_CMD_MASK | SDIF_INT_DATA_MASK |
				SDIF_INT_BUS_POWER);

		mmio_write_32(BASE + SDIF_INT_STATUS, mask);

		if (intmask & SDIF_INT_CMD_MASK) {
			SDIF_CmdIrq(base, intmask & SDIF_INT_CMD_MASK, sdif);
#ifndef CONFIG_KERNEL_NONE
			aos_event_set(&param[sdif]._gcmdEvent, 0x01, AOS_EVENT_OR);
#else
			param[sdif].cmd_completion = 1;
#endif
		}

		if (intmask & SDIF_INT_DMA_END) {
			uint64_t dma_addr;
			dma_addr = mmio_read_32(BASE + SDIF_ADMA_SA_LOW);
			mmio_write_32(BASE + SDIF_ADMA_SA_LOW, dma_addr);
			mmio_write_32(BASE + SDIF_ADMA_SA_HIGH, 0);
			return;
		}

		if (intmask & SDIF_INT_DATA_MASK) {
			SDIF_DataIrq(base, intmask & SDIF_INT_DATA_MASK, sdif);
#ifndef CONFIG_KERNEL_NONE
			aos_event_set(&param[sdif]._gdataEvent, 0x01, AOS_EVENT_OR);
#else
			param[sdif].data_completion = 1;
#endif
		}

		if (intmask & SDIF_INT_CARD_INT) {
			SDIF_TransferHandleSDIOInterrupt(base);
		}

		intmask &= ~(SDIF_INT_CARD_INSERT | SDIF_INT_CARD_REMOVE |
				SDIF_INT_CMD_MASK | SDIF_INT_DATA_MASK |
				SDIF_INT_ERROR | SDIF_INT_BUS_POWER |
				SDIF_INT_RETUNE | SDIF_INT_CARD_INT);

		if (intmask) {
			unexpected = intmask;
			mmio_write_32(BASE + SDIF_INT_STATUS, intmask);
			printf("unexpected interrupt: 0x%08x.", unexpected);
		}

		intmask = mmio_read_32(BASE + SDIF_INT_STATUS);
	} while (intmask && --max_loop);

	return;
}

void dw_sdio_irqhandler(csi_dev_t *dev)
{
    int idx = dev->idx;
	//CSI_PARAM_CHK_NORETVAL(s_sdifState[idx]);
    s_sdifIsr(s_sdifBase[idx], s_sdifState[idx]);
}

extern uint32_t csi_tick_get_ms(void);
status_t SDIF_SendCommand(SDIF_TYPE *base, sdif_command_t *cmd, uint32_t timeout)
{
	uint32_t flags = 0x00;
	uintptr_t BASE = (uintptr_t)base;
	uint32_t sdif = csi_sdif_get_idx(base);
	int end_time = 0;
	int start_time = csi_tick_get_ms();

	while (1) {
		if (!(mmio_read_32(BASE + SDIF_PRESENT_STATE) & SDIF_CMD_INHIBIT))
			break;
		end_time = csi_tick_get_ms();
		if (end_time - start_time >= 2000)
			return -1;
	}

	if (cmd->index == MMC_CMD0)
		flags |= SDIF_CMD_RESP_NONE;
	else if (cmd->index == MMC_CMD1)
		flags |= SDIF_CMD_RESP_SHORT;
	else if (cmd->index == SD_ACMD41)
		flags |= SDIF_CMD_RESP_SHORT;
	else if (cmd->response_type == SDIF_CARD_RESPONSE_R2){
		flags |= SDIF_CMD_RESP_LONG;
		flags |= SDIF_CMD_CRC;
	} else {
		flags |= SDIF_CMD_RESP_SHORT;
		flags |= SDIF_CMD_CRC;
		flags |= SDIF_CMD_INDEX;
	}

	// make sure dat line is clear if necessary
	if (flags & SDIF_CMD_RESP_SHORT_BUSY) {
		start_time = csi_tick_get_ms();
		while (1) {
			if (!(mmio_read_32(BASE + SDIF_PRESENT_STATE) & SDIF_CMD_INHIBIT_DAT))
				break;
			end_time = csi_tick_get_ms();
			if (end_time - start_time >= 2000)
				return -1;
		}
	}

	if (cmd->index == MMC_CMD19 || cmd->index == MMC_CMD21)
		flags |= SDIF_CMD_DATA;

	SDIF_HostInitForIrq(sdif);

// issue the cmd
	mmio_write_32(BASE + SDIF_ARGUMENT, cmd->argument);
	mmio_write_16(BASE + SDIF_COMMAND, SDIF_MAKE_CMD(cmd->index, flags));
	if (cmd->index == MMC_CMD19 || cmd->index == MMC_CMD21)
		return 0;

	SDIF_WaitCmdComplete(sdif);

	if (cmd->resopnse_error_flags) {   //cmd->err
		printf("%s command transfer failed\n", __func__);
		return -1;
	}

	return timeout ? kStatus_Success : kStatus_Fail;
}

/*!
 * @brief enable/disable the card power.
 * once turn power on, software should wait for regulator/switch
 * ramp-up time before trying to initialize card.
 * @param base SDIF peripheral base address.
 * @param enable/disable flag.
 */
void csi_sdif_enable_card_power(sdif_handle_t handle, bool enable)
{
    return;
    SDIF_TYPE *base = (SDIF_TYPE *)handle;
    uintptr_t BASE = (uintptr_t)base;
    if (enable) {
        mmio_write_8(BASE + SDIF_PWR_CONTROL,mmio_read_8(BASE + SDIF_PWR_CONTROL) | 0x1);
    } else {
        mmio_write_8(BASE + SDIF_PWR_CONTROL,mmio_read_8(BASE+ SDIF_PWR_CONTROL) & ~0x1);
    }
}

void csi_sdif_enable_interrupt(sdif_handle_t handle, uint32_t mask)
{
    SDIF_TYPE *base = (SDIF_TYPE *)handle;
	uintptr_t BASE = (uintptr_t)base;

	mask = mmio_read_16(BASE + SDIF_SIGNAL_ENABLE);
	mask |= SDIF_INT_CARD_INT;

	mmio_write_16(BASE + SDIF_INT_ENABLE, mask);
	mmio_write_16(BASE + SDIF_SIGNAL_ENABLE, mask);
}

void csi_sdif_disable_interrupt(sdif_handle_t handle, uint32_t mask)
{
    SDIF_TYPE *base = (SDIF_TYPE *)handle;
	uintptr_t BASE = (uintptr_t)base;

	mask = mmio_read_16(BASE + SDIF_SIGNAL_ENABLE);
	mask &= ~SDIF_INT_CARD_INT;

	mmio_write_16(BASE + SDIF_INT_ENABLE, mask);
	mmio_write_16(BASE + SDIF_SIGNAL_ENABLE, mask);
}

void csi_sdif_enable_card_detect_interrupt(sdif_handle_t handle)
{
    //该case废弃使用 mask无区分会打开SDIF中断,不接入
    //csi_sdif_enable_interrupt(handle, kSDIF_CardDetect);
}

void csi_sdif_enable_sdio_interrupt(sdif_handle_t handle)
{
    csi_sdif_enable_interrupt(handle, kSDIF_SDIOInterrupt);
}

void csi_sdif_disable_sdio_interrupt(sdif_handle_t handle)
{
    csi_sdif_disable_interrupt(handle, kSDIF_SDIOInterrupt);
}

/*!
 * @brief SDIF module detect card insert status function.
 * @param base SDIF peripheral base address.
 * @param data3 indicate use data3 as card insert detect pin
 * @retval 1 card is inserted
 *         0 card is removed
 */
uint32_t csi_sdif_detect_card_insert(sdif_handle_t handle, bool data3)
{
    SDIF_TYPE *base = (SDIF_TYPE *)handle;
    uintptr_t BASE = (uintptr_t)base;
    if (data3) {
        return (mmio_read_32(BASE+SDIF_PRESENT_STATE) & SDIF_CARD_STABLE) == SDIF_CARD_STABLE ? 0U : 1U;
    } else {
        return (mmio_read_32(BASE+SDIF_PRESENT_STATE) & SDIF_CARD_INSERTED) == SDIF_CARD_INSERTED ? 1U : 0U;
    }
}

bool csi_sdif_send_card_active(sdif_handle_t handle, uint32_t timeout)
{
    SDIF_TYPE *base = (SDIF_TYPE *)handle;
    (void)base;
    return true;
}

/*!
 * @brief SDIF module enable/disable card clock.
 * @param base SDIF peripheral base address.
 * @param enable/disable flag
 */
void csi_sdif_enable_card_clock(sdif_handle_t handle, bool enable)
{
    SDIF_TYPE *base = (SDIF_TYPE *)handle;
    uintptr_t BASE = (uintptr_t)base;
    if (enable) {
         mmio_write_16(BASE + SDIF_CLK_CTRL,
				mmio_read_32(BASE + SDIF_CLK_CTRL) | (0x1<<2)); // stop SD clock
    } else {
        mmio_write_16(BASE + SDIF_CLK_CTRL,
				mmio_read_32(BASE + SDIF_CLK_CTRL) & ~(0x1<<2)); // stop SD clock
    }
}

uint32_t SDIF_SetCardClock(SDIF_TYPE *base, uint32_t srcClock_Hz, uint32_t target_HZ)
{
	uintptr_t BASE = (uintptr_t)base;
	uint32_t divider = 1U;
	uint32_t i;

	assert(target_HZ > 0);

	if (srcClock_Hz <= target_HZ ) {
		divider = 0;
	} else {
		for (divider = 0x1; divider < 0x3FF; divider++){
			if(srcClock_Hz / (2*divider) <= target_HZ)
				break;
		}
		if(divider == 0x3FF){
			//printf("Warning: Can't set the freq to %d, divider is filled!!!\n", target_HZ);
		}
	}

	assert(divider <= 0x3FF);

	if (mmio_read_16(BASE + SDIF_HOST_CONTROL2) & 1<<15) {
		//printf("Use SDCLK Preset Value.\n");
	} else {
		mmio_write_16(BASE + SDIF_CLK_CTRL,
				mmio_read_16(BASE + SDIF_CLK_CTRL) & ~0x9); // disable INTERNAL_CLK_EN and PLL_ENABLE
		mmio_write_16(BASE + SDIF_CLK_CTRL,
				(mmio_read_16(BASE + SDIF_CLK_CTRL) & 0x3F) | ((divider & 0xff) << 8) | ((divider & 0x300) >> 2)); // set clk div

		mmio_write_16(BASE + SDIF_CLK_CTRL,
				mmio_read_16(BASE + SDIF_CLK_CTRL) | 0x1); // set INTERNAL_CLK_EN

		for (i = 0; i <= 150000; i += 100) {
			if (mmio_read_16(BASE + SDIF_CLK_CTRL) & 0x2) {
				break;
			}
			udelay(100);
		}

		if (i > 150000) {
			//printf("SD INTERNAL_CLK_EN seting FAILED!\n");
			assert(0);
		}

		mmio_write_16(BASE + SDIF_CLK_CTRL,
				mmio_read_16(BASE + SDIF_CLK_CTRL) | 0x8); // set PLL_ENABLE

		for (i = 0; i <= 150000; i += 100) {
			if (mmio_read_16(BASE + SDIF_CLK_CTRL) & 0x2) {
				return target_HZ;
			}
			udelay(100);
		}
	}

	printf("SD PLL seting FAILED!\n");
	return -1;
}

uint32_t SDIF_ChangeCardClock(SDIF_TYPE *base, uint32_t srcClock_Hz, uint32_t target_HZ)
{
	uintptr_t BASE = (uintptr_t)base;
	uint32_t divider = 1U;
	uint32_t i;

	if (target_HZ <= 0) {
		mmio_write_16(BASE + SDIF_CLK_CTRL,
				mmio_read_32(BASE + SDIF_CLK_CTRL) & ~(0x1<<2)); // stop SD clock
//		mmio_write_16(BASE + 0x31e, 0x00);
		return -1;
	}

	if (srcClock_Hz <= target_HZ ) {
		divider = 0;
	} else {
		for (divider = 0x1; divider < 0x3FF; divider++){
			if(srcClock_Hz / (2*divider) <= target_HZ)
				break;
		}
		if(divider == 0x3FF){
			//printf("Warning: Can't set the freq to %d, divider is filled!!!\n", target_HZ);
		}
	}

	assert(divider <= 0x3FF);

	mmio_write_16(BASE + SDIF_CLK_CTRL,
			mmio_read_16(BASE + SDIF_CLK_CTRL) & ~(0x1<<2)); // stop SD clock

//	mmio_write_16(BASE + 0x31e, 0x10);
	mmio_write_16(BASE + SDIF_CLK_CTRL,
			mmio_read_16(BASE + SDIF_CLK_CTRL) & ~0x8); // disable  PLL_ENABLE

	if (mmio_read_16(BASE + SDIF_HOST_CONTROL2) & 1<<15) {
		//printf("Use SDCLK Preset Value.\n");
		// 4 need recheck?
		mmio_write_16(BASE + SDIF_HOST_CONTROL2,
				mmio_read_16(BASE + SDIF_HOST_CONTROL2) & ~0x7); // clr UHS_MODE_SEL
	} else {
		mmio_write_16(BASE + SDIF_CLK_CTRL,
				(mmio_read_16(BASE + SDIF_CLK_CTRL) & 0x3F) | ((divider & 0xff) << 8) | ((divider & 0x300) >> 2)); // set clk div
		mmio_write_16(BASE + SDIF_CLK_CTRL,
				mmio_read_16(BASE + SDIF_CLK_CTRL) & ~(0x1 << 5)); // CLK_GEN_SELECT
	}

	mmio_write_16(BASE + SDIF_CLK_CTRL,
			mmio_read_16(BASE + SDIF_CLK_CTRL) | 0xc); // enable  PLL_ENABLE

	//printf("mmio_read_16(BASE + SDIF_CLK_CTRL) = 0x%x\n", mmio_read_16(BASE + SDIF_CLK_CTRL));

	for (i = 0; i <= 150000; i += 100) {
		if (mmio_read_16(BASE + SDIF_CLK_CTRL) & 0x2) {
			return target_HZ;
		}
		udelay(100);
	}

	printf("SD PLL seting FAILED!\n");

	return -1;
}

void SDIF_Init(SDIF_TYPE *base, uint32_t sdif)
{
	uintptr_t BASE = (uintptr_t)base;

	mmio_write_8(BASE + SDIF_SOFTWARE_RESET, 0x6);

	mmio_write_8(BASE + SDIF_PWR_CONTROL, (0x7 << 1));
	mmio_write_8(BASE + SDIF_TOUT_CTRL, 0xe);
	mmio_write_16(BASE + SDIF_HOST_CONTROL2, mmio_read_16(BASE + SDIF_HOST_CONTROL2) | 1<<11);
	mmio_write_16(BASE + SDIF_CLK_CTRL, mmio_read_16(BASE + SDIF_CLK_CTRL) & ~(0x1 << 5));
	mmio_write_16(BASE + SDIF_HOST_CONTROL2, mmio_read_16(BASE + SDIF_HOST_CONTROL2) | SDIF_HOST_VER4_ENABLE);

	mmio_write_16(BASE + SDIF_HOST_CONTROL2, mmio_read_16(BASE + SDIF_HOST_CONTROL2) | 0x1<<13);

	if (mmio_read_32(BASE + SDIF_CAPABILITIES1) & (0x1<<29)) {
		mmio_write_16(BASE + SDIF_HOST_CONTROL2, mmio_read_16(BASE + SDIF_HOST_CONTROL2) | (0x1<<14)); // enable async int
	}

	mdelay(20);

	mmio_write_16(BASE + SDIF_HOST_CONTROL2, mmio_read_16(BASE + SDIF_HOST_CONTROL2) & ~(0x1<<8)); // clr UHS2_IF_ENABLE
	mmio_write_8(BASE + SDIF_PWR_CONTROL, mmio_read_8(BASE + SDIF_PWR_CONTROL) | 0x1); // set SD_BUS_PWR_VDD1
	mmio_write_16(BASE + SDIF_HOST_CONTROL2, mmio_read_16(BASE + SDIF_HOST_CONTROL2) & ~0x7); // clr UHS_MODE_SEL

	mdelay(50);

	mmio_write_16(BASE + SDIF_CLK_CTRL, mmio_read_16(BASE + SDIF_CLK_CTRL) | (0x1<<2)); // supply SD clock

	udelay(400); // wait for voltage ramp up time at least 74 cycle, 400us is 80 cycles for 200Khz

	mmio_write_16(BASE + SDIF_INT_STATUS, mmio_read_16(BASE + SDIF_INT_STATUS) | (0x1 << 6));

	mmio_write_16(BASE + SDIF_INT_STATUS_EN, mmio_read_16(BASE + SDIF_INT_STATUS_EN) | 0xFFFF);
	mmio_write_16(BASE + SDIF_ERR_INT_STATUS_EN, mmio_read_16(BASE + SDIF_ERR_INT_STATUS_EN) | 0xFFFF);

	if (param[sdif].ier) {
		if (BASE == SDIO0_BASE)
			request_irq(36, SDIF_TransferHandleIRQ, 0, "sdio0_irq", base);
		else if(BASE == SDIO1_BASE)
			request_irq(38, SDIF_TransferHandleIRQ, 0, "sdio1_irq", base);
		else if(BASE == SDIO2_BASE)
			request_irq(34, SDIF_TransferHandleIRQ, 0, "sdio2_irq", base);

		mmio_write_32(BASE + SDIF_SIGNAL_ENABLE, param[sdif].ier);
	} else {
		mmio_write_32(BASE + SDIF_SIGNAL_ENABLE, 0);
	}
}

status_t SDIF_TransferBlocking(SDIF_TYPE *base, sdif_dma_config_t *dmaConfig, sdif_transfer_t *transfer)
{
    //reserved
    return kStatus_Success;
 }


uint32_t SDIF_PrepareData(SDIF_TYPE *base,sdif_dma_config_t *dmaConfig, sdif_transfer_t *transfer)
{
	sdif_data_t *data = transfer->data;
	uintptr_t BASE = (uintptr_t)base;

	uint64_t load_addr;
	uint32_t block_cnt, blksz;
	uint8_t tmp;

	blksz = data->block_size;
	block_cnt = data->block_count;

	load_addr = (uint64_t)dmaConfig->dma_des_buffer_start_addr;

	soc_dcache_clean_invalid_range(load_addr, dmaConfig->dma_des_buffer_len);

	mmio_write_32(BASE + SDIF_ADMA_SA_LOW, load_addr);
	mmio_write_32(BASE + SDIF_ADMA_SA_HIGH, (load_addr >> 32));

	mmio_write_32(BASE + SDIF_DMA_ADDRESS, block_cnt);
	mmio_write_16(BASE + SDIF_BLOCK_COUNT, 0);

	mmio_write_16(BASE + SDIF_BLOCK_SIZE, SDIF_MAKE_BLKSZ(7, blksz));

	// select SDMA
	tmp = mmio_read_8(BASE + SDIF_HOST_CONTROL);
	tmp &= ~SDIF_CTRL_DMA_MASK;
	tmp |= SDIF_CTRL_SDMA;

	mmio_write_8(BASE + SDIF_HOST_CONTROL, tmp);

	return 0;
}

#ifndef CONFIG_KERNEL_NONE

static void SDIF_WaitCmdComplete(uint32_t sdif)
{
	unsigned int actl_flags = 0;
	if (aos_event_get(&param[sdif]._gcmdEvent , 0x01, AOS_EVENT_OR_CLEAR,
			&actl_flags, 2000) != 0) {
		//param[sdif].cmd_error = -110;
    }
}

static void SDIF_WaitDataComplete(uint32_t sdif)
{
	unsigned int actl_flags = 0;
	if (aos_event_get(&param[sdif]._gdataEvent , 0x01, AOS_EVENT_OR_CLEAR,
			&actl_flags, 2000) != 0) {
		//param[sdif].data_error = -110;
    }
}

#else

static void SDIF_WaitCmdComplete(uint32_t sdif)
{
    while (!param[sdif].cmd_completion) {
		//printf("wait_cmd_complete.\n");
		;
	}
	param[sdif].cmd_completion = 0;
}

static void SDIF_WaitDataComplete(uint32_t sdif)
{
	while (!param[sdif].data_completion) {
		//printf("wait_data_complete.\n");
		;
	}
	param[sdif].data_completion = 0;
}
#endif

static void SDIF_HostInitForIrq(uint32_t sdif)
{
	param[sdif].cmd_completion = 0;
	param[sdif].data_completion = 0;
	param[sdif].cmd_error = 0;

	if (param[sdif].data)
		param[sdif].data_error = 0;
}

void SDIF_HwReset(SDIF_TYPE *base)
{
	uintptr_t BASE = (uintptr_t)base;

	mmio_write_16(BASE + SDIF_CLK_CTRL,
		(mmio_read_16(BASE + SDIF_CLK_CTRL) & 0x3F) | DEFAULT_DIV_SD_INIT_CLOCK << 8);

	mdelay(1);

	mmio_write_8(BASE + SDIF_SOFTWARE_RESET, 0x7);
	while (mmio_read_8(BASE + SDIF_SOFTWARE_RESET))
		;
}

#define REG_TOP_SD_PWRSW_CTRL		(0x1F4)

void SDIO_PadSetting(SDIF_TYPE *base)
{
	uintptr_t BASE = (uintptr_t)base;

	if (BASE == SDIO0_BASE) {
		//set power for sd0
		mmio_write_32(TOP_BASE + REG_TOP_SD_PWRSW_CTRL, 0x9);
		mdelay(1);
		//set pu/down
		mmio_write_32(REG_SDIO0_CD_PAD_REG,
			(mmio_read_32(REG_SDIO0_CD_PAD_REG) & REG_SDIO0_PAD_MASK) |
			REG_SDIO0_CD_PAD_VALUE << REG_SDIO0_PAD_SHIFT);
		mmio_write_32(REG_SDIO0_PWR_EN_PAD_REG,
			(mmio_read_32(REG_SDIO0_PWR_EN_PAD_REG) & REG_SDIO0_PAD_MASK) |
			REG_SDIO0_PWR_EN_PAD_VALUE << REG_SDIO0_PAD_SHIFT);
		mmio_write_32(REG_SDIO0_CLK_PAD_REG,
			(mmio_read_32(REG_SDIO0_CLK_PAD_REG) & REG_SDIO0_PAD_MASK) |
			REG_SDIO0_CLK_PAD_VALUE << REG_SDIO0_PAD_SHIFT);
		mmio_write_32(REG_SDIO0_CMD_PAD_REG,
			(mmio_read_32(REG_SDIO0_CMD_PAD_REG) & REG_SDIO0_PAD_MASK) |
			REG_SDIO0_CMD_PAD_VALUE << REG_SDIO0_PAD_SHIFT);
		mmio_write_32(REG_SDIO0_DAT1_PAD_REG,
			(mmio_read_32(REG_SDIO0_DAT1_PAD_REG) & REG_SDIO0_PAD_MASK) |
			REG_SDIO0_DAT1_PAD_VALUE << REG_SDIO0_PAD_SHIFT);
		mmio_write_32(REG_SDIO0_DAT0_PAD_REG,
			(mmio_read_32(REG_SDIO0_DAT0_PAD_REG) & REG_SDIO0_PAD_MASK) |
			REG_SDIO0_DAT0_PAD_VALUE << REG_SDIO0_PAD_SHIFT);
		mmio_write_32(REG_SDIO0_DAT2_PAD_REG,
			(mmio_read_32(REG_SDIO0_DAT2_PAD_REG) & REG_SDIO0_PAD_MASK) |
			REG_SDIO0_DAT2_PAD_VALUE << REG_SDIO0_PAD_SHIFT);
		mmio_write_32(REG_SDIO0_DAT3_PAD_REG,
			(mmio_read_32(REG_SDIO0_DAT3_PAD_REG) & REG_SDIO0_PAD_MASK) |
			REG_SDIO0_DAT3_PAD_VALUE << REG_SDIO0_PAD_SHIFT);
		//set pinmux
		mmio_write_8(REG_SDIO0_CD_PIO_REG, REG_SDIO0_CD_PIO_VALUE);
		mmio_write_8(REG_SDIO0_PWR_EN_PIO_REG, REG_SDIO0_PWR_EN_PIO_VALUE);
		mmio_write_8(REG_SDIO0_CLK_PIO_REG, REG_SDIO0_CLK_PIO_VALUE);
		mmio_write_8(REG_SDIO0_CMD_PIO_REG, REG_SDIO0_CMD_PIO_VALUE);
		mmio_write_8(REG_SDIO0_DAT0_PIO_REG, REG_SDIO0_DAT0_PIO_VALUE);
		mmio_write_8(REG_SDIO0_DAT1_PIO_REG, REG_SDIO0_DAT1_PIO_VALUE);
		mmio_write_8(REG_SDIO0_DAT2_PIO_REG, REG_SDIO0_DAT2_PIO_VALUE);
		mmio_write_8(REG_SDIO0_DAT3_PIO_REG, REG_SDIO0_DAT3_PIO_VALUE);
	}
	else if(BASE == SDIO1_BASE) {
		// set rtc sdio1 related register
		mmio_write_32(RTCSYS_CTRL, 0x1);
		mmio_write_32(RTCSYS_CLKMUX, 0x10);
		mmio_write_32(RTCSYS_CLKBYP, 0xfffffffc);
		//mmio_write_32(RTCSYS_MCU51_ICTRL1, 0x0);

		mmio_write_32(REG_SDIO1_CLK_PAD_REG,
			(mmio_read_32(REG_SDIO1_CLK_PAD_REG) & REG_SDIO1_PAD_MASK) |
			REG_SDIO1_CLK_PAD_VALUE << REG_SDIO1_PAD_SHIFT);
		mmio_write_32(REG_SDIO1_CMD_PAD_REG,
			(mmio_read_32(REG_SDIO1_CMD_PAD_REG) & REG_SDIO1_PAD_MASK) |
			REG_SDIO1_CMD_PAD_VALUE << REG_SDIO1_PAD_SHIFT);
		mmio_write_32(REG_SDIO1_DAT1_PAD_REG,
			(mmio_read_32(REG_SDIO1_DAT1_PAD_REG) & REG_SDIO1_PAD_MASK) |
			REG_SDIO1_DAT1_PAD_VALUE << REG_SDIO1_PAD_SHIFT);
		mmio_write_32(REG_SDIO1_DAT0_PAD_REG,
			(mmio_read_32(REG_SDIO1_DAT0_PAD_REG) & REG_SDIO1_PAD_MASK) |
			REG_SDIO1_DAT0_PAD_VALUE << REG_SDIO1_PAD_SHIFT);
		mmio_write_32(REG_SDIO1_DAT2_PAD_REG,
			(mmio_read_32(REG_SDIO1_DAT2_PAD_REG) & REG_SDIO1_PAD_MASK) |
			REG_SDIO1_DAT2_PAD_VALUE << REG_SDIO1_PAD_SHIFT);
		mmio_write_32(REG_SDIO1_DAT3_PAD_REG,
			(mmio_read_32(REG_SDIO1_DAT3_PAD_REG) & REG_SDIO1_PAD_MASK) |
			REG_SDIO1_DAT3_PAD_VALUE << REG_SDIO1_PAD_SHIFT);
		mmio_write_32(RTCSYS_CTRL, 0x1); // enable rtc2ap_ahb;
		//set pinmux
		mmio_write_32(TOP_BASE + 0x294, (mmio_read_32(TOP_BASE + 0x294) & 0xFFFFFBFF));
		mmio_write_8(REG_SDIO1_CLK_PIO_REG, REG_SDIO1_CLK_PIO_VALUE);
		mmio_write_8(REG_SDIO1_CMD_PIO_REG, REG_SDIO1_CMD_PIO_VALUE);
		mmio_write_8(REG_SDIO1_DAT0_PIO_REG, REG_SDIO1_DAT0_PIO_VALUE);
		mmio_write_8(REG_SDIO1_DAT1_PIO_REG, REG_SDIO1_DAT1_PIO_VALUE);
		mmio_write_8(REG_SDIO1_DAT2_PIO_REG, REG_SDIO1_DAT2_PIO_VALUE);
		mmio_write_8(REG_SDIO1_DAT3_PIO_REG, REG_SDIO1_DAT3_PIO_VALUE);
	}
	else if(BASE == SDIO2_BASE) {
		//set pu/down
		mmio_write_32(REG_SDIO2_RSTN_PAD_REG,
			(mmio_read_32(REG_SDIO2_RSTN_PAD_REG) & REG_SDIO2_PAD_MASK) |
			REG_SDIO2_RSTN_PAD_VALUE << REG_SDIO2_PAD_SHIFT);
		mmio_write_32(REG_SDIO2_CLK_PAD_REG,
			(mmio_read_32(REG_SDIO2_CLK_PAD_REG) & REG_SDIO2_PAD_MASK) |
			REG_SDIO2_CLK_PAD_VALUE << REG_SDIO2_PAD_SHIFT);
		mmio_write_32(REG_SDIO2_CMD_PAD_REG,
			(mmio_read_32(REG_SDIO2_CMD_PAD_REG) & REG_SDIO2_PAD_MASK) |
			REG_SDIO2_CMD_PAD_VALUE << REG_SDIO2_PAD_SHIFT);
		mmio_write_32(REG_SDIO2_DAT0_PAD_REG,
			(mmio_read_32(REG_SDIO2_DAT0_PAD_REG) & REG_SDIO2_PAD_MASK) |
			REG_SDIO2_DAT0_PAD_VALUE << REG_SDIO2_PAD_SHIFT);
		mmio_write_32(REG_SDIO2_DAT1_PAD_REG,
			(mmio_read_32(REG_SDIO2_DAT1_PAD_REG) & REG_SDIO2_PAD_MASK) |
			REG_SDIO2_DAT1_PAD_VALUE << REG_SDIO2_PAD_SHIFT);
		mmio_write_32(REG_SDIO2_DAT2_PAD_REG,
			(mmio_read_32(REG_SDIO2_DAT2_PAD_REG) & REG_SDIO2_PAD_MASK) |
			REG_SDIO2_DAT2_PAD_VALUE << REG_SDIO2_PAD_SHIFT);
		mmio_write_32(REG_SDIO2_DAT3_PAD_REG,
			(mmio_read_32(REG_SDIO2_DAT3_PAD_REG) & REG_SDIO2_PAD_MASK) |
			REG_SDIO2_DAT3_PAD_VALUE << REG_SDIO2_PAD_SHIFT);
		//set pinmux
		mmio_write_8(REG_SDIO2_RSTN_PIO_REG, REG_SDIO2_RSTN_PIO_VALUE);
		mmio_write_8(REG_SDIO2_CLK_PIO_REG, REG_SDIO2_CLK_PIO_VALUE);
		mmio_write_8(REG_SDIO2_CMD_PIO_REG, REG_SDIO2_CMD_PIO_VALUE);
		mmio_write_8(REG_SDIO2_DAT0_PIO_REG, REG_SDIO2_DAT0_PIO_VALUE);
		mmio_write_8(REG_SDIO2_DAT1_PIO_REG, REG_SDIO2_DAT1_PIO_VALUE);
		mmio_write_8(REG_SDIO2_DAT2_PIO_REG, REG_SDIO2_DAT2_PIO_VALUE);
		mmio_write_8(REG_SDIO2_DAT3_PIO_REG, REG_SDIO2_DAT3_PIO_VALUE);
	}
}

void SDIF_PhyInit(SDIF_TYPE *base, uint32_t sdif)
{
	uintptr_t BASE = (uintptr_t)base;

	uintptr_t vendor_base = BASE + (mmio_read_16(BASE + P_VENDOR_SPECIFIC_AREA) & ((1<<12)-1));

	SDIF_HwReset(base);

	mdelay(3);

	SDIO_PadSetting(base);

	if (BASE == SDIO2_BASE) {
		//reg_0x200[0] = 1 for sd2
		mmio_write_32(vendor_base,
				mmio_read_32(vendor_base) | BIT(0));
	}

	//reg_0x200[1] = 1
	mmio_write_32(vendor_base,
			mmio_read_32(vendor_base) | BIT(1));

	if (BASE == SDIO1_BASE) {
		//reg_0x200[16] = 1 for sd1
		mmio_write_32(vendor_base,
				mmio_read_32(vendor_base) | BIT(16));
	}

	mmio_write_32(vendor_base + SDIF_PHY_CONFIG,
			mmio_read_32(vendor_base + SDIF_PHY_CONFIG) | BIT(0));

	mmio_write_32(vendor_base + SDIF_PHY_TX_RX_DLY, 0x1000100);

}

#ifndef CONFIG_KERNEL_NONE
static inline void *align_alloc(uint64_t align, uint32_t size, void **mem_unalign)
{
	void *mem;
	uint64_t offset;
	size+=2*align;

	*mem_unalign = (void *)malloc(size);

	if (!*mem_unalign) {
		printf("sdio memalign error!\n");
		return NULL;
	}

	offset = (uint64_t)*mem_unalign % align;

	if (offset == 0) {
		mem = (void *)*mem_unalign;
	} else {
		mem = (void *)(*mem_unalign + (align - offset));
	}
	return mem;
}
#endif

status_t SDIF_SendDataCmd(SDIF_TYPE *base,
                                  sdif_state_t *handle,
                                  sdif_dma_config_t *dmaConfig,
                                  sdif_transfer_t *transfer,
                                  uint32_t sdif)
{
	assert(NULL != transfer);

	sdif_data_t *data   = transfer->data;
	sdif_command_t *cmd = transfer->command;
	uintptr_t BASE = (uintptr_t)base;
	uint32_t mode       = 0;
	uint32_t flags      = 0;
	dmaConfig->dma_des_buffer_len = data->block_size * data->block_count;

	while(1){
		if(!(mmio_read_32(BASE + SDIF_PRESENT_STATE) & SDIF_CMD_INHIBIT))
			break;
	}

#ifndef CONFIG_KERNEL_NONE
	void *src_align = NULL;
	void *src_unalign = NULL;
	if (data->rx_date_buffer != NULL) {
		if ((uint64_t)data->rx_date_buffer & (SDMMC_DMA_ALIGN_CACHE - 1)) {
			src_align = align_alloc(SDMMC_DMA_ALIGN_CACHE,
							dmaConfig->dma_des_buffer_len, (void **)&src_unalign);
			dmaConfig->dma_des_buffer_start_addr = (uint32_t *)src_align;
		} else
			dmaConfig->dma_des_buffer_start_addr = (uint32_t *)data->rx_date_buffer;
	} else
		dmaConfig->dma_des_buffer_start_addr = (uint32_t *)data->tx_data_buffer;
#else
	if (data->rx_date_buffer != NULL)
		dmaConfig->dma_des_buffer_start_addr = (uint32_t *)data->rx_date_buffer;
	else
		dmaConfig->dma_des_buffer_start_addr = (uint32_t *)data->tx_data_buffer;
#endif

	SDIF_PrepareData(base, dmaConfig, transfer);

	mode = SDIF_TRNS_DMA;
	if (mmc_op_multi(cmd->index) || data->block_count > 1) {
		mode |= SDIF_TRNS_MULTI | SDIF_TRNS_BLK_CNT_EN;
	}

	if (data->rx_date_buffer != NULL) {
		mode |= SDIF_TRNS_READ;
	} else {
		mode &= ~SDIF_TRNS_READ;
	}

	// set cmd flags
	if (cmd->index == 0)
		flags |= SDIF_CMD_RESP_NONE;
	else if (cmd->response_type == SDIF_CARD_RESPONSE_R2)
		flags |= SDIF_CMD_RESP_LONG;
	else
		flags |= SDIF_CMD_RESP_SHORT;
	flags |= SDIF_CMD_CRC;
	flags |= SDIF_CMD_INDEX;
	flags |= SDIF_CMD_DATA;

	SDIF_HostInitForIrq(sdif);

	mmio_write_16(BASE + SDIF_TRANSFER_MODE, mode);
	mmio_write_32(BASE + SDIF_ARGUMENT, cmd->argument);
	// issue the cmd
	mmio_write_16(BASE + SDIF_COMMAND, SDIF_MAKE_CMD(cmd->index, flags));

	SDIF_WaitCmdComplete(sdif);
	SDIF_WaitDataComplete(sdif);

	if (data->rx_date_buffer != NULL) {
		soc_dcache_invalid_range((uint64_t)dmaConfig->dma_des_buffer_start_addr,
									dmaConfig->dma_des_buffer_len);
#ifndef CONFIG_KERNEL_NONE
		if (src_unalign) {
			memcpy((void *)data->rx_date_buffer, src_align, dmaConfig->dma_des_buffer_len);
			free(src_unalign);
			src_align = NULL;
			src_unalign = NULL;
		}
#endif
	}

	if (data->block_count > 1 && mmc_op_multi(cmd->index)) {
		data->enable_auto_command12 = false;
	}

	if (param[sdif].cmd_error || param[sdif].data_error) {
		printf("SDIF_SendDataCmd failed.%d.%d.\n", param[sdif].cmd_error, param[sdif].data_error);
		return kStatus_Fail;
	}

	return kStatus_Success;
}

static int SDIF_SendNoDataCmd(SDIF_TYPE *base,
                                  sdif_state_t *handle,
                                  sdif_dma_config_t *dmaConfig,
                                  sdif_transfer_t *transfer,
                                  uint32_t sdif)
{
	uintptr_t BASE = (uintptr_t)base;
	sdif_command_t *cmd = transfer->command;
	uint32_t flags      = 0;
	int end_time = 0;
	int start_time = csi_tick_get_ms();

	while (1) {
		if (!(mmio_read_32(BASE + SDIF_PRESENT_STATE) & SDIF_CMD_INHIBIT))
			break;
		end_time = csi_tick_get_ms();
		if (end_time - start_time >= 2000)
			return -1;
	}

	if (cmd->index == MMC_CMD0)
		flags |= SDIF_CMD_RESP_NONE;
	else if (cmd->index == MMC_CMD1)
		flags |= SDIF_CMD_RESP_SHORT;
	else if (cmd->index == SD_ACMD41)
		flags |= SDIF_CMD_RESP_SHORT;
	else if (cmd->response_type == SDIF_CARD_RESPONSE_R2) {
		flags |= SDIF_CMD_RESP_LONG;
		flags |= SDIF_CMD_CRC;
	} else {
		flags |= SDIF_CMD_RESP_SHORT;
		//flags |= SDHCI_CMD_CRC;
		//flags |= SDHCI_CMD_INDEX;
	}

	if (flags & SDIF_CMD_RESP_SHORT_BUSY) {
		start_time = csi_tick_get_ms();
		while (1) {
			if (!(mmio_read_32(BASE + SDIF_PRESENT_STATE) & SDIF_CMD_INHIBIT_DAT))
				break;
			end_time = csi_tick_get_ms();
			if (end_time - start_time >= 2000)
				return -1;
		}
	}

	SDIF_HostInitForIrq(sdif);

	mmio_write_32(BASE + SDIF_ARGUMENT, cmd->argument);
	mmio_write_16(BASE + SDIF_COMMAND, SDIF_MAKE_CMD(cmd->index, flags));

	SDIF_WaitCmdComplete(sdif);

	if (param[sdif].cmd_error) {
		//printf("command transfer failed.%d.%d. 0x%08x.\n\n", param[sdif].cmd_error, cmd->index, param[sdif].intmask);
		return kStatus_Fail;
	}

	return kStatus_Success;
}

status_t SDIF_TransferNonBlocking(SDIF_TYPE *base,
                                  sdif_state_t *handle,
                                  sdif_dma_config_t *dmaConfig,
                                  sdif_transfer_t *transfer)
{
	assert(NULL != transfer);
	status_t ret;
	uint32_t sdif = csi_sdif_get_idx(base);

	if (transfer->data) {
		param[sdif].command = transfer->command;
		param[sdif].data = transfer->data;
		//printf("----->have data:%d.0x%x\n", transfer->command->index, transfer->command->argument);
		ret = SDIF_SendDataCmd(base, handle, dmaConfig, transfer, sdif);
	} else {
		param[sdif].command = transfer->command;
		param[sdif].data = NULL;
		//printf("----->no   data:%d.0x%x\n", transfer->command->index, transfer->command->argument);
		ret = SDIF_SendNoDataCmd(base, handle, dmaConfig, transfer, sdif);
	}
	s_sdifState[sdif]->callback.transfer_complete(sdif, NULL, ret, NULL);

	return ret;
}

static void SDIF_TransferCreateHandle(SDIF_TYPE *base,
                                      sdif_state_t *handle,
                                      sdif_callback_t *callback,
                                      void *user_data)
{
    assert(handle);
    assert(callback);

    /* reset the handle. */
    memset(handle, 0U, sizeof(*handle));

    /* Set the callback. */
    handle->callback.sdif_interrupt = callback->sdif_interrupt;
    handle->callback.dma_des_unavailable = callback->dma_des_unavailable;
    handle->callback.command_reload = callback->command_reload;
    handle->callback.transfer_complete = callback->transfer_complete;
    handle->callback.card_inserted = callback->card_inserted;
    handle->callback.card_removed = callback->card_removed;
    handle->user_data = user_data;

    /* Save the handle in global variables to support the double weak mechanism. */
    s_sdifState[csi_sdif_get_idx(base)] = handle;

    /* save IRQ handler */
    //s_sdifIsr = SDIF_TransferHandleIRQ;

#ifndef DW_SDIO_NOSUPPORT_DMA
    /* enable the global interrupt */
    //SDIF_EnableGlobalInterrupt(base, true);
#endif

    //SDIF_IRQEnable(s_sdifIRQ[csi_sdif_get_idx(base)], s_sdifIRQEntry[csi_sdif_get_idx(base)]);
}

void SDIF_GetCapability(SDIF_TYPE *base, sdif_capability_t *capability)
{
    assert(NULL != capability);

    capability->sd_version = SDIF_SUPPORT_SD_VERSION;
    capability->mmc_version = SDIF_SUPPORT_MMC_VERSION;
    capability->max_block_length = SDIF_BLKSIZ_BLOCK_SIZE_MASK;
    /* set the max block count = max byte count / max block size */
    capability->max_block_count = SDIF_BYTCNT_BYTE_COUNT_MASK / SDIF_BLKSIZ_BLOCK_SIZE_MASK;
    capability->flags = SDIF_SUPPORT_HIGH_SPEED | SDIF_SUPPORT_DMA_SPEED | SDIF_SUPPORT_USPEND_RESUME |
                        SDIF_SUPPORT_V330 | SDIF_SUPPORT_4BIT | SDIF_SUPPORT_8BIT;
}

void SDIF_IRQDisable(int id, void *handle);
void SDIF_Deinit(SDIF_TYPE *base)
{
    SDIF_IRQDisable(s_sdifIRQ[csi_sdif_get_idx(base)], s_sdifIRQEntry[csi_sdif_get_idx(base)]);
}
/*
void SDIF_IRQEnable(int id, void *handle)
{
    csi_irq_attach((uint32_t)id, handle, &s_sdif_dev[id]);
    csi_irq_enable((uint32_t)id);
}
*/
void SDIF_IRQDisable(int id, void *handle)
{
    csi_irq_disable(id);
    csi_irq_detach(id);
}

void SDIF_Binding(SDIF_TYPE **base, uint32_t sdif)
{
    *base = s_sdifBase[sdif];
}

/**
  \brief  Get sdif handle
  \param[in]   idx sdif index
  \return      sdif handle
*/
sdif_handle_t csi_sdif_get_handle(uint32_t idx)
{
    csi_dev_t *dev = &s_sdif_dev[idx];

    if (0 == target_get(DEV_DW_SDMMC_TAG, idx, dev)) {

        s_sdifBase[idx] = (SDIF_TYPE *)dev->reg_base;

        //SDIF_TransferCreateHandle(s_sdifBase[idx], &g_sdifState[idx], NULL, NULL);
    }

    return (sdif_handle_t)s_sdifBase[idx];
}

/**
  \brief  Get sdif index
  \param[out]   handle  SDIF handle to operate.
  \return       sdif index
*/
uint32_t csi_sdif_get_idx(sdif_handle_t handle)
{
    SDIF_TYPE *base = (SDIF_TYPE *)handle;
    uint8_t instance = 0U;

    while ((instance < ARRAY_SIZE(s_sdifBase)) && (s_sdifBase[instance] != base)) {
        instance++;
    }

    assert(instance < ARRAY_SIZE(s_sdifBase));

    return instance;
}

/**
  \brief  Initializes the SDIF according to the specified
  \param[in]   idx sdif index
  \param[in]   callback callback Structure pointer to contain all callback functions.
  \param[in]   user_data user_data Callback function parameter.
  \return sdif handle if success
*/
sdif_handle_t csi_sdif_initialize(uint32_t idx, sdif_callback_t *callback, void *user_data)
{
	csi_dev_t *dev = &s_sdif_dev[idx];

	if (0 == target_get(DEV_DW_SDMMC_TAG, idx, dev)) {

		s_sdifBase[idx]     = (SDIF_TYPE *)dev->reg_base;
		s_sdifIRQ[idx]      = dev->irq_num;
		// dev->irq_handler    = dw_sdio_irqhandler;
		s_sdifIRQEntry[idx] = dw_sdio_irqhandler;

		SDIF_TransferCreateHandle(s_sdifBase[idx], &g_sdifState[idx], callback, user_data);
		// csi_irq_attach((uint32_t)(dev->irq_num), &dw_sdio_irqhandler, dev);
		// csi_irq_enable((uint32_t)(dev->irq_num));
	}

	return (sdif_handle_t)s_sdifBase[idx];
}

/**
  \brief       De-initialize SD Interface. stops operation and releases the software resources used by the interface
  \param[in]   handle  sdio handle to operate.
  \return      error code
*/
void csi_sdif_uninitialize(uint32_t idx, sdif_handle_t handle)
{
    SDIF_TYPE *base = (SDIF_TYPE *)handle;
    SDIF_Deinit(base);
    csi_dev_t *dev = &s_sdif_dev[idx];
    csi_irq_detach((uint32_t)(dev->irq_num));
    csi_irq_disable((uint32_t)(dev->irq_num));
}

/**
  \brief  Initializes the SDIF config
  \param[in]   idx sdif index
  \param[in]   host  sdif config descriptor \ref sdif_config_t
*/
void csi_sdif_config(sdif_handle_t handle, sdif_config_t *config)
{
	uint32_t pio_irqs = SDIF_INT_DATA_AVAIL | SDIF_INT_SPACE_AVAIL;
	uint32_t dma_irqs = SDIF_INT_DMA_END | SDIF_INT_ADMA_ERROR;
	SDIF_TYPE *base = (SDIF_TYPE *)handle;
	uint32_t sdif = csi_sdif_get_idx(base);
	uintptr_t BASE = (uintptr_t)base;
	static bool sd0_clock_state = false;
	static bool sd1_clock_state = false;
	static bool sd2_clock_state = false;

	if (BASE == SDIO0_BASE) {
		//printf("MMC_FLAG_SDCARD.\n");
		if (sd0_clock_state == false) {
#ifndef CONFIG_KERNEL_NONE
			aos_event_new(&param[sdif]._gcmdEvent, 0);
			aos_event_new(&param[sdif]._gdataEvent, 0);
#endif
			mmio_write_32(MMC_SDIO0_PLL_REGISTER, MMC_MAX_CLOCK_DIV_VALUE);
			mmio_clrbits_32(CLOCK_BYPASS_SELECT_REGISTER, BIT(6));
			sd0_clock_state = true;
		}
	} else if (BASE == SDIO1_BASE) {
		//printf("MMC_FLAG_SDIO.\n");
		if (sd1_clock_state == false) {
#ifndef CONFIG_KERNEL_NONE
			aos_event_new(&param[sdif]._gcmdEvent, 0);
			aos_event_new(&param[sdif]._gdataEvent, 0);
#endif
			mmio_write_32(MMC_SDIO1_PLL_REGISTER, MMC_MAX_CLOCK_DIV_VALUE);
			mmio_clrbits_32(CLOCK_BYPASS_SELECT_REGISTER, BIT(7));
			sd1_clock_state = true;
		}
	} else if (BASE == SDIO2_BASE) {
		//printf("MMC_FLAG_EMMC.\n");
		if (sd2_clock_state == false) {
#ifndef CONFIG_KERNEL_NONE
			aos_event_new(&param[sdif]._gcmdEvent, 0);
			aos_event_new(&param[sdif]._gdataEvent, 0);
#endif
			mmio_write_32(MMC_SDIO2_PLL_REGISTER, MMC_MAX_CLOCK_DIV_VALUE);
			mmio_clrbits_32(CLOCK_BYPASS_SELECT_REGISTER, BIT(5));
			sd2_clock_state = true;
		}
	}

	param[sdif].ier = SDIF_INT_BUS_POWER | SDIF_INT_DATA_END_BIT |
		SDIF_INT_DATA_CRC | SDIF_INT_DATA_TIMEOUT |
		SDIF_INT_INDEX | SDIF_INT_END_BIT | SDIF_INT_CRC |
		SDIF_INT_TIMEOUT | SDIF_INT_DATA_END | SDIF_INT_RESPONSE;
	param[sdif].ier = (param[sdif].ier & ~pio_irqs) | dma_irqs;

	SDIF_PhyInit(base, sdif);

	SDIF_Init(base, sdif);

}

/**
  \brief       Get driver capabilities.
  \param[in]   handle  SDIF handle to operate.
  \param[out]  capability SDIF capability information \ref sdif_capabilities_t
 */
void csi_sdif_get_capabilities(sdif_handle_t handle, sdif_capability_t *capability)
{
    SDIF_TYPE *base = (SDIF_TYPE *)handle;
    SDIF_GetCapability(base, capability);
}

uint32_t csi_sdif_get_controller_status(sdif_handle_t handle)
{
    SDIF_TYPE *base = (SDIF_TYPE *)handle;
    return base->STATUS;
}

/**
  \brief   send command to the card
  \param[in]  handle  SDIF handle to operate.
  \param[in]  cmd command configuration collection
  \param[in]  retries total number of tries
  \return  error code
 */
int32_t csi_sdif_send_command(sdif_handle_t handle, sdif_command_t *cmd, uint32_t retries)
{
	return true;
	/*
	SDIF_TYPE *base = (SDIF_TYPE *)handle;
	return SDIF_SendCommand(base, cmd, retries);
	*/
}

/**
  \brief  SDIF transfer function data/cmd in a non-blocking way
      this API should be use in interrupt mode, when use this API user
      must call csi_sdif_create_state first, all status check through
      interrupt
  \param[in] handle  SDIF handle to operate.
  \param[in] state sdif state information   \ref sdif_state_t
  \param[in] dma_config DMA config structure \ref sdif_dma_config_t
        This parameter can be config as:
        1. NULL
            In this condition, polling transfer mode is selected
        2. avaliable DMA config
            In this condition, DMA transfer mode is selected
  \param[in] transfer  sdif transfer configuration collection. \ref sdif_transfer_t
  \return error code
  */
int32_t csi_sdif_transfer(sdif_handle_t handle, sdif_dma_config_t *dma_config, sdif_transfer_t *transfer)
{
	SDIF_TYPE *base = handle;
	uint32_t sdif;
	static bool ignore_cmd = false;

	for (sdif = 0; sdif < CONFIG_SDIO_NUM; sdif++) {
		if (base == s_sdifBase[sdif]) {
			break;
		}
	}

	if (sdif >= CONFIG_SDIO_NUM) {
		return kStatus_InvalidArgument;
	}

	if (!ignore_cmd && transfer->command->index == 52) {
		ignore_cmd = true;
		s_sdifState[sdif]->callback.transfer_complete(sdif, NULL, kStatus_Success, NULL);
		return kStatus_Success;
	}

	return SDIF_TransferNonBlocking(base, &g_sdifState[sdif], dma_config, transfer);
}

/**
 \brief    Poll-wait for the response to the last command to be ready.  This
   function should be called even after sending commands that have no
   response (such as CMD0) to make sure that the hardware is ready to
   receive the next command.
 \param[in]  handle  SDIF handle to operate.
 \param[in]  cmd : The command that was sent.  See 32-bit command definitions above.
*/
void csi_sdif_transfer_abort(sdif_handle_t handle)
{

}


/**
 \brief Called after change in Bus width has been selected (via ACMD6).  Most
    controllers will need to perform some special operations to work
    correctly in the new bus mode.
 \param[in]  handle  SDIF handle to operate.
 \param[in]  bus_width wide bus mode \ref sdif_bus_width_e.
*/
void csi_sdif_bus_width(sdif_handle_t handle, sdif_bus_width_e bus_width)
{
	SDIF_TYPE *base = handle;
	uintptr_t BASE = (uintptr_t)base;
	u32 ctrl;
	u16 ctrl_2;

	ctrl = mmio_read_8(BASE + SDIF_HOST_CONTROL);
	if (bus_width == SDIF_BUS_1BIT_WIDTH)
		ctrl &= ~SDIF_DAT_XFER_WIDTH;
	else if (bus_width == SDIF_BUS_4BIT_WIDTH)
		ctrl |= SDIF_DAT_XFER_WIDTH;
	ctrl |= SDIF_CTRL_HISPD;

	ctrl_2 = mmio_read_16(BASE + SDIF_HOST_CONTROL2);
	ctrl_2 &= ~SDIF_CTRL_UHS_MASK;
	ctrl_2 |= SDIF_CTRL_UHS_SDR25;
	mmio_write_16(BASE + SDIF_HOST_CONTROL2, ctrl_2);
	mdelay(1);
	mmio_write_8(BASE + SDIF_HOST_CONTROL, ctrl);
}

uint32_t csi_sdif_get_clock(uint32_t index)
{
	uint32_t clk;
	switch (index)
	{
	case 0:
		clk = 375 * 1000 * 1000;
		break;
	case 1:
		clk = 375 * 1000 * 1000;
		break;
	default:
		break;
	}

 	clk = 375 * 1000 * 1000;

	return clk;
}

/**
 \brief Enable/disable SDIF clocking
 \param[in]  handle  SDIF handle to operate.
 \param[in]  target_hz card bus clock frequency united in Hz.
 \return code error
*/
uint32_t csi_sdif_set_clock(sdif_handle_t handle, uint32_t target_hz)
{
	uint32_t source_clock_hz;
	uint32_t idx;
	uint32_t ret;

	SDIF_TYPE *base = (SDIF_TYPE *)handle;

	idx = csi_sdif_get_idx(handle);
	source_clock_hz = csi_sdif_get_clock(idx);
	if (target_hz <= 400000)
		ret = SDIF_SetCardClock(base, source_clock_hz, target_hz);
	else
		ret = SDIF_ChangeCardClock(base, source_clock_hz, target_hz);

//	printf("BASE: 0x%lx, clk reg: 0x%x, freq: %u\n", (uintptr_t)base, mmio_read_32((uintptr_t)base + SDIF_CLK_CTRL), target_hz);
	return ret;
}

