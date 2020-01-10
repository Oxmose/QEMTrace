extern unsigned int lock_get(unsigned int addr, unsigned int value);


void spinlock_lock(unsigned int addr)
{
    while(lock_get(addr, 1) != 0);
}

void spinlock_unlock(unsigned int addr)
{
    *((unsigned int*)addr) = 0;
}