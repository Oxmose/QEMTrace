@-------------------------------------------------------------------------------
@
@ File: paging.S
@
@ Author: Alexy Torres Aurora Dugo
@
@ Date: 03/05/2019
@
@ Version: 1.0
@
@ Kernel paging functions
@-------------------------------------------------------------------------------

@-----------------------------------------
@ EXTERN FUNCTIONS
@-----------------------------------------

.globl init_paging
.globl enable_paging
.globl disable_paging

.extern create_pgtable

init_paging:
    push {r0, r1, lr}

    @ Set TTBCR
    mov r0, #0
    mcr p15, 0, r0, c2, c0, 2

    @ Set TTBR0
    mrc p15, 0, r0, c2, c0, 0 
    ldr r1, =page_table_lvl1
    and r0, r0, #0x4
    orr r0, r1, r0 
    add r0, r0, #0x52
    mcr p15, 0, r0, c2, c0, 0 

    bl invalidate_tlb
    bl set_domain
    bl set_access_control

    bl create_pgtable

    pop {r0, r1, lr}
    bx lr

invalidate_tlb:
    push {r0}
    mov r0, #0 
    mcr p15, #0, r0, c8, c7, #0 
    dsb
    pop {r0}
    bx lr    

set_domain:
    push {r0}
    ldr r0, =#0x55555555
    mcr  p15, #0, r0, c3, c0, #0
    dsb
    pop {r0}
    bx lr

set_access_control:
    push {r0, r1}
    mrc  p15, #0, r0, c1, c0, #0 
    mov r1, #3
    lsl r1, r1, #30
    add r1, r1, #3
    bic  r0, r0, r1
    mcr  p15, #0, r0, c1, c0, #0
    dsb
    pop {r0, r1}
    bx lr

enable_paging:
    push {r0, lr}

    bl invalidate_tlb

    @ Enable MMU
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #1
    mcr p15, 0, r0, c1, c0, 0 
    dsb 
    pop {r0, lr}
    bx lr

disable_paging:
    push {r0, lr}

    bl invalidate_tlb

    @ Disable MMU
    mrc p15, 0, r0, c1, c0, 0
    bic r0, #1
    mcr p15, 0, r0, c1, c0, 0 
    dsb
    pop {r0, lr}
    bx lr
