#ifndef __CACHE_H_
#define __CACHE_H_

#define L1_CACHE_BYTES     64
#ifndef ALIGN
#define ALIGN(x, a)              (((x) + (a) - 1) & ~((a) - 1))
#endif

/*
 * dcache.ipa rs1 (invalidate)
 * | 31 - 25 | 24 - 20 | 19 - 15 | 14 - 12 | 11 - 7 | 6 - 0 |
 *   0000001    01010      rs1       000      00000  0001011
 *
 * dcache.cpa rs1 (clean)
 * | 31 - 25 | 24 - 20 | 19 - 15 | 14 - 12 | 11 - 7 | 6 - 0 |
 *   0000001    01001      rs1       000      00000  0001011
 *
 * dcache.cipa rs1 (clean then invalidate)
 * | 31 - 25 | 24 - 20 | 19 - 15 | 14 - 12 | 11 - 7 | 6 - 0 |
 *   0000001    01011      rs1       000      00000  0001011
 *
 * sync.s
 * | 31 - 25 | 24 - 20 | 19 - 15 | 14 - 12 | 11 - 7 | 6 - 0 |
 *   0000000    11001     00000      000      00000  0001011
 */
#define DCACHE_IPA_A0	".long 0x02a5000b"
#define DCACHE_CPA_A0	".long 0x0295000b"
#define DCACHE_CIPA_A0	".long 0x02b5000b"

#define SYNC_S		".long 0x0190000b"

#define CACHE_OP_RANGE(OP, start, size) \
	register unsigned long i asm("a0") = (start) & ~(L1_CACHE_BYTES - 1); \
	for (; i < ALIGN((start) + (size), L1_CACHE_BYTES); i += L1_CACHE_BYTES) \
		__asm__ __volatile__(OP); \
	 __asm__ __volatile__(SYNC_S)

#define inv_dcache_range(start, size) CACHE_OP_RANGE(DCACHE_IPA_A0, start, size)

#define clean_dcache_range(start, size) CACHE_OP_RANGE(DCACHE_CPA_A0, start, size)

#define flush_dcache_range(start, size) CACHE_OP_RANGE(DCACHE_CIPA_A0, start, size)


#endif
