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
#include "qemu-ppce500.h" /* CONFIG_SYS_NS16550_COM1 */

/* Header file */
#include "output.h"

/* This is the serial output memory mapped IO, kind of like 0xB8000 on x86 for
 * VGA text ouput.
 */
static uint8_t* serial_out = (uint8_t*)CONFIG_SYS_NS16550_COM1;

int32_t output_str(const char* str, const OUTPUT_TYPE_E type)
{
    if(type != SERIAL)
    {
        return -1;
    }

    int32_t i = 0;

    /* Print string */
    while(*str != 0)
    {
       *serial_out = (uint8_t)*str++;
    }

    return i;
}
