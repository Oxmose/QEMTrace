/***************************************************************************//**
 * @file serial.h
 *
 * @see serial.c
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

#ifndef __SERIAL_H_
#define __SERIAL_H_

#include "common.h" /* Generic types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief UART0 base address. */
#define UART0_BASE 0x1C090000
/** @brief UART0 base address. */
#define UART1_BASE 0x1C0A0000
/** @brief UART0 base address. */
#define UART2_BASE 0x1C0B0000
/** @brief UART0 base address. */
#define UART3_BASE 0x1C0C0000

/** @brief Redefinition of serial COM1 base port ID for ease of use. */
#define COM1 UART0_BASE
/** @brief Redefinition of serial COM2 base port ID for ease of use. */
#define COM2 UART1_BASE
/** @brief Redefinition of serial COM3 base port ID for ease of use. */
#define COM3 UART2_BASE
/** @brief Redefinition of serial COM4 base port ID for ease of use. */
#define COM4 UART3_BASE


/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/


/**
 * @brief Writes the data given as patameter on the desired port.
 * 
 * @details The function will output the data given as parameter on the selected
 * port. This call is blocking until the data has been sent to the serial port
 * controler.
 *
 * @param[in] port The desired port to write the data to.
 * @param[in] data The byte to write to the serial port.
 */
void serial_write(const uint32_t port, const uint8_t data);

/**
 * @brief Write the string given as patameter on the debug port.
 * 
 * @details The function will output the data given as parameter on the debug
 * port. This call is blocking until the data has been sent to the serial port
 * controler.
 *
 * @param[in] string The string to write to the serial port.
 * @param[in] size The string size to write to the serial port.
 * 
 * @warning string must be NULL terminated.
 */
void serial_put_string(const char* string, const uint32_t size);

/**
 * @brief Write the character given as patameter on the debug port.
 * 
 * @details The function will output the character given as parameter on the 
 * debug port. This call is blocking until the data has been sent to the serial 
 * port controler.
 *
 * @param[in] character The character to write to the serial port.
 */
void serial_put_char(const char character);

#endif /* __SERIAL_H_ */
