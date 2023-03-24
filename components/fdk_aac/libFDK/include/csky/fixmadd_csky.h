#if !defined(FIXMADD_CSKY_H)
#define FIXMADD_CSKY_H

#if defined(__csky__)

#if ((defined(__ck804ef__) || defined(__ck805ef__)) && defined(FDK_CSKY_ASM))

#define FUNCTION_fixmadddiv2_DD
#ifdef FUNCTION_fixmadddiv2_DD
inline FIXP_DBL fixmadddiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  asm(
      "mula.s32.hs %0, %1, %2\n\t"
      : "+r"(x)
      : "%r"(a), "r"(b)
  );
  return x;
}
#endif /* #ifdef FUNCTION_fixmadddiv2_DD */

#define FUNCTION_fixmsubdiv2_DD
#ifdef FUNCTION_fixmsubdiv2_DD
inline FIXP_DBL fixmsubdiv2_DD(FIXP_DBL x, const FIXP_DBL a, const FIXP_DBL b) {
  asm(
      "muls.s32.hs %0, %1, %2\n\t"
      : "+r"(x)
      : "%r"(a), "r"(b)
  );
  return x;
}
#endif /* #ifdef FUNCTION_fixmsubdiv2_DD */


#define FUNCTION_fixmadddiv2_DS
#ifdef FUNCTION_fixmadddiv2_DS
inline FIXP_DBL fixmadddiv2_DS(FIXP_DBL x, const FIXP_DBL a, const FIXP_SGL b) {
  asm(
      "mulaxl.s32.s %0, %1, %2\n\t"
      : "+r"(x)
      : "%r"(a), "r"(b)
  );
  return x;
}
#endif /* #ifdef FUNCTION_fixmadddiv2_DS */

#define FUNCTION_fixmadddiv2BitExact_DD
#ifdef FUNCTION_fixmadddiv2BitExact_DD
#define fixmadddiv2BitExact_DD(a, b, c) fixmadddiv2_DD(a, b, c)
#endif /* #ifdef FUNCTION_fixmadddiv2BitExact_DD */

#define FUNCTION_fixmsubdiv2BitExact_DD
#ifdef FUNCTION_fixmsubdiv2BitExact_DD
#define fixmsubdiv2BitExact_DD(a, b, c) fixmsubdiv2_DD(a, b, c)
#endif /* #ifdef FUNCTION_fixmsubdiv2BitExact_DD */

#define FUNCTION_fixmadddiv2BitExact_DS
#ifdef FUNCTION_fixmadddiv2BitExact_DS
#define fixmadddiv2BitExact_DS(a, b, c) fixmadddiv2_DS(a, b, c)
#endif /* #ifdef FUNCTION_fixmadddiv2BitExact_DS */

//#define FUNCTION_fixmsubdiv2BitExact_DS
//#ifdef FUNCTION_fixmsubdiv2BitExact_DS
//inline FIXP_DBL fixmsubdiv2BitExact_DS(FIXP_DBL x, const FIXP_DBL a,
//                                       const FIXP_SGL b) {
//  return x - fixmuldiv2BitExact_DS(a, b);
//}
//#endif /* #ifdef FUNCTION_fixmsubdiv2BitExact_DS */

#endif

#endif /* __csky__ */

#endif /* !defined(FIXMADD_CSKY_H) */
