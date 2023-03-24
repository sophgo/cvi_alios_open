#include "ck_fft.h"

static inline drv_fft_mode_sel_t fft_len_to_mode_sel(
    csky_mca_fft_len_t fft_len) {
    // csky_mca_fft_len_t and drv_fft_mode_sel_t have same enum values.
    return (drv_fft_mode_sel_t)fft_len;
}

void csky_mca_rfft_fxp32(
    csky_mca_fft_len_t fft_len,
    const fxp32_t *input,
    size_t input_size,
    fxp32_t *output) {
    DRV_FFT_ASSERT_FFT_MODE_SEL_VALID(fft_len_to_mode_sel(fft_len));
    assert(NULL != input && NULL != output);
    DRV_FFT_ASSERT_INPUT_LEN_IN_RANGE(input_size, fft_len_to_mode_sel(fft_len));

    drv_fft_regs->in_addr = (uint32_t)input;
    drv_fft_regs->out_addr = (uint32_t)output;
    drv_fft_regs->mode_sel =
        (fft_len_to_mode_sel(fft_len) << DRV_FFT_MODE_SEL_POS) |
        (DRV_FFT_FUNC_SEL_REAL_FFT << DRV_FFT_FUNC_SEL_POS);
    drv_fft_regs->in_num = input_size;

    drv_fft_regs->start = DRV_FFT_START_MASK;

    drv_fft_await();
}

void csky_mca_cfft_fxp32(
    csky_mca_fft_len_t fft_len,
    const fxp32_t *input,
    fxp32_t *output) {
    DRV_FFT_ASSERT_FFT_MODE_SEL_VALID(fft_len_to_mode_sel(fft_len));
    assert(NULL != input && NULL != output);

    drv_fft_regs->in_addr = (uint32_t)input;
    drv_fft_regs->out_addr = (uint32_t)output;
    drv_fft_regs->mode_sel =
        (fft_len_to_mode_sel(fft_len) << DRV_FFT_MODE_SEL_POS) |
        (DRV_FFT_FUNC_SEL_COMPLEX_FFT << DRV_FFT_FUNC_SEL_POS);

    drv_fft_regs->start = DRV_FFT_START_MASK;

    drv_fft_await();
}

void csky_mca_rifft_fxp32(
    csky_mca_fft_len_t fft_len,
    const fxp32_t *input,
    fxp32_t *output) {
    DRV_FFT_ASSERT_FFT_MODE_SEL_VALID(fft_len_to_mode_sel(fft_len));
    assert(NULL != input && NULL != output);

    drv_fft_regs->in_addr = (uint32_t)input;
    drv_fft_regs->out_addr = (uint32_t)output;
    drv_fft_regs->mode_sel =
        (fft_len_to_mode_sel(fft_len) << DRV_FFT_MODE_SEL_POS) |
        (DRV_FFT_FUNC_SEL_REAL_IFFT << DRV_FFT_FUNC_SEL_POS);

    drv_fft_regs->start = DRV_FFT_START_MASK;

    drv_fft_await();
}

void csky_mca_cifft_fxp32(
    csky_mca_fft_len_t fft_len,
    const fxp32_t *input,
    fxp32_t *output) {
    DRV_FFT_ASSERT_FFT_MODE_SEL_VALID(fft_len_to_mode_sel(fft_len));
    assert(NULL != input && NULL != output);

    drv_fft_regs->in_addr = (uint32_t)input;
    drv_fft_regs->out_addr = (uint32_t)output;
    drv_fft_regs->mode_sel =
        (fft_len_to_mode_sel(fft_len) << DRV_FFT_MODE_SEL_POS) |
        (DRV_FFT_FUNC_SEL_COMPLEX_IFFT << DRV_FFT_FUNC_SEL_POS);

    drv_fft_regs->start = DRV_FFT_START_MASK;

    drv_fft_await();
}

void csky_mca_power_spectrum_fxp32(
    csky_mca_fft_len_t fft_len,
    const fxp32_t *input,
    size_t input_size,
    fxp64_t *output) {
    DRV_FFT_ASSERT_FFT_MODE_SEL_VALID(fft_len_to_mode_sel(fft_len));
    assert(NULL != input && NULL != output);

    DRV_FFT_ASSERT_INPUT_LEN_IN_RANGE(input_size, fft_len_to_mode_sel(fft_len));

    drv_fft_regs->in_addr = (uint32_t)input;
    drv_fft_regs->out_addr = (uint32_t)output;
    drv_fft_regs->mode_sel =
        (fft_len_to_mode_sel(fft_len) << DRV_FFT_MODE_SEL_POS) |
        (DRV_FFT_FUNC_SEL_POWER_SPECTRUM << DRV_FFT_FUNC_SEL_POS);
    drv_fft_regs->in_num = input_size;

    drv_fft_regs->start = DRV_FFT_START_MASK;

    drv_fft_await();
}
