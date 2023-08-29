#ifndef __CVI_TEMPSEN_H
#define __CVI_TEMPSEN_H

#include <drv/tempsen.h>
#include <soc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef csi_tempsen_t cvi_tempsen_t;

cvi_error_t  cvi_tempsen_init(cvi_tempsen_t *tps);
cvi_error_t  cvi_tempsen_uninit(cvi_tempsen_t *tps);
int	     cvi_tempsen_read_temp(cvi_tempsen_t *tps, unsigned int timeout_ms);
int	     cvi_tempsen_read_max_temp(cvi_tempsen_t *tps);
unsigned int cvi_tempsen_test_force_val(cvi_tempsen_t *tps,
					unsigned int   force_val);

#ifdef __cplusplus
}
#endif

#endif /* __CVI_TEMPSEN_H */
