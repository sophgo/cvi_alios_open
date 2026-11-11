#ifndef __CVI_HW_SPINLOCK_H__
#define __CVI_HW_SPINLOCK_H__

#define HW_SPIN_LOCK_NUM 16

int hw_spin_lock(unsigned char lock_id);
void hw_spin_unlock(unsigned char lock_id);

#endif