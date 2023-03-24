#if ((defined(__ck804ef__) || defined(__ck805ef__)) && defined(FDK_CSKY_ASM))

#if !defined(CPLX_MUL_CSKY_H)
#define CPLX_MUL_CSKY_H

#if defined(__csky__)

#define FUNCTION_cplxMultDiv2_32x16

#define FUNCTION_cplxMultDiv2_32x32X2
#ifdef FUNCTION_cplxMultDiv2_32x32X2
inline void cplxMultDiv2(FIXP_DBL *c_Re, FIXP_DBL *c_Im, const FIXP_DBL a_Re,
                         const FIXP_DBL a_Im, const FIXP_DBL b_Re,
                         const FIXP_DBL b_Im) {
  LONG tmp1, tmp2;

  asm("mul.s32.h %0, %2, %4;\n"     /* tmp1  = a_Re * b_Re */
      "muls.s32.hs %0, %3, %5;\n"   /* tmp1 -= a_Im * b_Im */
      "mul.s32.h %1, %2, %5;\n"     /* tmp2  = a_Re * b_Im */
      "mula.s32.hs %1, %3, %4;\n"   /* tmp2 += a_Im * b_Re */
      : "=&r"(tmp1), "=&r"(tmp2)
      : "r"(a_Re), "r"(a_Im), "r"(b_Re), "r"(b_Im));
  *c_Re = tmp1;
  *c_Im = tmp2;
}
#endif /* FUNCTION_cplxMultDiv2_32x32X2 */

#if defined(FUNCTION_cplxMultDiv2_32x16)
inline void cplxMultDiv2(FIXP_DBL *c_Re, FIXP_DBL *c_Im, const FIXP_DBL a_Re,
                         const FIXP_DBL a_Im, FIXP_SPK wpk) {
  FIXP_DBL b_Im = FX_SGL2FX_DBL(wpk.v.im);
  FIXP_DBL b_Re = FX_SGL2FX_DBL(wpk.v.re);
  cplxMultDiv2(c_Re, c_Im, a_Re, a_Im, b_Re, b_Im);
}
#endif /* FUNCTION_cplxMultDiv2_32x16 */

#ifdef FUNCTION_cplxMultDiv2_32x16X2
inline void cplxMultDiv2(FIXP_DBL *c_Re, FIXP_DBL *c_Im, const FIXP_DBL a_Re,
                         const FIXP_DBL a_Im, const FIXP_SGL b_Re,
                         const FIXP_SGL b_Im) {
  FIXP_DBL b_re = FX_SGL2FX_DBL(b_Re);
  FIXP_DBL b_im = FX_SGL2FX_DBL(b_Im);
  cplxMultDiv2(c_Re, c_Im, a_Re, a_Im, b_re, b_im);
}
#endif /* FUNCTION_cplxMultDiv2_32x16X2 */

#endif

#endif /* !defined(CPLX_MUL_CSKY_H) */

#endif
