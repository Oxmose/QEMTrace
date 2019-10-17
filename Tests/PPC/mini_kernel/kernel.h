/*******************************************************************************
 *
 * File: kernel.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 20/06/2018
 *
 * Version: 1.0
 *
 * Boot code for the PowerPC e500 version of MiniKernel
 ******************************************************************************/

#ifndef __BOOT_H_
#define __BOOT_H_

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Kernel kickstart function. The BSP is initialized then the main function is
 * called. This main function points to the test area where all the user tests
 * will then execute.
 * Once the main returned, the function just loops.
 * This function never returns.
 */
void kernel_kickstart(void);

#endif /* __BOOT_H_ */
