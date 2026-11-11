#include "io.h"
#include "drv/common.h"
#include "cvi_rpmsg_internal.h"


#define CVI_MBOX_SPINLOCK_OFFSET   0xc0
#define CVI_MBOX_SPINLOCK_NUM      16

/* Well-known lock field indices (shared convention with the AliOS side). */
#define CVI_MBOX_SPIN_MBOX         1   /* protect mailbox register access */
#define CVI_MBOX_SPIN_SHM          2   /* protect shared memory access  */

/* Owner values: per-CPU, non-zero, distinct.  Linux CPU0 = 0x0001. */
#define CVI_MBOX_SPINLOCK_CPU2_VAL 0x0100u


/*
 * Acquire hardware spinlock @lock.  Writes the owner value and reads it back;
 * on match the lock is ours.  Retries with a 1us backoff until acquired or the
 * budget is exhausted.  Returns 0 on success, -ETIMEDOUT on timeout,
 * -EINVAL on a bad field/value.
 *
 * NOTE: this may be called from contexts that can spin briefly but not block
 * indefinitely; the retry budget bounds the worst case.  Pair every successful
 * lock with hw_spin_unlock().
 */
int hw_spin_lock(unsigned char lock_id)
{
    volatile void *reg_base = (void *)CVI_RPMSG_MBOX_REG_BASE;
    volatile void *reg;
    unsigned int tries;
    u16 v = CVI_MBOX_SPINLOCK_CPU2_VAL;

    if (lock_id >= CVI_MBOX_SPINLOCK_NUM)
        return -1;

    reg = reg_base + CVI_MBOX_SPINLOCK_OFFSET + lock_id * sizeof(u32);

    for (tries = 0; tries < 1000000; tries++) {
        writew(v, reg);
        if (readw(reg) == v)
            return 0;
        udelay(1);
    }

    printf("hw spinlock %u acquire timeout (val=0x%x)\n", lock_id, v);
    return -1;
}

/*
 * Release hardware spinlock @lock.  Only writes (which unlocks) when the
 * current lock value matches our owner value, i.e. we still own it.  Writing
 * the held value clears it to 0 (unlocked).  No-op if we don't hold it.
 */
void hw_spin_unlock(unsigned char lock_id)
{
    volatile void *reg_base = (void *)CVI_RPMSG_MBOX_REG_BASE;
    volatile void *reg;
    u16 v = CVI_MBOX_SPINLOCK_CPU2_VAL;

    if (lock_id >= CVI_MBOX_SPINLOCK_NUM)
        return;

    reg = reg_base + CVI_MBOX_SPINLOCK_OFFSET + lock_id * sizeof(u32);

    if (readw(reg) == v)
        writew(v, reg);
}
