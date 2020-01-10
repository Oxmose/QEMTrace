/*******************************************************************************
 *
 * File: common.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 20/06/2018
 *
 * Version: 1.0
 *
 * Common definitions for the PowerPC e500 version of MiniKernel
 ******************************************************************************/

#ifndef __COMMON_H_
#define __COMMON_H_

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define SPR_Exxx_L1CSR0       (0x3F2)
#define SPR_Exxx_L1CSR1       (0x3F3)

#define CCSBAR_BASE_PHYS_ADDR   0xFE0000000ULL
#define CCSBAR_BASE_ADDR        0xe0000000

#define CCSBAR_UDLB_OFFSET      0x4500
#define CCSBAR_UTHR_OFFSET      0x4500
#define CCSBAR_UDMB_OFFSET      0x4501
#define CCSBAR_UFCR_OFFSET      0x4502
#define CCSBAR_UAFR_OFFSET      0x4502
#define CCSBAR_ULCR_OFFSET      0x4503
#define CCSBAR_UMCR_OFFSET      0x4504
#define CCSBAR_ULSR_OFFSET      0x4505

#endif /* __COMMON_H_ */
