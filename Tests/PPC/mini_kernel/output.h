/*******************************************************************************
 *
 * File: output.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 20/06/2018
 *
 * Version: 1.0
 *
 * Output code for the PowerPC e500 version of MiniKernel
 ******************************************************************************/

#ifndef __OUTPUT_H_
#define __OUTPUT_H_

#include "common.h" /* Generic int types, OUTPUT_TYPE_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Output the character string given as parameter on the desired output.
 *
 * ­@param str The string to output.
 * @param type The type of device to output the string to.
 * @returns The function returns the number of character written.
 */
int32_t output_str(const char* str, const OUTPUT_TYPE_E type);

#endif /* __OUTPUT_H_ */
