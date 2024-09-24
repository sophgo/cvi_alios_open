#ifndef __BASE_CB_H__
#define __BASE_CB_H__

#ifdef __cplusplus
	extern "C" {
#endif
#include "rtos_types.h"
#define IDTOSTR(X) (X < E_MODULE_BUTT) ? CB_MOD_STR[X] : "UNDEF" \

#define CB_FOREACH_MOD(MOD) {	\
		MOD(BASE)	\
		MOD(SYS)	\
		MOD(CIF)	\
		MOD(SNSR_I2C)	\
		MOD(VI)		\
		MOD(VPSS)	\
		MOD(VO)		\
		MOD(LDC)	\
		MOD(RGN)	\
		MOD(AVS)	\
		MOD(VCODEC)	\
		MOD(BUTT)	\
	}

#define CB_GENERATE_STRING(STRING) (#STRING),
#define CB_GENERATE_ENUM(ENUM) E_MODULE_##ENUM,

enum ENUM_MODULES_ID CB_FOREACH_MOD(CB_GENERATE_ENUM);
extern const char * const CB_MOD_STR[];

struct base_m_cb_info {
	enum ENUM_MODULES_ID module_id;
	void *dev;
	int (*cb)(void *dev, enum ENUM_MODULES_ID caller, u32 cmd, void *arg);
};

struct base_exe_m_cb {
	enum ENUM_MODULES_ID caller;
	enum ENUM_MODULES_ID callee;
	u32  cmd_id;
	void *data;
};

int base_rm_module_cb(enum ENUM_MODULES_ID module_id);
int base_reg_module_cb(struct base_m_cb_info *cb_info);
int base_exe_module_cb(struct base_exe_m_cb *exe_cb);

#ifdef __cplusplus
}
#endif

#endif /* __BASE_CB_H__ */
