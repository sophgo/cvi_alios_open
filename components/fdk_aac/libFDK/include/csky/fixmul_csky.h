#if !defined(FIXMUL_CSKY_H)
#define FIXMUL_CSKY_H

#if defined(__csky__)

#if ((defined(__ck804ef__) || defined(__ck805ef__)) && defined(FDK_CSKY_ASM))

#define FUNCTION_fixmuldiv2_DD

#define FUNCTION_fixmuldiv2BitExact_DD
#ifdef FUNCTION_fixmuldiv2BitExact_DD
#define fixmuldiv2BitExact_DD(a, b) fixmuldiv2_DD(a, b)
#endif /* #ifdef FUNCTION_fixmuldiv2BitExact_DD */

#define FUNCTION_fixmulBitExact_DD
#ifdef FUNCTION_fixmulBitExact_DD
#define fixmulBitExact_DD(a, b) (fixmuldiv2BitExact_DD(a, b) << 1)
#endif /* #ifdef FUNCTION_fixmulBitExact_DD */

#define FUNCTION_fixmuldiv2BitExact_DS
#ifdef FUNCTION_fixmuldiv2BitExact_DS
#define fixmuldiv2BitExact_DS(a, b) fixmuldiv2_DS(a, b)
#endif /* #ifdef FUNCTION_fixmuldiv2BitExact_DS */

#define FUNCTION_fixmulBitExact_DS
#ifdef FUNCTION_fixmulBitExact_DS
#define fixmulBitExact_DS(a, b) fixmul_DS(a, b)
#endif /* #ifdef FUNCTION_fixmulBitExact_DS */

#ifdef FUNCTION_fixmuldiv2_DD
inline INT fixmuldiv2_DD(const INT a, const INT b) {
  INT x;
  asm(
      "mul.s32.h %0, %1, %2\n\t"
      : "=r"(x)
      : "%r"(a), "r"(b)
  );
  return x;
}
#endif /* #ifdef FUNCTION_fixmuldiv2_DD */

#define FUNCTION_fixmuldiv2_SD
#ifdef FUNCTION_fixmuldiv2_SD
inline INT fixmuldiv2_SD(const SHORT a, const INT b) {
  return fixmuldiv2_DD((INT)(a << 16), b);
}
#endif /* #ifdef FUNCTION_fixmuldiv2_SD */

#define FUNCTION_fixmul_DD
#ifdef FUNCTION_fixmul_DD
inline INT fixmul_DD(const INT a, const INT b) {
  return (fixmuldiv2_DD(a, b) << 1);
}
#endif /* #ifdef FUNCTION_fixmul_DD */

#endif

#endif /* __csky__ */

#endif /* !defined(FIXMUL_CSKY_H) */
