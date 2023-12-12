/**
 *Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 * \brief Application interface for cvitek audio
 * \author cvitek
 * \date 2022
 */
#ifndef __CVITEK_PP_INTERFACE_H__
#define __CVITEK_PP_INTERFACE_H__
#include "cvi_audio_eq.h"
#include "cvi_audio_drc.h"
#include "cvi_audio_hpf.h"

#define FRAME_SIZE	(960)

typedef struct {
	int sample_rate;
	int channel;
	float input_gain;
	int flag;//bit 0, high pass, bit 1, eq, bit 2, drc
	void *cvi_hpf;
	void *cvi_eq;
	void *cvi_limiter;
	void *cvi_compress;
	void *cvi_expander;
	float *input;
	float *output;
} pp_handle, *pp_handle_t;

/**
 * post processing initial
 * param1, samplerate
 * param2, channel number
 * return
 * success return handle
 * fail return NULL
 */
void *cvi_audio_pp_init(int samplerate, int channels);

/**
 * post processing process
 * param1, handle
 * param2, input buffer pointer
 * param3, output buffer pointer
 * param4, frame size
 */
void cvi_audio_pp_process(void *handle, void *input, void *output, int frame_size);

/**
 * post processing uninitial
 * param1, handle
 */
void cvi_audio_pp_deinit(void *handle);

/**
 * eq set params
 * param1, handle
 * param2, eq params pointer
 */
int cvi_audio_pp_set_eq_params(void *handle, CVI_ST_EQ_PARAMS_PTR pstEqParams);

/**
 * eq get params
 * param1, handle
 * param2, eq params pointer
 */
int cvi_audio_pp_get_eq_params(void *handle, CVI_ST_EQ_PARAMS_PTR pstEqParams);

/**
 * drc set params
 * param1, handle
 * param2, drc params pointer
 * param3, drc type
 */
int cvi_audio_pp_set_drc_params(void *handle, void *pstDrcParams, int eDrcType);

/**
 * drc get params
 * param1, handle
 * param2, drc params pointer
 * param3, drc type
 */
int cvi_audio_pp_get_drc_params(void *handle, void *pstDrcParams, int eDrcType);


/**
 * hpf set params
 * param1, handle
 * param2, freq
 */
int cvi_audio_set_hpf_params(void *handle, unsigned int freq);

/**
 * hpf get params
 * param1, handle
 * return freq value
 */
int cvi_audio_get_hpf_freq(void *handle);

/**
 * set input gain
 * param1, handle
 * param2  gain value
 */
int cvi_audio_pp_set_inputgain(void *handle, float gain);

/**
 * set flag
 * param1, handle
 * param2  flag value
 */
int cvi_audio_pp_set_flag(void *handle, int flag);

#endif
