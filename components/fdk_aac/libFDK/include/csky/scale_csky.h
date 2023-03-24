#if !defined(SCALE_CSKY_H)
#define SCALE_CSKY_H

#if defined(__csky__)

#if ((defined(__ck804ef__) || defined(__ck805ef__)) && defined(FDK_CSKY_ASM))

inline static INT shiftRightSat(INT src, int scale) {
  INT result;
  asm("clipi.s32 %0,%1,%2;\n"

      : "=&r"(result)
      : "r"(src >> scale), "M"(SAMPLE_BITS));

  return result;
}

#define SATURATE_INT_PCM_RIGHT_SHIFT(src, scale) shiftRightSat(src, scale)

inline static INT shiftLeftSat(INT src, int scale) {
  INT result;
  asm("clipi.s32 %0,%1,%2;\n"

      : "=&r"(result)
      : "r"(src << scale), "M"(SAMPLE_BITS));

  return result;
}

#define SATURATE_INT_PCM_LEFT_SHIFT(src, scale) shiftLeftSat(src, scale)

#endif

#endif /* compiler selection */

#define FUNCTION_scaleValueInPlace
#ifdef FUNCTION_scaleValueInPlace
inline void scaleValueInPlace(FIXP_DBL *value, /*!< Value */
                              INT scalefactor  /*!< Scalefactor */
) {
  INT newscale;
  if ((newscale = scalefactor) >= 0)
    *value <<= newscale;
  else
    *value >>= -newscale;
}
#endif /* #ifdef FUNCTION_scaleValueInPlace */

#define SATURATE_RIGHT_SHIFT(src, scale, dBits)                                \
  ((((LONG)(src) ^ ((LONG)(src) >> (DFRACT_BITS - 1))) >> (scale)) >           \
   (LONG)(((1U) << ((dBits)-1)) - 1))                                          \
      ? ((LONG)(src) >> (DFRACT_BITS - 1)) ^ (LONG)(((1U) << ((dBits)-1)) - 1) \
      : ((LONG)(src) >> (scale))

#define SATURATE_LEFT_SHIFT(src, scale, dBits)                                 \
  (((LONG)(src) ^ ((LONG)(src) >> (DFRACT_BITS - 1))) >                        \
   ((LONG)(((1U) << ((dBits)-1)) - 1) >> (scale)))                             \
      ? ((LONG)(src) >> (DFRACT_BITS - 1)) ^ (LONG)(((1U) << ((dBits)-1)) - 1) \
      : ((LONG)(src) << (scale))

#endif /* !defined(SCALE_CSKY_H) */
