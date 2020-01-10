/*******************************************************************************
 *
 * File: output.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 20/06/2018
 *
 * Version: 1.0
 *
 * Output code for the PowerPC e500 version of MiniKernel
 ******************************************************************************/

#include "common.h"       /* Generic int types, OUTPUT_TYPE_E */

volatile char* serial_out = (char*)(CCSBAR_BASE_ADDR + CCSBAR_UTHR_OFFSET);

void _putchar(char character)
{
    *serial_out = character;
}