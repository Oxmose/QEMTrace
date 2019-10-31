@-------------------------------------------------------------------------------
@
@ File: int_handlers.S
@
@ Author: Alexy Torres Aurora Dugo
@
@ Date: 09/05/2019
@
@ Version: 1.0
@
@ Global handlers for the ARM V7 A15. 
@ The global handler should redirect or root interrupt to the correct handler.
@
@-------------------------------------------------------------------------------

.extern int_handler 

.global global_sw_int_handler

.text

.align 4
global_sw_int_handler:
    @ CPU State (from reg[12] reg[0])
    stmfd    sp!, {R0-R12} 

    add sp, sp, #-12
    mov r0, sp 
    
    @ CPU State (PC, LR, SP)
    mrs     r1, CPSR
    orr     r1, r1, #0x1F
    msr     CPSR, r1 @ Switch to system mode
    str     sp,[r0]
    str     lr,[r0,#2]
    str     pc,[r0,#3]
    mrs     r1, CPSR
    bic     r1, r1, #0xF
    orr     r1, r1, #0x3
    msr     CPSR, r1 @ Switch back to SVC mode
    @ CPU State addr
    mov r0, sp

    @ Stack state
    mrs  r1, SPSR 
    push {r1}
    mov  r2, sp

    @ Int ID
    ldr r1, [lr,#-4]
    bic r1, r1,#0xFF000000     @ extract comment field.

    bl int_handler
    
    @ Stack state
    pop {r0}

    @ CPU State (from reg[12] to reg[0])
    pop {r0}
    pop {r0}
    pop {r0}
    pop {r0}
    pop {r0}
    pop {r0}
    pop {r0}
    pop {r0}
    pop {r0}
    pop {r0}
    pop {r0}
    pop {r0}
    pop {r0}
    @ CPU State (pc, lr, sp)
    pop {r0}
    pop {r0}
    pop {r0}

    pop      {R0,R3}               @ Get SPSR from stack.
    msr      SPSR, R0              @ Restore SPSR.
    LDMFD    sp!, {R0-R3,R12,pc}^  @ Restore registers and return.

sp_temp: .word sp_temp
lr_temp: .word lr_temp
pc_temp: .word pc_temp