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

void tlb_set_entry(const unsigned char tlb_id, const unsigned char entry,
	               const unsigned char inv_protect, const unsigned char tid,
	               const unsigned char tspace, const unsigned char page_size,
				   const unsigned int epn, const unsigned char attributes,
	               const unsigned long long rpn, const unsigned char perms)
{
	/* Check applicability to our arch */
  	if ((mfspr(SPRN_MMUCFG) & MMUCFG_MAVN) == MMUCFG_MAVN_V1 &&  (page_size & 1))
  	{
    	return;
  	}

	/* Set the MAS register values */
  	unsigned int mas0 = FSL_BOOKE_MAS0(tlb_id, entry, 0);
  	unsigned int mas1 = FSL_BOOKE_MAS1(1, inv_protect, tid, tspace, page_size);
  	unsigned int mas2 = FSL_BOOKE_MAS2(epn, attributes);
  	unsigned int mas3 = FSL_BOOKE_MAS3(rpn, 0, perms);
  	unsigned int mas7 = FSL_BOOKE_MAS7(rpn);

	/* Set MAS register to update TLB */
  	mtspr(MAS0, mas0);
	mtspr(MAS1, mas1);
  	mtspr(MAS2, mas2);
  	mtspr(MAS3, mas3);
  	mtspr(MAS7, mas7);

	/* Update TLB */
  	asm volatile("isync;tlbwe;msync;isync");
}

void tlb_rem_entry(const unsigned char tlb_id, const unsigned char entry)
{
	/* Set the MAS register values */
  	unsigned int mas0 = FSL_BOOKE_MAS0(tlb_id, entry, 0);
  	unsigned int mas1 = FSL_BOOKE_MAS1(0, 0, 0, 0, 0);
  	unsigned int mas2 = FSL_BOOKE_MAS2(0, 0);
  	unsigned int mas3 = FSL_BOOKE_MAS3(0, 0, 0);
  	unsigned int mas7 = FSL_BOOKE_MAS7(0);

	/* Set MAS register to update TLB */
  	mtspr(MAS0, mas0);
	mtspr(MAS1, mas1);
  	mtspr(MAS2, mas2);
  	mtspr(MAS3, mas3);
  	mtspr(MAS7, mas7);

	/* Update TLB */
  	asm volatile("isync;tlbwe;msync;isync");
}
