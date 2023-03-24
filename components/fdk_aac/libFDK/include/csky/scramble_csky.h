#if !defined(SCRAMBLE_CSKY_H)
#define SCRAMBLE_CSKY_H

#define FUNCTION_scramble
inline void scramble(FIXP_DBL x[], INT n) {
  FDK_ASSERT(!(((INT)x) & (ALIGNMENT_DEFAULT - 1)));
  asm("mov     r2, #1;\n"     /* r2(m) = 1;           */
      "sub     r3, %1, #1;\n" /* r3 = n-1;            */
      "mov     r4, #0;\n"     /* r4(j) = 0;           */

      "scramble_m_loop%=:\n" /* {                    */
      "mov     r5, %1;\n"    /*  r5(k) = 1;          */

      "scramble_k_loop%=:\n"         /*  {                   */
      "lsri    r5, r5, #1;\n"        /*   k >>= 1;           */
      "xor     r4, r4, r5;\n"        /*   j ^=k;             */
      "and     r10, r4, r5;\n"       /*   r10 = r4 & r5;      */
      "bez     r10, scramble_k_loop%=;\n" /*  } while (r10 == 0);  */

      "cmplt   r2, r4;\n" /*   if (r4 < r2) break;        */
      "bf      scramble_m_loop_end%=;\n"

      "lsli    r5, r2, #3;\n" /* m(r5) = r2*4*2               */
      "add     r7, %0, r5;\n"
      "pldbi.d r10, (r7);\n"      /* r10 = x[r5], r11 = x[r5+1]     */
      "lsli    r6, r4, #3;\n"     /* j(r6) = r4*4*2              */
      "add     r7, %0, r6;\n"
      "pldbi.d r8, (r7);\n"       /* r8 = x[r6], r9 = x[r6+1];  */
      "add     r6, %0, r6;\n"
      "stbi.w  r10, (r6);\n"      /* x[r6,r6+1] = r10,r11;        */
      "stbi.w  r11, (r6);\n"      /* x[r6,r6+1] = r10,r11;        */
      "add     r5, %0, r5;\n"
      "stbi.w  r8, (r5);\n"       /* x[r5,r5+1] = r8,r9;          */
      "stbi.w  r9, (r5);\n"       /* x[r5,r5+1] = r8,r9;          */

      "scramble_m_loop_end%=:\n"
      "add     r2, r2, #1;\n" /* r2++;                        */
      "cmplt   r2, r3;\n"
      "bt      scramble_m_loop%=;\n" /* } while (r2(m) < r3(n-1));   */
      :
      : "r"(x), "r"(n)
      : "r2", "r3", "r4", "r5", "r10", "r11", "r8", "r9", "r6", "r7");
}


#endif /* !defined(SCRAMBLE_CSKY_H) */
