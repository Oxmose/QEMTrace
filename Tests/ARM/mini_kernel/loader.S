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
.extern kernel_stack_end
.extern kernel_kickstart
.extern _edata
.extern _end
.extern global_sw_int_handler

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

    @ Set kernel stack
    ldr sp, =kernel_stack_end

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

    @ Clear screen
    bl serial_clear_screen

    @ Init serial for debug
    bl serial_clear_screen

    @ Setup interrupt vector
    bl int_vector_init

    @ Setup the kernel heap
    @bl setup_kheap

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
