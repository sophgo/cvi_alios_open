#ifndef __CVI_COMM_HW_I80_H__
#define __CVI_COMM_HW_I80_H__

#include "stdbool.h"
#ifdef CONFIG_KERNEL_RHINO
#include "rtos_types.h"
#else
#include <linux/types.h>
#endif

#include "cvi_common.h"
#include "cvi_comm_vo.h"

enum HW_I80_MODE {
	VO_MCU_MODE_RGB565 = 0,
	VO_MCU_MODE_RGB888,
	VO_MCU_MODE_MAX,
};

enum VO_TOP_MCU_MUX {
	VO_MUX_MCU_CS = 0,
	VO_MUX_MCU_RS,
	VO_MUX_MCU_WR,
	VO_MUX_MCU_RD,
	VO_MUX_MCU_DATA0,
	VO_MUX_MCU_DATA1,
	VO_MUX_MCU_DATA2,
	VO_MUX_MCU_DATA3,
	VO_MUX_MCU_DATA4,
	VO_MUX_MCU_DATA5,
	VO_MUX_MCU_DATA6,
	VO_MUX_MCU_DATA7,
	VO_MCU_MUX_MAX,
};

typedef struct _HW_I80_INSTR_S {
	CVI_U8	delay;
	CVI_U8  data_type;
	CVI_U8	data;
} HW_I80_INSTR_S;

struct HW_I80_INSTRS {
	unsigned char instr_num;
	HW_I80_INSTR_S instr_cmd[MAX_MCU_INSTR];
};

typedef struct _HW_I80_CFG_S {
	enum HW_I80_MODE mode;
	struct VO_PINMUX pins;
	struct HW_I80_INSTRS instrs;
	struct sync_info_s sync_info;
	CVI_U16 u16FrameRate;
} HW_I80_CFG_S;

#endif // __CVI_COMM_MIPI_TX_H__
