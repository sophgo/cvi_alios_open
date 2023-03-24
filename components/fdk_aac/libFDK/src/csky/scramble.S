/*
 * void scramble(FIXP_DBL x[], INT n);
 */
#if ((defined(__ck804ef__) || defined(__ck805ef__)) && defined(FDK_CSKY_ASM))

    .section        .text.scramble_asm,"ax",@progbits
    .align          2
    .global         scramble_asm
    .type           scramble_asm, @function

scramble_asm:
    push            lr
    movi            a2, 1               // m = 1
    subi            a3, a1, 1           // n-1
    movi            t0, 0               // j = 0

.mloop:
    mov             t1, a1              // k = n

.kloop:
    lsri            t1, t1, 1           // k >>= 1
    xor             t0, t0, t1          // j ^= k
    and             t8, t0, t1          // t8 = t0 & t1
    bez             t8, .kloop          // while (t8 == 0)

    cmplt           a2, t0              // if (m < j)
    bf              .mloop_end

    lsli            t1, a2, 3           // m(t1) = a2*4*2
    add             t3, a0, t1
    pldbi.d         t8, (t3)            // t8 = x[t1], t9 = x[t1+1]
    lsli            t2, t0, 3           // j(t2) = t0*4*2
    add             t3, a0, t2
    pldbi.d         t6, (t3)            // t6 = x[t2], t7 = x[t2+1]
    add             t2, a0, t2
    stbi.w          t8, (t2)            // x[t2,t2+1] = t8,t9
    stbi.w          t9, (t2)            // x[t2,t2+1] = t8,t9
    add             t1, a0, t1
    stbi.w          t6, (t1)            // x[t1,t1+1] = t6,t7
    stbi.w          t7, (t1)            // x[t1,t1+1] = t6,t7

.mloop_end:
    addi            a2, a2, 1           // m++
    cmplt           a2, a3
    bt              .mloop

    pop             lr
    .size           scramble_asm, .-scramble_asm

#endif


