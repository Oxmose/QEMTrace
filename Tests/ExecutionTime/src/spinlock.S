#include "ppc-asm.h"

.globl lock_get

lock_get: 
    lwarx  r5, 0, r3
    cmpwi  r5, 0
    bc     4, 2, done
    stwcx. r4, 0, r3
    bc     4, 2, lock_get
done:
    mr     r3, r5
    blr