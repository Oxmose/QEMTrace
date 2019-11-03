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
    push {r0, lr}
    bl int_handler
    pop {r0, pc}

sp_temp: .word sp_temp
lr_temp: .word lr_temp
pc_temp: .word pc_temp
