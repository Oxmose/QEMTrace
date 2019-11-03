
/***************************************************************************//**
 * @file gic.h
 *
 * @see gic.c
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

#ifndef __GIC_H_
#define __GIC_H_

#include "common.h" /* Generic types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
/** @brief GICD CTLR register offset. */
#define GICD_CTLR_REG 0x1000

/** @brief GICC CTLR register offset. */
#define GICC_CTLR_REG 0x2000

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** @brief Holds the CPU register values */
struct cpu_state
{
    /** @brief CPU's SP register. */
    uint32_t sp;
    /** @brief CPU's LR register. */
    uint32_t lr;
    /** @brief CPU's PC register. */
    uint32_t pc;

    /** @brief CPU's registers. */
    uint32_t registers[13];

} __attribute__((packed));

/** 
 * @brief Defines cpu_state_t type as a shorcut for struct cpu_state.
 */
typedef struct cpu_state cpu_state_t;

/** @brief Hold the stack state before the interrupt */
struct stack_state
{
    /** @brief CSPR value before interrupt. */
    uint32_t cspr;
} __attribute__((packed));

/** 
 * @brief Defines stack_state_t type as a shorcut for struct stack_state.
 */
typedef struct stack_state stack_state_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Enables the GIC.
 *
 * @details Enables the GIC interrupts by enabling the GICC and GICD CTLR 
 * registers. 
 */
void gic_enable(void);

/**
 * @brief Disables the GIC.
 *
 * @details Disables the GIC interrupts by disabling the GICC and GICD CTLR 
 * registers. 
 */
void gic_disable(void);

/** 
 * @brief Returns the GIC interrupt enable status.
 * 
 * @details Returns the GIC interrupt enable status by reading the GICC and GICD 
 * CTLR registers.
 * 
 * @return The function returns 0 is interrupts are disabled, >0 otherwise.
 */
uint32_t gic_get_status(void);

#endif /* __GIC_H_ */
