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

void tlb_set_entry(const unsigned char tlb_entry, const unsigned char entry,
	               const unsigned char inv_protect, const unsigned char tid,
	               const unsigned char tspace, const unsigned char page_size,
				   const unsigned int epn, const unsigned char attributes,
	               const unsigned long long rpn, const unsigned char perms);

void tlb_rem_entry(const unsigned char tlb_id, const unsigned char entry);

#endif /* __TLB_H_ */
