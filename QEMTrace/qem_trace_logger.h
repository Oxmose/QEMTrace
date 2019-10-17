/*
 * QEM Trace global logger.
 *
 * Provides QEMTrace the logging facility.
 * 
 * Header included in
 *     accel/tcg/qem_trace_engine.h
 *
 *  Copyright (c) 2019 Alexy Torres Aurora Dugo
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __QEM_TRACE_LOGGER_H_
#define __QEM_TRACE_LOGGER_H_

#include "qem_trace_config.h" /* QEM Trace configuration */

#if QEM_TRACE_ENABLED

#include <stdio.h> /* printf */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define QEM_ERROR_TCG_DIS 100

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Info message */
#define QEM_TRACE_INFO(MSG, CODE){     \
    printf("[QEMINFO] %s");            \
    if(CODE != 0)                      \
    {                                  \
        printf(" | Code %d\n", CODE);  \
    }                                  \
    else                               \
    {                                  \
        printf("\n");                  \
    }                                  \
}

/* Warning message */
#define QEM_TRACE_WARNING(MSG, CODE){  \
    printf("[QEMWARN] %s");            \
    if(CODE != 0)                      \
    {                                  \
        printf(" | Code %d\n", CODE);  \
    }                                  \
    else                               \
    {                                  \
        printf("\n");                  \
    }                                  \
}

/* Error message */
#define QEM_TRACE_ERROR(MSG, CODE, CRITICAL){ \
    printf("[QEMERROR] %s");                  \
    if(CODE != 0)                             \
    {                                         \
        printf(" | Code %d\n", CODE);         \
    }                                         \
    else                                      \
    {                                         \
        printf("\n");                         \
    }                                         \
                                              \
    if(CRITICAL != 0)                         \
    {                                         \
        exit(CODE);                           \
    }                                         \
}

#endif /* QEM_TRACE_ENABLED */

#endif /* __QEM_TRACE_LOGGER_H_ */