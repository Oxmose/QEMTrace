/*******************************************************************************
 * test_opcode_trace.S
 *
 *  Author  Alexy Torres Aurora Dugo
 *  Version 1.0
 *
 * MiniKernel test: Start tracing Qemu custom instruction
 * The Qemu emulator should manage this instruction and start/stop
 * tracing the system memory access.
 ******************************************************************************/

/* Compute address at 1M */
li r4, 0x1000
li r3, 0x8
slw r4, r4, r3

li r5, 0x1234

/* Disable tracing: 0xFFFFFFF1 */
.long 0xFFFFFFF1

stw r5, 0(r4)
stw r5, 0(r4)
stw r5, 0(r4)

/* Enable tracing: 0xFFFFFFF0 */
.long 0xFFFFFFF0

/* Compute address at 1M */
li r4, 0x1000
li r3, 0x8
slw r4, r4, r3

li r5, 0x1234

stw r5, 0(r4)
stw r5, 0(r4)
stw r5, 0(r4)
