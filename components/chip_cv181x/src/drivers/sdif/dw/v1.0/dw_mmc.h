/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _FSL_SDIF_H_
#define _FSL_SDIF_H_

#include <drv/sdif.h>
#include <drv/common.h>
#include <dw_mmc_reg.h>
#include <dw_sdmmc.h>
#include <mmio.h>
#include "aos/cli.h"

#ifdef __cplusplus
extern "C" {
#endif
/*!
 * @addtogroup sdif
 * @{
 */

/**********************************
 * Definitions.
 *****************************************************************************/

enum sdif_msg_prio {
	SDIF_ERR 	= 0x0001,
	SDIF_WARN	= 0x0002,
	SDIF_INFO	= 0x0004,
	SDIF_DBG 	= 0x0008,
	SDIF_VB2 	= 0x0010,
	SDIF_ISP_IRQ = 0x0020,
};

#ifndef BIT
#define BIT(nr)      (UINT64_C(1) << (nr))
#endif
#define sdif_printf(level, fmt, arg...) \
		do { \
			if (level <= SDIF_ISP_IRQ) \
				aos_cli_printf("[%s():%d] " fmt, __FUNCTION__, __LINE__, ## arg); \
		} while (0)

#define SDMMC_DMA_ALIGN_CACHE 64

/*! @name Driver version */
/*@{*/
/*! @brief Driver version 2.0.5. */
#define FSL_SDIF_DRIVER_VERSION (MAKE_VERSION(2U, 0U, 6U))
/*@}*/

#define SDIF_SD_CORE_ID         0
#define SDIF_SDIO_CORE_ID       1

/*! @brief  SDIOCLKCTRL setting
* Below clock delay setting should depend on specific platform, so
* it can be redefined when timing mismatch issue occur.
* Such as: response error/CRC error and so on
*/
/*! @brief clock range value which need to add delay to avoid timing issue */
#ifndef SDIF_CLOCK_RANGE_NEED_DELAY
#define SDIF_CLOCK_RANGE_NEED_DELAY (50000000U)
#endif

/*
* Fixed delay configuration
* min hold time:2ns
* min setup time: 6ns
* delay = (x+1)*250ps
*/
/*! @brief High speed mode clk_sample fixed delay*/
#ifndef SDIF_HIGHSPEED_SAMPLE_DELAY
#define SDIF_HIGHSPEED_SAMPLE_DELAY (0U)
#endif
/*! @brief High speed mode clk_drv fixed delay */
#ifndef SDIF_HIGHSPEED_DRV_DELAY
#define SDIF_HIGHSPEED_DRV_DELAY (0x1FU)
#endif

/*
* Phase shift delay configuration
* 0 degree: no delay
* 90 degree: 0.25/source clk value
* 180 degree: 0.50/source clk value
* 270 degree: 0.75/source clk value
*/
/*! @brief High speed mode clk_sample phase shift */
#ifndef SDIF_HIGHSPEED_SAMPLE_PHASE_SHIFT
#define SDIF_HIGHSPEED_SAMPLE_PHASE_SHIFT (0U)
#endif
/*! @brief High speed mode clk_drv phase shift */
#ifndef SDIF_HIGHSPEED_DRV_PHASE_SHIFT
#define SDIF_HIGHSPEED_DRV_PHASE_SHIFT (1U) /* 90 degrees clk_drv phase delay */
#endif

/*! @brief SDIF internal DMA descriptor address and the data buffer address align */
#define SDIF_INTERNAL_DMA_ADDR_ALIGN (4U)

/*! @brief SDIF status */
enum _sdif_status {
    kStatus_SDIF_DescriptorBufferLenError = MAKE_STATUS(kStatusGroup_SDIF, 0U), /*!< Set DMA descriptor failed */
    kStatus_SDIF_InvalidArgument = MAKE_STATUS(kStatusGroup_SDIF, 1U),          /*!< invalid argument status */
    kStatus_SDIF_SyncCmdTimeout = MAKE_STATUS(kStatusGroup_SDIF, 2U), /*!< sync command to CIU timeout status */
    kStatus_SDIF_SendCmdFail = MAKE_STATUS(kStatusGroup_SDIF, 3U),    /*!< send command to card fail */
    kStatus_SDIF_SendCmdErrorBufferFull =
        MAKE_STATUS(kStatusGroup_SDIF, 4U), /*!< send command to card fail, due to command buffer full
                                     user need to resend this command */
    kStatus_SDIF_DMATransferFailWithFBE =
        MAKE_STATUS(kStatusGroup_SDIF, 5U), /*!< DMA transfer data fail with fatal bus error ,
                                     to do with this error :issue a hard reset/controller reset*/
    kStatus_SDIF_DMATransferDescriptorUnavailable =
        MAKE_STATUS(kStatusGroup_SDIF, 6U),                             /*!< DMA descriptor unavailable */
    kStatus_SDIF_DataTransferFail = MAKE_STATUS(kStatusGroup_SDIF, 6U), /*!< transfer data fail */
    kStatus_SDIF_ResponseError = MAKE_STATUS(kStatusGroup_SDIF, 7U),    /*!< response error */
    kStatus_SDIF_DMAAddrNotAlign = MAKE_STATUS(kStatusGroup_SDIF, 8U),  /*!< DMA address not align */
};



/*! @brief define the reset type */
enum _sdif_reset_type {
    kSDIF_ResetController =
        SDIF_CTRL_CONTROLLER_RESET_MASK,                /*!< reset controller,will reset: BIU/CIU interface
                                                          CIU and state machine,ABORT_READ_DATA,SEND_IRQ_RESPONSE
                                                          and READ_WAIT bits of control register,START_CMD bit of the
                                                          command register*/
    kSDIF_ResetFIFO = SDIF_CTRL_FIFO_RESET_MASK,        /*!< reset data FIFO*/
    kSDIF_ResetDMAInterface = SDIF_CTRL_DMA_RESET_MASK, /*!< reset DMA interface */

    kSDIF_ResetAll = kSDIF_ResetController | kSDIF_ResetFIFO | /*!< reset all*/
                     kSDIF_ResetDMAInterface,
};


/*! @brief define the command flags */
enum _sdif_command_flags {
    kSDIF_CmdResponseExpect = SDIF_CMD_RESPONSE_EXPECT_MASK,      /*!< command request response*/
    kSDIF_CmdResponseLengthLong = SDIF_CMD_RESPONSE_LENGTH_MASK,  /*!< command response length long */
    kSDIF_CmdCheckResponseCRC = SDIF_CMD_CHECK_RESPONSE_CRC_MASK, /*!< request check command response CRC*/
    kSDIF_DataExpect = SDIF_CMD_DATA_EXPECTED_MASK,               /*!< request data transfer,either read/write*/
    kSDIF_DataWriteToCard = SDIF_CMD_READ_WRITE_MASK,             /*!< data transfer direction */
    kSDIF_DataStreamTransfer = SDIF_CMD_TRANSFER_MODE_MASK,    /*!< data transfer mode :stream/block transfer command */
    kSDIF_DataTransferAutoStop = SDIF_CMD_SEND_AUTO_STOP_MASK, /*!< data transfer with auto stop at the end of */
    kSDIF_WaitPretransfer_complete =
        SDIF_CMD_WAIT_PRVDATA_COMPLETE_MASK, /*!< wait pre transfer complete before sending this cmd  */
    kSDIF_TransferStopAbort =
        SDIF_CMD_STOP_ABORT_CMD_MASK, /*!< when host issue stop or abort cmd to stop data transfer
                                       ,this bit should set so that cmd/data state-machines of CIU can return
                                       to idle correctly*/
    kSDIF_SendInitialization =
        SDIF_CMD_SEND_INITIALIZATION_MASK, /*!< send initialization  80 clocks for SD card after power on  */
    kSDIF_CmdUpdateClockRegisterOnly =
        SDIF_CMD_UPDATE_CLOCK_REGISTERS_ONLY_MASK,                /*!< send cmd update the CIU clock register only */
    kSDIF_CmdtoReadCEATADevice = SDIF_CMD_READ_CEATA_DEVICE_MASK, /*!< host is perform read access to CE-ATA device */
    kSDIF_CmdExpectCCS = SDIF_CMD_CCS_EXPECTED_MASK,         /*!< command expect command completion signal signal */
    kSDIF_BootModeEnable = SDIF_CMD_ENABLE_BOOT_MASK,        /*!< this bit should only be set for mandatory boot mode */
    kSDIF_BootModeExpectAck = SDIF_CMD_EXPECT_BOOT_ACK_MASK, /*!< boot mode expect ack */
    kSDIF_BootModeDisable = SDIF_CMD_DISABLE_BOOT_MASK,      /*!< when software set this bit along with START_CMD, CIU
                                                                terminates the boot operation*/
    kSDIF_BootModeAlternate = SDIF_CMD_BOOT_MODE_MASK,       /*!< select boot mode ,alternate or mandatory*/
    kSDIF_CmdVoltageSwitch = SDIF_CMD_VOLT_SWITCH_MASK,      /*!< this bit set for CMD11 only */
    kSDIF_CmdDataUseHoldReg = SDIF_CMD_USE_HOLD_REG_MASK,    /*!< cmd and data send to card through the HOLD register*/
};

/*! @brief define the interrupt mask flags */
enum _sdif_interrupt_mask {
    kSDIF_CardDetect = SDIF_INTMASK_CDET_MASK,                 /*!< mask for card detect */
    kSDIF_ResponseError = SDIF_INTMASK_RE_MASK,                /*!< command response error */
    kSDIF_CommandDone = SDIF_INTMASK_CDONE_MASK,               /*!< command transfer over*/
    kSDIF_DataTransferOver = SDIF_INTMASK_DTO_MASK,            /*!< data transfer over flag*/
    kSDIF_WriteFIFORequest = SDIF_INTMASK_TXDR_MASK,           /*!< write FIFO request */
    kSDIF_ReadFIFORequest = SDIF_INTMASK_RXDR_MASK,            /*!< read FIFO request */
    kSDIF_ResponseCRCError = SDIF_INTMASK_RCRC_MASK,           /*!< response CRC error */
    kSDIF_DataCRCError = SDIF_INTMASK_DCRC_MASK,               /*!< data CRC error */
    kSDIF_ResponseTimeout = SDIF_INTMASK_RTO_MASK,             /*!< response timeout */
    kSDIF_DataReadTimeout = SDIF_INTMASK_DRTO_MASK,            /*!< read data timeout */
    kSDIF_DataStarvationByHostTimeout = SDIF_INTMASK_HTO_MASK, /*!< data starvation by host time out */
    kSDIF_FIFOError = SDIF_INTMASK_FRUN_MASK,                  /*!< indicate the FIFO under run or overrun error */
    kSDIF_HardwareLockError = SDIF_INTMASK_HLE_MASK,           /*!< hardware lock write error */
    kSDIF_DataStartBitError = SDIF_INTMASK_SBE_MASK,           /*!< start bit error */
    kSDIF_AutoCmdDone = SDIF_INTMASK_ACD_MASK,                 /*!< indicate the auto command done */
    kSDIF_DataEndBitError = SDIF_INTMASK_EBE_MASK,             /*!< end bit error */
    kSDIF_SDIOInterrupt = SDIF_INTMASK_SDIO_INT_MASK_MASK,     /*!< interrupt from the SDIO card */

    kSDIF_CommandTransferStatus = kSDIF_ResponseError | kSDIF_CommandDone | kSDIF_ResponseCRCError |
                                  kSDIF_ResponseTimeout |
                                  kSDIF_HardwareLockError, /*!< command transfer status collection*/
    kSDIF_DataTransferStatus = kSDIF_DataTransferOver | kSDIF_WriteFIFORequest | kSDIF_ReadFIFORequest |
                               kSDIF_DataCRCError | kSDIF_DataReadTimeout | kSDIF_DataStarvationByHostTimeout |
                               kSDIF_FIFOError | kSDIF_DataStartBitError | kSDIF_DataEndBitError |
                               kSDIF_AutoCmdDone, /*!< data transfer status collection */
    kSDIF_DataTransferError =
        kSDIF_DataCRCError | kSDIF_FIFOError | kSDIF_DataStartBitError | kSDIF_DataEndBitError | kSDIF_DataReadTimeout,
    kSDIF_AllInterruptStatus = 0x1FFFFU, /*!< all interrupt mask */

};

/*! @brief define the internal DMA status flags */
enum _sdif_dma_status {
    kSDIF_DMATransFinishOneDescriptor = SDIF_IDSTS_TI_MASK, /*!< DMA transfer finished for one DMA descriptor */
    kSDIF_DMARecvFinishOneDescriptor = SDIF_IDSTS_RI_MASK,  /*!< DMA receive finished for one DMA descriptor */
    kSDIF_DMAFatalBusError = SDIF_IDSTS_FBE_MASK,           /*!< DMA fatal bus error */
    kSDIF_DMADescriptorUnavailable = SDIF_IDSTS_DU_MASK,    /*!< DMA descriptor unavailable */
    kSDIF_DMACardErrorSummary = SDIF_IDSTS_CES_MASK,        /*!< card error summary */
    kSDIF_NormalInterruptSummary = SDIF_IDSTS_NIS_MASK,     /*!< normal interrupt summary */
    kSDIF_AbnormalInterruptSummary = SDIF_IDSTS_AIS_MASK,   /*!< abnormal interrupt summary*/

    kSDIF_DMAAllStatus = kSDIF_DMATransFinishOneDescriptor | kSDIF_DMARecvFinishOneDescriptor | kSDIF_DMAFatalBusError |
                         kSDIF_DMADescriptorUnavailable | kSDIF_DMACardErrorSummary | kSDIF_NormalInterruptSummary |
                         kSDIF_AbnormalInterruptSummary,

};

/*! @brief define the internal DMA descriptor flag */
enum _sdif_dma_descriptor_flag {
    kSDIF_DisableCompleteInterrupt = 0x2U,     /*!< disable the complete interrupt flag for the ends
                                                in the buffer pointed to by this descriptor*/
    kSDIF_DMADescriptorDataBufferEnd = 0x4U,   /*!< indicate this descriptor contain the last data buffer of data */
    kSDIF_DMADescriptorDataBufferStart = 0x8U, /*!< indicate this descriptor contain the first data buffer
                                                 of data,if first buffer size is 0,next descriptor contain
                                                 the begin of the data*/
    kSDIF_DMASecondAddrChained = 0x10U,        /*!< indicate that the second addr in the descriptor is the
                                               next descriptor addr not the data buffer */
    kSDIF_DMADescriptorEnd = 0x20U,            /*!< indicate that the descriptor list reached its final descriptor*/
    kSDIF_DMADescriptorOwnByDMA = 0x80000000U, /*!< indicate the descriptor is own by SD/MMC DMA */
};
/*! @brief define the internal DMA descriptor */
typedef struct _sdif_dma_descriptor {
    uint32_t dmaDesAttribute;           /*!< internal DMA attribute control and status */
    uint32_t dmaDataBufferSize;         /*!< internal DMA transfer buffer size control */
    const uint32_t *dmaDataBufferAddr0; /*!< internal DMA buffer 0 addr ,the buffer size must be 32bit aligned */
    const uint32_t *dmaDataBufferAddr1; /*!< internal DMA buffer 1 addr ,the buffer size must be 32bit aligned */

} sdif_dma_descriptor_t;



/*************************************************************************************************
 * API
 ************************************************************************************************/

/*!
 * @brief SDIF module initialization function.
 *
 * Configures the SDIF according to the user configuration.
 * @param base SDIF peripheral base address.
 * @param config SDIF configuration information.
 */
//void SDIF_Init(SDIF_TYPE *base, sdif_config_t *config);

/*!
 * @brief SDIF module deinit function.
 * user should call this function follow with IP reset
 * @param base SDIF peripheral base address.
 */
//void SDIF_Deinit(SDIF_TYPE *base);

/*!
 * @brief SDIF send initialize 80 clocks for SD card after initial
 * @param base SDIF peripheral base address.
 * @param timeout value
 */
bool SDIF_SendCardActive(SDIF_TYPE *base, uint32_t timeout);
void sdhci_pinmux_dumpregs();
/*!
 * @brief SDIF module detect card insert status function.
 * @param base SDIF peripheral base address.
 * @param data3 indicate use data3 as card insert detect pin
 * @retval 1 card is inserted
 *         0 card is removed
 */
static inline uint32_t SDIF_DetectCardInsert(SDIF_TYPE *base, bool data3)
{
     uintptr_t BASE = (uintptr_t)base;
    if (data3) {
        return (mmio_read_32(BASE+SDIF_PRESENT_STATE) & SDIF_CARD_STABLE) == SDIF_CARD_STABLE ? 0U : 1U;
    } else {
        return (mmio_read_32(BASE+SDIF_PRESENT_STATE) & SDIF_CARD_INSERTED) == SDIF_CARD_INSERTED ? 1U : 0U;
    }
}

/*!
 * @brief SDIF module enable/disable card clock.
 * @param base SDIF peripheral base address.
 * @param enable/disable flag
 */
static inline void SDIF_EnableCardClock(SDIF_TYPE *base, bool enable)
{
    uintptr_t BASE = (uintptr_t)base;
    if (enable) {
         mmio_write_16(BASE + SDIF_CLK_CTRL,
				mmio_read_32(BASE + SDIF_CLK_CTRL) | (0x1<<2)); // stop SD clock
    } else {
        mmio_write_16(BASE + SDIF_CLK_CTRL,
				mmio_read_32(BASE + SDIF_CLK_CTRL) & ~(0x1<<2)); // stop SD clock
    }
}

/*!
 * @brief SDIF module enable/disable module disable the card clock
 * to enter low power mode when card is idle,for SDIF cards, if
 * interrupts must be detected, clock should not be stopped
 * @param base SDIF peripheral base address.
 * @param enable/disable flag
 */
static inline void SDIF_EnableLowPowerMode(SDIF_TYPE *base, bool enable)
{
    if (enable) {
        base->CLKENA |= SDIF_CLKENA_CCLK_LOW_POWER_MASK;
    } else {
        base->CLKENA &= ~SDIF_CLKENA_CCLK_LOW_POWER_MASK;
    }
}

/*!
 * @brief Sets the card bus clock frequency.
 *
 * @param base SDIF peripheral base address.
 * @param srcClock_Hz SDIF source clock frequency united in Hz.
 * @param target_HZ card bus clock frequency united in Hz.
 * @return The nearest frequency of busClock_Hz configured to SD bus.
 */
uint32_t SDIF_SetCardClock(SDIF_TYPE *base, uint32_t srcClock_Hz, uint32_t target_HZ);

/*!
 * @brief reset the different block of the interface.
 * @param base SDIF peripheral base address.
 * @param mask indicate which block to reset.
 * @param timeout value,set to wait the bit self clear
 * @return reset result.
 */
//bool SDIF_Reset(SDIF_TYPE *base, uint32_t mask, uint32_t timeout);

/*!
 * @brief enable/disable the card power.
 * once turn power on, software should wait for regulator/switch
 * ramp-up time before trying to initialize card.
 * @param base SDIF peripheral base address.
 * @param enable/disable flag.
 */
static inline void SDIF_EnableCardPower(SDIF_TYPE *base, bool enable)
{
    return;
    uintptr_t BASE = (uintptr_t)base;
    if (enable) {
        mmio_write_8(BASE + SDIF_PWR_CONTROL,mmio_read_8(BASE + SDIF_PWR_CONTROL) | 0x1);
    } else {
        mmio_write_8(BASE + SDIF_PWR_CONTROL,mmio_read_8(BASE+ SDIF_PWR_CONTROL) & ~0x1);
    }
}

/*!
 * @brief get the card write protect status
 * @param base SDIF peripheral base address.
 */
static inline uint32_t SDIF_GetCardWriteProtect(SDIF_TYPE *base)
{
    return base->WRTPRT & SDIF_WRTPRT_WRITE_PROTECT_MASK;
}



/*!
 * @brief toggle state on hardware reset PIN
 * This is used which card has a reset PIN typically.
 * @param base SDIF peripheral base address.
 */
static inline void SDIF_AssertHardwareReset(SDIF_TYPE *base)
{
    base->RST_N &= ~SDIF_RST_N_CARD_RESET_MASK;
}

/*!
 * @brief Get the instance.
 *
 * @param base SDIF peripheral base address.
 * @return Instance number.
 */
 uint32_t SDIF_GetInstance(SDIF_TYPE *base);

/*!
 * @brief send command to the card
 * @param base SDIF peripheral base address.
 * @param command configuration collection
 * @param timeout value
 * @return command excute status
 */
//status_t SDIF_SendCommand(SDIF_TYPE *base, sdif_command_t *cmd, uint32_t timeout);

/*!
 * @brief SDIF enable/disable global interrupt
 * @param base SDIF peripheral base address.
 * @param enable/disable flag
 */
static inline void SDIF_EnableGlobalInterrupt(SDIF_TYPE *base, bool enable)
{
    return;
    uintptr_t BASE = (uintptr_t)base;
    sdif_printf(SDIF_DBG,"SDIF_EnableGlobalInterrupt BASE:%x \n",BASE);
    if (enable) {
        mmio_write_16(BASE + SDIF_INT_STATUS_EN, mmio_read_16(BASE + SDIF_INT_STATUS_EN) | 0xFF);
    } else {
        mmio_write_16(BASE + SDIF_INT_STATUS_EN, mmio_read_16(BASE + SDIF_INT_STATUS_EN) & ~0xFF);
    }
}

/*!
 * @brief SDIF enable interrupt
 * @param base SDIF peripheral base address.
 * @param interrupt mask
 */
static inline void SDIF_EnableInterrupt(SDIF_TYPE *base, uint32_t mask)
{
    return;
    base->INTMASK |= mask;
}

/*!
 * @brief SDIF disable interrupt
 * @param base SDIF peripheral base address.
 * @param interrupt mask
 */
static inline void SDIF_DisableInterrupt(SDIF_TYPE *base, uint32_t mask)
{
    return;
    base->INTMASK &= ~mask;
}

/*!
 * @brief SDIF get interrupt status
 * @param base SDIF peripheral base address.
 */
static inline uint32_t SDIF_GetInterruptStatus(SDIF_TYPE *base)
{
    return base->RINTSTS;
}

/*!
 * @brief SDIF clear interrupt status
 * @param base SDIF peripheral base address.
 * @param status mask to clear
 */
static inline void SDIF_ClearInterruptStatus(SDIF_TYPE *base, uint32_t mask)
{
    base->RINTSTS &= mask;
}

/*!
 * @brief Creates the SDIF handle.
 * register call back function for interrupt and enable the interrupt
 * @param base SDIF peripheral base address.
 * @param handle SDIF handle pointer.
 * @param callback Structure pointer to contain all callback functions.
 * @param user_data Callback function parameter.
 */
//void SDIF_TransferCreateHandle(SDIF_TYPE *base,
//                             sdif_handle_t *handle,
//                            sdif_callback_t *callback,
//                           void *user_data);

/*!
 * @brief SDIF enable DMA interrupt
 * @param base SDIF peripheral base address.
 * @param interrupt mask to set
 */
static inline void SDIF_EnableDmaInterrupt(SDIF_TYPE *base, uint32_t mask)
{
    base->IDINTEN |= mask;
}

/*!
 * @brief SDIF disable DMA interrupt
 * @param base SDIF peripheral base address.
 * @param interrupt mask to clear
 */
static inline void SDIF_DisableDmaInterrupt(SDIF_TYPE *base, uint32_t mask)
{
    base->IDINTEN &= ~mask;
}

/*!
 * @brief SDIF get internal DMA status
 * @param base SDIF peripheral base address.
 * @return the internal DMA status register
 */
static inline uint32_t SDIF_GetInternalDMAStatus(SDIF_TYPE *base)
{
    return base->IDSTS;
}

/*!
 * @brief SDIF clear internal DMA status
 * @param base SDIF peripheral base address.
 * @param status mask to clear
 */
static inline void SDIF_ClearInternalDMAStatus(SDIF_TYPE *base, uint32_t mask)
{
    base->IDSTS &= mask;
}

/*!
 * @brief SDIF internal DMA config function
 * @param base SDIF peripheral base address.
 * @param internal DMA configuration collection
 * @param data buffer pointer
 * @param data buffer size
 */
//status_t SDIF_InternalDMAConfig(SDIF_TYPE *base, sdif_dma_config_t *config, const uint32_t *data, uint32_t dataSize);

/*!
 * @brief SDIF internal DMA enable
 * @param base SDIF peripheral base address.
 * @param enable internal DMA enable or disable flag.
 */
static inline void SDIF_EnableInternalDMA(SDIF_TYPE *base, bool enable)
{
    if (enable) {
        /* use internal DMA interface */
        base->CTRL |= SDIF_CTRL_USE_INTERNAL_DMAC_MASK;
        /* enable the internal SD/MMC DMA */
        base->BMOD |= SDIF_BMOD_DE_MASK;
    } else {
        /* use internal DMA interface */
        base->CTRL &= ~SDIF_CTRL_USE_INTERNAL_DMAC_MASK;
        /* enable the internal SD/MMC DMA */
        base->BMOD &= ~SDIF_BMOD_DE_MASK;
    }
}

/*!
 * @brief SDIF send read wait to SDIF card function
 * @param base SDIF peripheral base address.
 */
static inline void SDIF_SendReadWait(SDIF_TYPE *base)
{
    base->CTRL |= SDIF_CTRL_READ_WAIT_MASK;
}

/*!
 * @brief SDIF abort the read data when SDIF card is in suspend state
 * Once assert this bit,data state machine will be reset which is waiting for the
 * next blocking data,used in SDIO card suspend sequence,should call after suspend
 * cmd send
 * @param base SDIF peripheral base address.
 * @param timeout value to wait this bit self clear which indicate the data machine
 * reset to idle
 */
//bool SDIF_AbortReadData(SDIF_TYPE *base, uint32_t timeout);

/*!
 * @brief SDIF enable/disable CE-ATA card interrupt
 * this bit should set together with the card register
 * @param base SDIF peripheral base address.
 * @param enable/disable flag
 */
static inline void SDIF_EnableCEATAInterrupt(SDIF_TYPE *base, bool enable)
{
    if (enable) {
        base->CTRL |= SDIF_CTRL_CEATA_DEVICE_INTERRUPT_STATUS_MASK;
    } else {
        base->CTRL &= ~SDIF_CTRL_CEATA_DEVICE_INTERRUPT_STATUS_MASK;
    }
}

/*!
 * @brief SDIF transfer function data/cmd in a non-blocking way
 * this API should be use in interrupt mode, when use this API user
 * must call SDIF_TransferCreateHandle first, all status check through
 * interrupt
 * @param base SDIF peripheral base address.
 * @param sdif handle
 * @param DMA config structure
 *  This parameter can be config as:
 *      1. NULL
            In this condition, polling transfer mode is selected
        2. avaliable DMA config
            In this condition, DMA transfer mode is selected
 * @param sdif transfer configuration collection
 */
//status_t SDIF_TransferNonBlocking(SDIF_TYPE *base,
//                                  sdif_state_t *handle,
//                                  sdif_dma_config_t *dmaConfig,
//                                  sdif_transfer_t *transfer);

/*!
 * @brief SDIF transfer function data/cmd in a blocking way
 * @param base SDIF peripheral base address.
 * @param DMA config structure
 *       1. NULL
 *           In this condition, polling transfer mode is selected
 *       2. avaliable DMA config
 *           In this condition, DMA transfer mode is selected
 * @param sdif transfer configuration collection
 */
status_t SDIF_TransferBlocking(SDIF_TYPE *base, sdif_dma_config_t *dmaConfig, sdif_transfer_t *transfer);

/*!
 * @brief SDIF release the DMA descriptor to DMA engine
 * this function should be called when DMA descriptor unavailable status occurs
 * @param base SDIF peripheral base address.
 * @param sdif DMA config pointer
 */
//status_t SDIF_ReleaseDMADescriptor(SDIF_TYPE *base, sdif_dma_config_t *dmaConfig);

/*!
 * @brief SDIF return the controller capability
 * @param base SDIF peripheral base address.
 * @param sdif capability pointer
 */
//void SDIF_GetCapability(SDIF_TYPE *base, sdif_capability_t *capability);

/*!
 * @brief SDIF return the controller status
 * @param base SDIF peripheral base address.
 */
static inline uint32_t SDIF_GetControllerStatus(SDIF_TYPE *base)
{
    return base->STATUS;
}

/*!
 * @brief SDIF send command  complete signal disable to CE-ATA card
 * @param base SDIF peripheral base address.
 * @param send auto stop flag
 */
static inline void SDIF_SendCCSD(SDIF_TYPE *base, bool withAutoStop)
{
    if (withAutoStop) {
        base->CTRL |= SDIF_CTRL_SEND_CCSD_MASK | SDIF_CTRL_SEND_AUTO_STOP_CCSD_MASK;
    } else {
        base->CTRL |= SDIF_CTRL_SEND_CCSD_MASK;
    }
}

static inline void SDIF_ConfigRegExt(SDIF_TYPE *base, uint32_t value)
{
    base->UHS_REG_EXT &= ~0x30000;
    base->UHS_REG_EXT |= value << 16;
}

/*!
 * @brief SDIF clock delay config
 * param smpl_phase
 * param smpl_dly
 * param drv_phase
 * param drv_dly
 */
static inline void SDIF_Clock_Delay(SDIF_TYPE *base, uint32_t smpl_phase, uint32_t smpl_dly, uint32_t drv_phase, uint32_t drv_dly)
{
    uint32_t uhs_reg_ext = base->UHS_REG_EXT;

    uhs_reg_ext &= ~SDIF_UHS_REG_EXT_CLK_SMPL_PHASE_MASK;
    uhs_reg_ext |= SDIF_UHS_REG_EXT_CLK_SMPL_PHASE(smpl_phase);

    uhs_reg_ext &= ~SDIF_UHS_REG_EXT_CLK_SMPL_DLY_MASK;
    uhs_reg_ext |= SDIF_UHS_REG_EXT_CLK_SMPL_DLY(smpl_dly);

    uhs_reg_ext &= ~SDIF_UHS_REG_EXT_CLK_DRV_PHASE_MASK;
    uhs_reg_ext |= SDIF_UHS_REG_EXT_CLK_DRV_PHASE(drv_phase);

    uhs_reg_ext &= ~SDIF_UHS_REG_EXT_CLK_DRV_DLY_MASK;
    uhs_reg_ext |= SDIF_UHS_REG_EXT_CLK_DRV_DLY(drv_dly);

    base->UHS_REG_EXT = uhs_reg_ext;
}

/*!
 * @brief SDIF config the clock delay
 * This function is used to config the cclk_in delay to
 * sample and driver the data ,should meet the min setup
 * time and hold time, and user need to config this parameter
 * according to your board setting
 * @param target freq work mode
 * @param clock divider which is used to decide if use phase shift for delay
 */
//void SDIF_ConfigClockDelay(uint32_t target_HZ, uint32_t divider);

//void SDIF_Binding(SDIF_TYPE **base, uint32_t sdif);


/* @} */

#if defined(__cplusplus)
}
#endif
/*! @} */

#endif /* _FSL_sdif_H_*/
