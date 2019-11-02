/***************************************************************************//**
 * @file gic.c
 *
 * @see gic.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/05/2019
 *
 * @version 1.0
 *
 * @brief GIC management functions
 *
 * @details GIC management functions.Used to set the GIC and configure the 
 * module.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include "common.h" /* Generic types */

/* Header file */
#include "gic.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief Peripheral base address. Todo: add actual address */
volatile uint32_t* periph_base_address;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void gic_enable(void)
{
    periph_base_address[GICD_CTLR_REG] = 1;
    periph_base_address[GICC_CTLR_REG] = 1;
}

void gic_disable(void)
{
    periph_base_address[GICD_CTLR_REG] = 0;
    periph_base_address[GICC_CTLR_REG] = 0;
}

uint32_t gic_get_status(void)
{
    return periph_base_address[GICD_CTLR_REG] |  
           periph_base_address[GICC_CTLR_REG];
}