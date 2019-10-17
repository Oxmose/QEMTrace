/*******************************************************************************
 *
 * File: tlb.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 20/06/2018
 *
 * Version: 1.0
 *
 * TLB management for the PowerPC e500 version of MiniKernel
 ******************************************************************************/

#include "processor.h" /* mtspr */
#include "mmu.h"       /* FSL_BOOKE_MASx */

/* Header file */
#include "tlb.h"

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void tlb_set_entry(const uint8_t tlb_id, const uint8_t entry,
	               const uint8_t inv_protect, const uint8_t tid,
	               const uint8_t tspace, const uint8_t page_size,
				   const uint32_t epn, const uint8_t attributes,
	               const uint64_t rpn, const uint8_t perms)
{
	/* Check applicability to our arch */
  	if ((mfspr(SPRN_MMUCFG) & MMUCFG_MAVN) == MMUCFG_MAVN_V1 &&  (page_size & 1))
  	{
    	return;
  	}

	/* Set the MAS register values */
  	uint32_t mas0 = FSL_BOOKE_MAS0(tlb_id, entry, 0);
  	uint32_t mas1 = FSL_BOOKE_MAS1(1, inv_protect, tid, tspace, page_size);
  	uint32_t mas2 = FSL_BOOKE_MAS2(epn, attributes);
  	uint32_t mas3 = FSL_BOOKE_MAS3(rpn, 0, perms);
  	uint32_t mas7 = FSL_BOOKE_MAS7(rpn);

	/* Set MAS register to update TLB */
  	mtspr(MAS0, mas0);
	mtspr(MAS1, mas1);
  	mtspr(MAS2, mas2);
  	mtspr(MAS3, mas3);
  	mtspr(MAS7, mas7);

	/* Update TLB */
  	asm volatile("isync;tlbwe;msync;isync");
}
