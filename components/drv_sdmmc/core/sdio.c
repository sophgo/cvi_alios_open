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

#include "sdio.h"
#include "soc.h"
#include "assert.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief define the tuple number will be read during init */
#define SDIO_COMMON_CIS_TUPLE_NUM (3U)
/*! @brief SDIO retry times */
#define SDIO_RETRY_TIMES (32U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief send card operation condition
 * @param card Card descriptor.
 * @param command argment
 *  argument = 0U , means to get the operation condition
 *  argument !=0 , set the operation condition register
 */
static status_t SDIO_SendOperationCondition(sdio_card_t *card, uint32_t argument);

/*!
 * @brief card Send relative address
 * @param card Card descriptor.
 */
static status_t SDIO_SendRca(sdio_card_t *card);

/*!
 * @brief card select card
 * @param card Card descriptor.
 * @param select/diselect flag
 */
static status_t inline SDIO_SelectCard(sdio_card_t *card, bool isSelected);

/*!
 * @brief card go idle
 * @param card Card descriptor.
 */
static status_t inline SDIO_GoIdle(sdio_card_t *card);

/*!
 * @brief decode CIS
 * @param card Card descriptor.
 * @param func number
 * @param data buffer pointer
 * @param tuple code
 * @param tuple link
 */
static status_t SDIO_DecodeCIS(
    sdio_card_t *card, sdio_func_num_t func, uint8_t *dataBuffer, uint32_t tplCode, uint32_t tplLink);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* define the tuple list */
static const uint32_t g_tupleList[SDIO_COMMON_CIS_TUPLE_NUM] = {
    SDIO_TPL_CODE_MANIFID, SDIO_TPL_CODE_FUNCID, SDIO_TPL_CODE_FUNCE,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
static status_t inline SDIO_SelectCard(sdio_card_t *card, bool isSelected)
{
    assert(card);

    return SDMMC_SelectCard(card->host.base, card->host.transfer, card->relativeAddress, isSelected);
}

static status_t inline SDIO_GoIdle(sdio_card_t *card)
{
    assert(card);

    return SDMMC_GoIdle(card->host.base, card->host.transfer);
}

static status_t SDIO_SendRca(sdio_card_t *card)
{
    assert(card);

    uint32_t i = FSL_SDMMC_MAX_CMD_RETRIES;

    SDMMCHOST_TRANSFER content = {0};
    SDMMCHOST_COMMAND command = {0};

    command.index = kSDIO_SendRelativeAddress;
    command.argument = 0U;
    command.response_type = SDIF_CARD_RESPONSE_R6;
    command.resopnse_error_flags = kSDIO_StatusR6Error | kSDIO_StatusIllegalCmd | kSDIO_StatusCmdCRCError;

    content.command = &command;
    content.data = NULL;

    while (--i) {
        if (kStatus_Success == card->host.transfer(card->host.base, &content)) {
            /* check illegal state and cmd CRC error, may be the voltage or clock not stable, retry the cmd*/
            if (command.response[0U] & (kSDIO_StatusIllegalCmd | kSDIO_StatusCmdCRCError)) {
                continue;
            }

            card->relativeAddress = (command.response[0U] >> 16U);

            return kStatus_Success;
        }
    }

    return kStatus_SDMMC_TransferFailed;
}

status_t SDIO_CardInActive(sdio_card_t *card)
{
    assert(card);

    return SDMMC_SetCardInactive(card->host.base, card->host.transfer);
}

static status_t SDIO_SendOperationCondition(sdio_card_t *card, uint32_t argument)
{
    assert(card);

    SDMMCHOST_TRANSFER content = {0U};
    SDMMCHOST_COMMAND command = {0U};
    uint32_t i = SDIO_RETRY_TIMES;

    command.index = kSDIO_SendOperationCondition;
    command.argument = argument;
    command.response_type = SDIF_CARD_RESPONSE_R4;

    content.command = &command;
    content.data = NULL;

    while (--i) {
        if (kStatus_Success != card->host.transfer(card->host.base, &content) || (command.response[0U] == 0U)) {
            continue;
        }

        /* if argument equal 0, then should check and save the info */
        if (argument == 0U) {
            /* check if memory present */
            if ((command.response[0U] & kSDIO_OcrMemPresent) == kSDIO_OcrMemPresent) {
                card->memPresentFlag = true;
            }

            /* save the io number */
            card->ioTotalNumber = (command.response[0U] & kSDIO_OcrIONumber) >> 28U;
            /* save the operation condition */
            card->ocr = command.response[0U] & 0xFFFFFFU;

            break;
        }
        /* wait the card is ready for after initialization */
        else if (command.response[0U] & kSDIO_OcrPowerUpBusyFlag) {
            break;
        }
    }

    return ((i != 0U) ? kStatus_Success : kStatus_Fail);
}

status_t SDIO_IO_Write_Direct(sdio_card_t *card, sdio_func_num_t func, uint32_t regAddr, uint8_t *data, bool raw)
{
    assert(card);
    assert(func <= kSDIO_FunctionNum7);

    SDMMCHOST_TRANSFER content = {0U};
    SDMMCHOST_COMMAND command = {0U};

    command.index = kSDIO_RWIODirect;
    command.argument = (func << SDIO_CMD_ARGUMENT_FUNC_NUM_POS) |
                       ((regAddr & SDIO_CMD_ARGUMENT_REG_ADDR_MASK) << SDIO_CMD_ARGUMENT_REG_ADDR_POS) |
                       (1U << SDIO_CMD_ARGUMENT_RW_POS) | ((raw ? 1U : 0U) << SDIO_DIRECT_CMD_ARGUMENT_RAW_POS) |
                       (*data & SDIO_DIRECT_CMD_DATA_MASK);
    command.response_type = SDIF_CARD_RESPONSE_R5;
    command.resopnse_error_flags = (kSDIO_StatusCmdCRCError | kSDIO_StatusIllegalCmd | kSDIO_StatusError |
                                    kSDIO_StatusFunctionNumError | kSDIO_StatusOutofRange);

    content.command = &command;
    content.data = NULL;

    if (kStatus_Success != card->host.transfer(card->host.base, &content)) {
        return kStatus_SDMMC_TransferFailed;
    }

    /* read data from response */
    *data = command.response[0U] & SDIO_DIRECT_CMD_DATA_MASK;

    return kStatus_Success;
}

status_t SDIO_IO_Read_Direct(sdio_card_t *card, sdio_func_num_t func, uint32_t regAddr, uint8_t *data)
{
    assert(card);
    assert(func <= kSDIO_FunctionNum7);

    SDMMCHOST_TRANSFER content = {0U};
    SDMMCHOST_COMMAND command = {0U};

    command.index = kSDIO_RWIODirect;
    command.argument = (func << SDIO_CMD_ARGUMENT_FUNC_NUM_POS) |
                       ((regAddr & SDIO_CMD_ARGUMENT_REG_ADDR_MASK) << SDIO_CMD_ARGUMENT_REG_ADDR_POS);
    command.response_type = SDIF_CARD_RESPONSE_R5;
    command.resopnse_error_flags = (kSDIO_StatusCmdCRCError | kSDIO_StatusIllegalCmd | kSDIO_StatusError |
                                    kSDIO_StatusFunctionNumError | kSDIO_StatusOutofRange);

    content.command = &command;
    content.data = NULL;

    if (kStatus_Success != card->host.transfer(card->host.base, &content)) {
        return kStatus_SDMMC_TransferFailed;
    }

    /* read data from response */
    *data = command.response[0U] & SDIO_DIRECT_CMD_DATA_MASK;

    return kStatus_Success;
}

status_t SDIO_IO_Write_Extended(
    sdio_card_t *card, sdio_func_num_t func, uint32_t regAddr, uint8_t *buffer, uint32_t count, uint32_t flags)
{
    assert(card);
    assert(buffer);
    assert(func <= kSDIO_FunctionNum7);

    SDMMCHOST_TRANSFER content = {0U};
    SDMMCHOST_COMMAND command = {0U};
    SDMMCHOST_DATA data = {0U};
    bool blockMode = false;
    bool opCode = false;

    /* check if card support block mode */
    if ((card->cccrflags & kSDIO_CCCRSupportMultiBlock) && (flags & SDIO_EXTEND_CMD_BLOCK_MODE_MASK)) {
        blockMode = true;
    }

    if (flags & SDIO_EXTEND_CMD_OP_CODE_MASK) {
        opCode = true;
    }

    /* check the byte size counter in non-block mode
    * so you need read CIS for each function first,before you do read/write
    */
    if (!blockMode) {
        if ((func == kSDIO_FunctionNum0) && (card->commonCIS.fn0MaxBlkSize != 0U) &&
            (count > card->commonCIS.fn0MaxBlkSize)) {
            return kStatus_SDMMC_SDIO_InvalidArgument;
        } else if ((func != kSDIO_FunctionNum0) && (card->funcCIS[func - 1U].ioMaxBlockSize != 0U) &&
                   (count > card->funcCIS[func - 1U].ioMaxBlockSize)) {
            return kStatus_SDMMC_SDIO_InvalidArgument;
        }
    }

    command.index = kSDIO_RWIOExtended;
    command.argument = (func << SDIO_CMD_ARGUMENT_FUNC_NUM_POS) |
                       ((regAddr & SDIO_CMD_ARGUMENT_REG_ADDR_MASK) << SDIO_CMD_ARGUMENT_REG_ADDR_POS) |
                       (1U << SDIO_CMD_ARGUMENT_RW_POS) | (count & SDIO_EXTEND_CMD_COUNT_MASK) |
                       ((blockMode ? 1 : 0) << SDIO_EXTEND_CMD_ARGUMENT_BLOCK_MODE_POS |
                        ((opCode ? 1 : 0) << SDIO_EXTEND_CMD_ARGUMENT_OP_CODE_POS));
    command.response_type = SDIF_CARD_RESPONSE_R5;
    command.resopnse_error_flags = (kSDIO_StatusCmdCRCError | kSDIO_StatusIllegalCmd | kSDIO_StatusError |
                                    kSDIO_StatusFunctionNumError | kSDIO_StatusOutofRange);

    if (blockMode) {
        if (func == kSDIO_FunctionNum0) {
            data.block_size = card->io0block_size;
        } else {
            data.block_size = card->ioFBR[func - 1U].ioBlockSize;
        }

        data.block_count = count;
    } else {
        data.block_size = count;
        data.block_count = 1U;
    }

    data.tx_data_buffer = (uint32_t *)buffer;

    content.command = &command;
    content.data = &data;

    if (kStatus_Success != card->host.transfer(card->host.base, &content)) {
        return kStatus_SDMMC_TransferFailed;
    }

    return kStatus_Success;
}

status_t SDIO_IO_Read_Extended(
    sdio_card_t *card, sdio_func_num_t func, uint32_t regAddr, uint8_t *buffer, uint32_t count, uint32_t flags)
{
    assert(card);
    assert(buffer);
    assert(func <= kSDIO_FunctionNum7);

    SDMMCHOST_TRANSFER content = {0U};
    SDMMCHOST_COMMAND command = {0U};
    SDMMCHOST_DATA data = {0U};
    bool blockMode = false;
    bool opCode = false;

    /* check if card support block mode */
    if ((card->cccrflags & kSDIO_CCCRSupportMultiBlock) && (flags & SDIO_EXTEND_CMD_BLOCK_MODE_MASK)) {
        blockMode = true;
    }

    /* op code =0 : read/write to fixed addr
    *  op code =1 :read/write addr incrementing
     */
    if (flags & SDIO_EXTEND_CMD_OP_CODE_MASK) {
        opCode = true;
    }

    /* check the byte size counter in non-block mode
    * so you need read CIS for each function first,before you do read/write
    */
    if (!blockMode) {
        if ((func == kSDIO_FunctionNum0) && (card->commonCIS.fn0MaxBlkSize != 0U) &&
            (count > card->commonCIS.fn0MaxBlkSize)) {
            return kStatus_SDMMC_SDIO_InvalidArgument;
        } else if ((func != kSDIO_FunctionNum0) && (card->funcCIS[func - 1U].ioMaxBlockSize != 0U) &&
                   (count > card->funcCIS[func - 1U].ioMaxBlockSize)) {
            return kStatus_SDMMC_SDIO_InvalidArgument;
        }
    }

    command.index = kSDIO_RWIOExtended;
    command.argument = (func << SDIO_CMD_ARGUMENT_FUNC_NUM_POS) |
                       ((regAddr & SDIO_CMD_ARGUMENT_REG_ADDR_MASK) << SDIO_CMD_ARGUMENT_REG_ADDR_POS) |
                       (count & SDIO_EXTEND_CMD_COUNT_MASK) |
                       ((blockMode ? 1U : 0U) << SDIO_EXTEND_CMD_ARGUMENT_BLOCK_MODE_POS |
                        ((opCode ? 1U : 0U) << SDIO_EXTEND_CMD_ARGUMENT_OP_CODE_POS));
    command.response_type = SDIF_CARD_RESPONSE_R5;
    command.resopnse_error_flags = (kSDIO_StatusCmdCRCError | kSDIO_StatusIllegalCmd | kSDIO_StatusError |
                                    kSDIO_StatusFunctionNumError | kSDIO_StatusOutofRange);

    if (blockMode) {
        if (func == kSDIO_FunctionNum0) {
            data.block_size = card->io0block_size;
        } else {
            data.block_size = card->ioFBR[func - 1U].ioBlockSize;
        }

        data.block_count = count;
    } else {
        data.block_size = count;
        data.block_count = 1U;
    }

    data.rx_date_buffer = (uint32_t *)buffer;

    content.command = &command;
    content.data = &data;

    if (kStatus_Success != card->host.transfer(card->host.base, &content)) {
        return kStatus_SDMMC_TransferFailed;
    }

    return kStatus_Success;
}

status_t SDIO_GetCardCapability(sdio_card_t *card, sdio_func_num_t func)
{
    assert(card);
    assert(func <= kSDIO_FunctionNum7);

    uint8_t tempBuffer[20] = {0U};
    uint32_t i = 0U;

    for (i = 0U; i < 20U; i++) {
        if (kStatus_Success != SDIO_IO_Read_Direct(card, kSDIO_FunctionNum0, SDIO_FBR_BASE(func) + i, &tempBuffer[i])) {
            return kStatus_SDMMC_TransferFailed;
        }
    }

    switch (func) {
        case kSDIO_FunctionNum0:

            card->sd_version = tempBuffer[1U];
            card->sdioVersion = tempBuffer[0U] >> 4U;
            card->cccrVersioin = tempBuffer[0U] & 0xFU;

            /* continuous SPI interrupt */
            if (tempBuffer[7U] & 0x40U) {
                card->cccrflags |= kSDIO_CCCRSupportContinuousSPIInt;
            }

            /* card capability register */
            card->cccrflags |= (tempBuffer[8U] & 0xDFU);

            /* master power control */
            if (tempBuffer[18U] & 0x01U) {
                card->cccrflags |= kSDIO_CCCRSupportMasterPowerControl;
            }

            /* high speed flag */
            if (tempBuffer[19U] & 0x01U) {
                card->cccrflags |= kSDIO_CCCRSupportHighSpeed;
            }

            /* high speed flag */
            if (tempBuffer[8U] & 0x80U) {
                card->cccrflags |= kSDIO_CCCRSupportLowSpeed4Bit;
            }

            /* common CIS pointer */
            card->commonCISPointer = tempBuffer[9U] | (tempBuffer[10U] << 8U) | (tempBuffer[11U] << 16U);

            break;

        case kSDIO_FunctionNum1:
        case kSDIO_FunctionNum2:
        case kSDIO_FunctionNum3:
        case kSDIO_FunctionNum4:
        case kSDIO_FunctionNum5:
        case kSDIO_FunctionNum6:
        case kSDIO_FunctionNum7:
            card->ioFBR[func - 1U].ioStdFunctionCode = tempBuffer[0U] & 0x0FU;
            card->ioFBR[func - 1U].ioExtFunctionCode = tempBuffer[1U];
            card->ioFBR[func - 1U].ioPointerToCIS = tempBuffer[9U] | (tempBuffer[10U] << 8U) | (tempBuffer[11U] << 16U);
            card->ioFBR[func - 1U].ioPointerToCSA =
                tempBuffer[12U] | (tempBuffer[13U] << 8U) | (tempBuffer[14U] << 16U);

            if (tempBuffer[2U] & 0x01U) {
                card->ioFBR[func - 1U].flags |= kSDIO_FBRSupportPowerSelection;
            }

            if (tempBuffer[0U] & 0x40U) {
                card->ioFBR[func - 1U].flags |= kSDIO_FBRSupportCSA;
            }

            break;

        default:
            break;
    }

    return kStatus_Success;
}

status_t SDIO_SetBlockSize(sdio_card_t *card, sdio_func_num_t func, uint32_t block_size)
{
    assert(card);
    assert(func <= kSDIO_FunctionNum7);
    assert(block_size <= SDIO_MAX_BLOCK_SIZE);

    uint8_t temp = 0U;

    /* check the block size for block mode
    * so you need read CIS for each function first,before you do read/write
    */
    if ((func == kSDIO_FunctionNum0) && (card->commonCIS.fn0MaxBlkSize != 0U) &&
        (block_size > card->commonCIS.fn0MaxBlkSize)) {
        return kStatus_SDMMC_SDIO_InvalidArgument;
    } else if ((func != kSDIO_FunctionNum0) && (card->funcCIS[func - 1U].ioMaxBlockSize != 0U) &&
               (block_size > card->funcCIS[func - 1U].ioMaxBlockSize)) {
        return kStatus_SDMMC_SDIO_InvalidArgument;
    }

    temp = block_size & 0xFFU;

    if (kStatus_Success !=
        SDIO_IO_Write_Direct(card, kSDIO_FunctionNum0, SDIO_FBR_BASE(func) + kSDIO_RegFN0BlockSizeLow, &temp, true)) {
        return kStatus_SDMMC_SetCardBlockSizeFailed;
    }

    temp = (block_size >> 8U) & 0xFFU;

    if (kStatus_Success !=
        SDIO_IO_Write_Direct(card, kSDIO_FunctionNum0, SDIO_FBR_BASE(func) + kSDIO_RegFN0BlockSizeHigh, &temp, true)) {
        return kStatus_SDMMC_SetCardBlockSizeFailed;
    }

    /* record the current block size */
    if (func == kSDIO_FunctionNum0) {
        card->io0block_size = block_size;
    } else {
        card->ioFBR[func - 1U].ioBlockSize = block_size;
    }

    return kStatus_Success;
}

status_t SDIO_CardReset(sdio_card_t *card)
{
    uint8_t reset = 0x08U;

    return SDIO_IO_Write_Direct(card, kSDIO_FunctionNum0, kSDIO_RegIOAbort, &reset, false);
}

status_t SDIO_SetDataBusWidth(sdio_card_t *card, sdio_bus_width_t busWidth)
{
    assert(card);

    uint8_t regBusInterface = 0U;

    if ((busWidth == kSDIO_DataBus4Bit) && ((card->cccrflags & kSDIO_CCCRSupportHighSpeed) == 0U) &&
        ((card->cccrflags & kSDIO_CCCRSupportLowSpeed4Bit) == 0U)) {
        return kStatus_SDMMC_SDIO_InvalidArgument;
    }

    /* load bus interface register */
    if (kStatus_Success != SDIO_IO_Read_Direct(card, kSDIO_FunctionNum0, kSDIO_RegBusInterface, &regBusInterface)) {
        return kStatus_SDMMC_TransferFailed;
    }

    /* set bus width */
    regBusInterface &= 0xFCU;
    regBusInterface |= busWidth;

    /* write to register */
    if (kStatus_Success !=
        SDIO_IO_Write_Direct(card, kSDIO_FunctionNum0, kSDIO_RegBusInterface, &regBusInterface, true)) {
        return kStatus_SDMMC_TransferFailed;
    }

    if (busWidth == kSDIO_DataBus4Bit) {
        SDMMCHOST_SET_CARD_BUS_WIDTH(card->host.base, kSDMMCHOST_DATABUSWIDTH4BIT);
    } else {
        SDMMCHOST_SET_CARD_BUS_WIDTH(card->host.base, kSDMMCHOST_DATABUSWIDTH1BIT);
    }

    return kStatus_Success;
}

status_t SDIO_SwitchToHighSpeed(sdio_card_t *card)
{
    assert(card);

    uint8_t temp = 0U;
    uint32_t retryTimes = SDIO_RETRY_TIMES;
    status_t status = kStatus_SDMMC_SDIO_SwitchHighSpeedFail;

    if (card->cccrflags & kSDIO_CCCRSupportHighSpeed) {
        do {
            retryTimes--;
            /* enable high speed mode */
            temp = 0x02U;

            if (kStatus_Success != SDIO_IO_Write_Direct(card, kSDIO_FunctionNum0, kSDIO_RegHighSpeed, &temp, true)) {
                continue;
            }

            /* either EHS=0 and SHS=0 ,the card is still in default mode  */
            if ((temp & 0x03U) == 0x03U) {
                /* high speed mode , set freq to 50MHZ */
                card->busClock_Hz =
                    SDMMCHOST_SET_CARD_CLOCK(card->host.base, card->host.source_clock_hz, SDHOST_CLOCK);
                status = kStatus_Success;
                break;
            } else {
                continue;
            }

        } while (retryTimes);
    } else {
        /* default mode 25MHZ */
        card->busClock_Hz = SDMMCHOST_SET_CARD_CLOCK(card->host.base, card->host.source_clock_hz, SD_CLOCK_25MHZ);
        status = kStatus_Success;
    }

    return status;
}

static status_t SDIO_DecodeCIS(
    sdio_card_t *card, sdio_func_num_t func, uint8_t *dataBuffer, uint32_t tplCode, uint32_t tplLink)
{
    assert(card);
    assert(func <= kSDIO_FunctionNum7);

    if (func == kSDIO_FunctionNum0) {
        /* only decode MANIFID,FUNCID,FUNCE here  */
        if (tplCode == SDIO_TPL_CODE_MANIFID) {
            card->commonCIS.mID = dataBuffer[0U] | (dataBuffer[1U] << 8U);
            card->commonCIS.mInfo = dataBuffer[2U] | (dataBuffer[3U] << 8U);
        } else if (tplCode == SDIO_TPL_CODE_FUNCID) {
            card->commonCIS.funcID = dataBuffer[0U];
        } else if (tplCode == SDIO_TPL_CODE_FUNCE) {
            /* max transfer block size and data size */
            card->commonCIS.fn0MaxBlkSize = dataBuffer[1U] | (dataBuffer[2U] << 8U);
            /* max transfer speed */
            card->commonCIS.maxTransSpeed = dataBuffer[3U];
        } else {
            /* reserved here */
            return kStatus_Fail;
        }
    } else {
        /* only decode FUNCID,FUNCE here  */
        if (tplCode == SDIO_TPL_CODE_FUNCID) {
            card->funcCIS[func].funcID = dataBuffer[0U];
        } else if (tplCode == SDIO_TPL_CODE_FUNCE) {
            if (tplLink == 0x2A) {
                card->funcCIS[func - 1U].funcInfo = dataBuffer[1U];
                card->funcCIS[func - 1U].ioVersion = dataBuffer[2U];
                card->funcCIS[func - 1U].cardPSN =
                    dataBuffer[3U] | (dataBuffer[4U] << 8U) | (dataBuffer[5U] << 16U) | (dataBuffer[6U] << 24U);
                card->funcCIS[func - 1U].ioCSASize =
                    dataBuffer[7U] | (dataBuffer[8U] << 8U) | (dataBuffer[9U] << 16U) | (dataBuffer[10U] << 24U);
                card->funcCIS[func - 1U].ioCSAProperty = dataBuffer[11U];
                card->funcCIS[func - 1U].ioMaxBlockSize = dataBuffer[12U] | (dataBuffer[13U] << 8U);
                card->funcCIS[func - 1U].ioOCR =
                    dataBuffer[14U] | (dataBuffer[15U] << 8U) | (dataBuffer[16U] << 16U) | (dataBuffer[17U] << 24U);
                card->funcCIS[func - 1U].ioOPMinPwr = dataBuffer[18U];
                card->funcCIS[func - 1U].ioOPAvgPwr = dataBuffer[19U];
                card->funcCIS[func - 1U].ioOPMaxPwr = dataBuffer[20U];
                card->funcCIS[func - 1U].ioSBMinPwr = dataBuffer[21U];
                card->funcCIS[func - 1U].ioSBAvgPwr = dataBuffer[22U];
                card->funcCIS[func - 1U].ioSBMaxPwr = dataBuffer[23U];
                card->funcCIS[func - 1U].ioMinBandWidth = dataBuffer[24U] | (dataBuffer[25U] << 8U);
                card->funcCIS[func - 1U].ioOptimumBandWidth = dataBuffer[26U] | (dataBuffer[27U] << 8U);
                card->funcCIS[func - 1U].ioReadyTimeout = dataBuffer[28U] | (dataBuffer[29U] << 8U);

                card->funcCIS[func - 1U].ioHighCurrentAvgCurrent = dataBuffer[34U] | (dataBuffer[35U] << 8U);
                card->funcCIS[func - 1U].ioHighCurrentMaxCurrent = dataBuffer[36U] | (dataBuffer[37U] << 8U);
                card->funcCIS[func - 1U].ioLowCurrentAvgCurrent = dataBuffer[38U] | (dataBuffer[39U] << 8U);
                card->funcCIS[func - 1U].ioLowCurrentMaxCurrent = dataBuffer[40U] | (dataBuffer[41U] << 8U);
            } else {
                return kStatus_Fail;
            }
        } else {
            return kStatus_Fail;
        }
    }

    return kStatus_Success;
}

status_t SDIO_ReadCIS(sdio_card_t *card, sdio_func_num_t func, const uint32_t *tupleList, uint32_t tupleNum)
{
    assert(card);
    assert(func <= kSDIO_FunctionNum7);
    assert(tupleList);

    uint8_t tplCode = 0U;
    uint8_t tplLink = 0U;
    uint32_t cisPtr = 0U;
    uint32_t i = 0U, num = 0U;
    bool tupleMatch = false;

    uint8_t dataBuffer[255U] = {0U};

    /* get the CIS pointer for each function */
    if (func == kSDIO_FunctionNum0) {
        cisPtr = card->commonCISPointer;
    } else {
        cisPtr = card->ioFBR[func - 1U].ioPointerToCIS;
    }

    if (0U == cisPtr) {
        return kStatus_SDMMC_SDIO_ReadCISFail;
    }

    do {
        if (kStatus_Success != SDIO_IO_Read_Direct(card, kSDIO_FunctionNum0, cisPtr++, &tplCode)) {
            return kStatus_SDMMC_TransferFailed;
        }

        /* end of chain tuple */
        if (tplCode == 0xFFU) {
            break;
        }

        if (tplCode == 0U) {
            continue;
        }

        for (i = 0; i < tupleNum; i++) {
            if (tplCode == tupleList[i]) {
                tupleMatch = true;
                break;
            }
        }

        if (kStatus_Success != SDIO_IO_Read_Direct(card, kSDIO_FunctionNum0, cisPtr++, &tplLink)) {
            return kStatus_SDMMC_TransferFailed;
        }

        /* end of chain tuple */
        if (tplLink == 0xFFU) {
            break;
        }

        if (tupleMatch) {
            memset(dataBuffer, 0U, 255U);

            for (i = 0; i < tplLink; i++) {
                if (kStatus_Success != SDIO_IO_Read_Direct(card, kSDIO_FunctionNum0, cisPtr++, &dataBuffer[i])) {
                    return kStatus_SDMMC_TransferFailed;
                }
            }

            tupleMatch = false;
            /* pharse the data */
            SDIO_DecodeCIS(card, func, dataBuffer, tplCode, tplLink);

            /* read finish then return */
            if (++num == tupleNum) {
                break;
            }
        } else {
            /* move pointer */
            cisPtr += tplLink;
            /* tuple code not match,continue read tuple code */
            continue;
        }
    } while (1);

    return kStatus_Success;
}

status_t SDIO_CardInit(sdio_card_t *card)
{
    assert(card);

    sdio_bus_width_t busWidth = kSDIO_DataBus1Bit;

    if (!card->isHostReady) {
        return kStatus_SDMMC_HostNotReady;
    }

    /* Identify mode ,set clock to 400KHZ. */
    card->busClock_Hz = SDMMCHOST_SET_CARD_CLOCK(card->host.base, card->host.source_clock_hz, SDMMC_CLOCK_400KHZ);
    SDMMCHOST_SET_CARD_BUS_WIDTH(card->host.base, kSDMMCHOST_DATABUSWIDTH1BIT);
    SDMMCHOST_SEND_CARD_ACTIVE(card->host.base, 100U);

    /* get host capability */
    GET_SDMMCHOST_CAPABILITY(card->host.base, &(card->host.capability));

    SDIO_CardReset(card);

    /* card go idle */
    if (kStatus_Success != SDIO_GoIdle(card)) {
        return kStatus_SDMMC_GoIdleFailed;
    }

    /* Get IO OCR-CMD5 with arg0 ,set new voltage if needed*/
    if (kStatus_Success != SDIO_SendOperationCondition(card, 0U)) {
        return kStatus_SDMMC_HandShakeOperationConditionFailed;
    }

    /* there is a memonly card */
    if ((card->ioTotalNumber == 0U) && (card->memPresentFlag)) {
        return kStatus_SDMMC_SDIO_InvalidCard;
    }

    /* verify the voltage and set the new voltage */
    if (card->host.capability.flags & kSDMMCHOST_SupportV330) {
        if (kStatus_Success != SDIO_SendOperationCondition(card, kSDIO_OcrVdd32_33Flag | kSDIO_OcrVdd33_34Flag)) {
            return kStatus_SDMMC_InvalidVoltage;
        }
    } else {
        return kStatus_SDMMC_InvalidVoltage;
    }

    /* send relative address ,cmd3*/
    if (kStatus_Success != SDIO_SendRca(card)) {
        return kStatus_SDMMC_SendRelativeAddressFailed;
    }

    /* select card cmd7 */
    if (kStatus_Success != SDIO_SelectCard(card, true)) {
        return kStatus_SDMMC_SelectCardFailed;
    }

    /* get card capability */
    if (kStatus_Success != SDIO_GetCardCapability(card, kSDIO_FunctionNum0)) {
        return kStatus_SDMMC_TransferFailed;
    }

    /* read common CIS here */
    if (SDIO_ReadCIS(card, kSDIO_FunctionNum0, g_tupleList, SDIO_COMMON_CIS_TUPLE_NUM)) {
        return kStatus_SDMMC_SDIO_ReadCISFail;
    }



    /* switch data bus width */
    if ((card->cccrflags & kSDIO_CCCRSupportHighSpeed) || (card->cccrflags & kSDIO_CCCRSupportLowSpeed4Bit)) {
        busWidth = kSDIO_DataBus4Bit;
    }

    if (kStatus_Success != SDIO_SetDataBusWidth(card, busWidth)) {
        return kStatus_SDMMC_SetDataBusWidthFailed;
    }

    /* try to switch high speed */
    if (kStatus_Success != SDIO_SwitchToHighSpeed(card)) {
        return kStatus_SDMMC_SDIO_SwitchHighSpeedFail;
    }

    return kStatus_Success;
}

void SDIO_CardDeinit(sdio_card_t *card)
{
    assert(card);

    SDIO_CardReset(card);
    SDIO_SelectCard(card, false);
}

status_t SDIO_HostInit(sdio_card_t *card)
{
    assert(card);

    if ((!card->isHostReady) && SDMMCHOST_Init(&(card->host), (void *)(card->usrParam.cd)) != kStatus_Success) {
        return kStatus_Fail;
    }

    /* set the host status flag, after the card re-plug in, don't need init host again */
    card->isHostReady = true;

    return kStatus_Success;
}

void SDIO_HostDeinit(sdio_card_t *card)
{
    assert(card);

    SDMMCHOST_Deinit(&(card->host));

    /* should re-init host */
    card->isHostReady = false;
}

void SDIO_HostReset(SDMMCHOST_CONFIG *host)
{
    SDMMCHOST_Reset(host->base);
}

status_t SDIO_WaitCardDetectStatus(SDMMCHOST_TYPE hostBase, const sdmmchost_detect_card_t *cd, bool waitCardStatus)
{
    return SDMMCHOST_WaitCardDetectStatus(hostBase, cd, waitCardStatus);
}

bool SDIO_IsCardPresent(sdio_card_t *card)
{
    return SDMMCHOST_IsCardPresent(card->host.base);
}

void SDIO_PowerOnCard(SDMMCHOST_TYPE base, const sdmmchost_pwr_card_t *pwr)
{
    SDMMCHOST_PowerOnCard(base, pwr);
}

void SDIO_PowerOffCard(SDMMCHOST_TYPE base, const sdmmchost_pwr_card_t *pwr)
{
    SDMMCHOST_PowerOffCard(base, pwr);
}

status_t SDIO_Init(sdio_card_t *card)
{
    assert(card);
    assert(card->host.base);

    if (!card->isHostReady) {
        if (SDIO_HostInit(card) != kStatus_Success) {
            return kStatus_SDMMC_HostNotReady;
        }
    } else {
        /* reset the host */
        SDIO_HostReset(&(card->host));
    }

#if 0
    /* power off card */
    SDIO_PowerOffCard(card->host.base, card->usrParam.pwr);

    /* card detect */
    if (SDIO_WaitCardDetectStatus(card->host.base, card->usrParam.cd, true) != kStatus_Success) {
        return kStatus_SDMMC_CardDetectFailed;
    }

    /* power on card */
    SDIO_PowerOnCard(card->host.base, card->usrParam.pwr);
#endif

    return SDIO_CardInit(card);
}

void SDIO_Deinit(sdio_card_t *card)
{
    assert(card);

    SDIO_CardDeinit(card);
    SDIO_HostDeinit(card);
}

status_t SDIO_EnableIOInterrupt(sdio_card_t *card, sdio_func_num_t func, bool enable)
{
    assert(card);
    assert(func <= kSDIO_FunctionNum7);

    uint8_t intEn = 0U;

    /* load io interrupt enable register */
    if (kStatus_Success != SDIO_IO_Read_Direct(card, kSDIO_FunctionNum0, kSDIO_RegIOIntEnable, &intEn)) {
        return kStatus_SDMMC_TransferFailed;
    }

    if (enable) {
        /* if already enable , do not need enable again */
        if ((((intEn >> func) & 0x01U) == 0x01U) && (intEn & 0x01U)) {
            return kStatus_Success;
        }

        /* enable the interrupt and interrupt master */
        intEn |= (1U << func) | 0x01U;
    } else {
        /* if already disable , do not need enable again */
        if (((intEn >> func) & 0x01U) == 0x00U) {
            return kStatus_Success;
        }

        /* disable the interrupt, don't disable the interrupt master here */
        intEn &= ~(1U << func);
    }

    /* write to register */
    if (kStatus_Success != SDIO_IO_Write_Direct(card, kSDIO_FunctionNum0, kSDIO_RegIOIntEnable, &intEn, true)) {
        return kStatus_SDMMC_TransferFailed;
    }

    return kStatus_Success;
}

status_t SDIO_EnableIO(sdio_card_t *card, sdio_func_num_t func, bool enable)
{
    assert(card);
    assert(func <= kSDIO_FunctionNum7);

    uint8_t ioEn = 0U, ioReady = 0U;
    uint32_t i = SDIO_RETRY_TIMES;

    /* load io enable register */
    if (kStatus_Success != SDIO_IO_Read_Direct(card, kSDIO_FunctionNum0, kSDIO_RegIOEnable, &ioEn)) {
        return kStatus_SDMMC_TransferFailed;
    }

    /* if already enable/disable , do not need enable/disable again */
    if (((ioEn >> func) & 0x01U) == (enable ? 1U : 0U)) {
        return kStatus_Success;
    }

    /* enable the io */
    if (enable) {
        ioEn |= (1U << func);
    } else {
        ioEn &= ~(1U << func);
    }

    /* write to register */
    if (kStatus_Success != SDIO_IO_Write_Direct(card, kSDIO_FunctionNum0, kSDIO_RegIOEnable, &ioEn, true)) {
        return kStatus_SDMMC_TransferFailed;
    }

    /* if enable io, need check the IO ready status */
    if (enable) {
        do {
            /* wait IO ready */
            if (kStatus_Success != SDIO_IO_Read_Direct(card, kSDIO_FunctionNum0, kSDIO_RegIOReady, &ioReady)) {
                return kStatus_SDMMC_TransferFailed;
            }

            /* check if IO ready */
            if ((ioReady & (1 << func)) != 0U) {
                return kStatus_Success;
            }

            i--;
        } while (i);
    }

    return kStatus_Fail;
}

status_t SDIO_SelectIO(sdio_card_t *card, sdio_func_num_t func)
{
    assert(card);
    assert(func <= kSDIO_FunctionMemory);

    uint8_t ioSel = func;

    /* write to register */
    if (kStatus_Success != SDIO_IO_Write_Direct(card, kSDIO_FunctionNum0, kSDIO_RegFunctionSelect, &ioSel, true)) {
        return kStatus_SDMMC_TransferFailed;
    }

    return kStatus_Success;
}

status_t SDIO_AbortIO(sdio_card_t *card, sdio_func_num_t func)
{
    assert(card);
    assert(func <= kSDIO_FunctionNum7);

    uint8_t ioAbort = func;

    /* write to register */
    if (kStatus_Success != SDIO_IO_Write_Direct(card, kSDIO_FunctionNum0, kSDIO_RegIOAbort, &ioAbort, true)) {
        return kStatus_SDMMC_TransferFailed;
    }

    return kStatus_Success;
}
