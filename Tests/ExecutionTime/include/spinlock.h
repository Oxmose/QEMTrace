#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

void spinlock_lock(unsigned int addr);
void spinlock_unlock(unsigned int addr);

#endif 