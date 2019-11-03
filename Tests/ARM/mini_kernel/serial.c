/***************************************************************************//**
 * @file serial.c
 *
 * @see serial.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/05/2019
 *
 * @version 1.0
 *
 * @brief PL011 UART communication driver.
 *
 * @details Serial communication driver. Initializes the serial ports as in and
 * output. The serial can be used to output data or communicate with other
 * prepherals that support this communication method.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include "common.h" /* Generic types */

/* Header file */
#include "serial.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* None */

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void serial_write(const uint32_t port, const uint8_t data)
{
    if(port != COM1 && port != COM2 && port != COM3 && port != COM4)
    {
        return;
    }

    if(data == '\n')
    {
        serial_write(port, '\r');
        *(volatile uint32_t*)(port) = '\n';
    }
    else
    {
        *(volatile uint32_t*)(port) = data;
    }
}

void serial_put_string(const char* string, const uint32_t size)
{
    uint32_t i;
    for(i = 0; i < size; ++i)
    {
        serial_write(COM1, string[i]);
    }
}

void serial_put_char(const char character)
{
    serial_write(COM1, character);
}

void serial_clear_screen(void)
{
    uint8_t i;
    /* On 80x25 screen, just print 25 line feed. */
    for(i = 0; i < 25; ++i)
    {
        serial_write(COM1, '\n');
    }
}

