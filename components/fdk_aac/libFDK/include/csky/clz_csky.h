#if ((defined(__ck804ef__) || defined(__ck805ef__)) && defined(FDK_CSKY_ASM))

#if !defined(CLZ_CSKY_H)
#define CLZ_CSKY_H

#if defined(__csky__)

#define FUNCTION_fixnormz_D
#define FUNCTION_fixnorm_D
#define FUNCTION_fixnormz_S
#define FUNCTION_fixnorm_S

#ifdef FUNCTION_fixnormz_D
inline INT fixnormz_D(LONG value) {
  INT x;
  asm(
      "ff1 %0, %1\n\t"
      : "=r"(x)
      : "r"(value)
  );
  return x;
}
#endif /* #ifdef FUNCTION_fixnormz_D */

#ifdef FUNCTION_fixnorm_D
inline INT fixnorm_D(LONG value) {
  if (!value) return 0;
  if (value < 0) value = ~value;
  return fixnormz_D(value) - 1;
}
#endif /* #ifdef FUNCTION_fixnorm_D */

#ifdef FUNCTION_fixnormz_S
inline INT fixnormz_S(SHORT value) {
  INT result;
  result = (LONG)(value << 16);
  if (result == 0)
    result = 16;
  else
    result = fixnormz_D(result);
  return result;
}
#endif /* #ifdef FUNCTION_fixnormz_S */

#ifdef FUNCTION_fixnorm_S
inline INT fixnorm_S(SHORT value) {
  LONG lvalue = (LONG)(value << 16);
  if (!lvalue) return 0;
  if (lvalue < 0) lvalue = ~lvalue;
  return fixnormz_D(lvalue) - 1;
}
#endif /* #ifdef FUNCTION_fixnorm_S */

#endif /* __csky__ */

#endif /* !defined(CLZ_CSKY_H) */

#endif
