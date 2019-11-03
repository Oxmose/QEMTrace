@-------------------------------------------------------------------------------
@
@ File: loader.S
@
@ Author: Alexy Torres Aurora Dugo
@
@ Date: 03/05/2019
@
@ Version: 1.0
@
@ Kernel entry point and cpu initialization
@-------------------------------------------------------------------------------

@-----------------------------------------
@ EXTERN FUNCTIONS
@-----------------------------------------
.extern _edata
.extern _end
.extern global_sw_int_handler
.extern init_paging

.global loader @ Kernel entry point

@-----------------------------
@ PAGING SETTINGS
@-----------------------------

@-----------------------------
@ BOOTSTRAP CODE
@-----------------------------
.text
@ Load the kernel
loader:
    @ Deactivate all AP cpu 
    mrc p15, 0, r1, c0, c0, 5
    mov r0, #0x0F03
    and r1, r1, r0 
    mov r0, #0
    cmp r1, r0 
    bne loop

@ Blank all uninitialized memory
blank_mem: 
    mov r0, #0
    ldr r1, =(_edata)
    ldr r2, =(_end)

blank_mem_loop:        
    str r0, [r1]
    add r1, r1, #1
    cmp  r1, r2
    blt  blank_mem_loop

@ Kernel initialization
init:
    @ Init system's values and register
    bl init_regs

    @ Init stacks
    bl init_stacks

    @ Setup interrupt vector
    bl int_vector_init

    @ Initialize paging
    bl init_paging

    @ Kickstart the kernel 
    bl main

loop:
    b loop

init_regs:
    @ Read Configuration Base Address Register
    mrc p15, 4, r1, c15, c0, 0
    @ Store in memory
    ldr r0, =(periph_base_address)
    str r1, [r0]
    

    @ Init shared registers
    mov r0,  lr
    mov r1,  #0x0
    mov r2,  #0x0
    mov r3,  #0x0
    mov r4,  #0x0
    mov r5,  #0x0
    mov r6,  #0x0
    mov r7,  #0x0
    mov r8,  #0x0
    mov r9,  #0x0
    mov r10, #0x0
    mov r11, #0x0
    mov r12, #0x0

    @ Init SVC specific registers

    mrs r1, cpsr
    msr spsr_cxsf, r1

    @ Init FIQ specific registers
    cps #17
    mov r8,  #0x0
    mov r9,  #0x0
    mov r10, #0x0
    mov r11, #0x0
    mov r12, #0x0

    mrs r1,  cpsr
    msr spsr_cxsf, r1

    @ Init IRQ specific registers
    cps #18

    mrs r1,cpsr
    msr spsr_cxsf, r1

    @ Init Abort specific registers
    cps #23

    mrs r1,cpsr
    msr spsr_cxsf, r1

    @ Init Undefined specific registers
    cps #27

    mrs r1,cpsr
    msr spsr_cxsf, r1

    @ Init System/User specific registers
    cps #31

    mrs r1,cpsr
    msr spsr_cxsf, r1

    @ Switch back to SCV
    cps #19

    bx  lr

init_stacks:
    @ Set usr stack
    cps #31
    ldr sp, =usr_stack_end
    @ Set fiq stack
    cps #17
    ldr sp, =fiq_stack_end
    @ Set irq stack
    cps #18
    ldr sp, =irq_stack_end
    @ Set svc stack
    cps #19
    ldr sp, =svc_stack_end
    @ Set abt stack
    cps #23
    ldr sp, =abt_stack_end
    @ Set undef stack
    cps #27
    ldr sp, =undef_stack_end
    @ Switch back to SCV
    cps #19

    bx  lr

int_vector_init:
    ldr r0, =(int_vector)
    
    mcr p15, 0, r0, c12, c0, 0
    mcr p15, 0, r0, c12, c0, 1
    mcr p15, 4, r0, c12, c0, 0
    bx lr

@-----------------------------
@ Interrupt vector
@-----------------------------
.align 4
int_vector:
    ldr pc, =global_sw_int_handler
    ldr pc, =global_sw_int_handler
    ldr pc, =global_sw_int_handler
    ldr pc, =global_sw_int_handler
    ldr pc, =global_sw_int_handler
    ldr pc, =global_sw_int_handler

int_vector_end:
