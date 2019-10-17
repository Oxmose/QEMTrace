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
/* Exact-width integer types */
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned           uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

typedef enum
{
    SERIAL,
    SCREEN
} OUTPUT_TYPE_E;

#endif /* __COMMON_H_ */
