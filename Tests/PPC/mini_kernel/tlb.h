/*******************************************************************************
 *
 * File: tlb.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 20/06/2018
 *
 * Version: 1.0
 *
 * TLB management for the PowerPC e500 version of MiniKernel
 ******************************************************************************/

#ifndef __TLB_H_
#define __TLB_H_

#include "common.h" /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void tlb_set_entry(const uint8_t tlb_entry, const uint8_t entry,
	               const uint8_t inv_protect, const uint8_t tid,
	               const uint8_t tspace, const uint8_t page_size,
				   const uint32_t epn, const uint8_t attributes,
	               const uint64_t rpn, const uint8_t perms);

#endif /* __TLB_H_ */
