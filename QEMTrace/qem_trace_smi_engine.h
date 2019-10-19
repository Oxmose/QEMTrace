/*
 * Guest memory access SMI engine.
 *
 * Provides tools to send data through a shared memory.
 *
 * Copyright (c) 2019 Alexy Torres Aurora Dugo
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

#ifndef _QEM_TRACE_SMI_ENGINE_H_
#define _QEM_TRACE_SMI_ENGINE_H_

#include "qem_trace_config.h" /* QEM Trace configuration */

#if QEM_TRACE_ENABLED

#if QEM_TRACE_TYPE == QEM_TRACE_SMI

#include <stdint.h> /* Generuc types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* Stream version */
#define QEM_SMI_STREAM_VERSION    1

/* Shared memory region names */
#define QEM_SMI_SHM_NAME          "/QEM_SMI_SHM"
#define QEM_SMI_SHARED_SEM_SERVER "/QEM_SMI_SEM_SERVER"
#define QEM_SMI_SHARED_SEM_CLIENT "/QEM_SMI_SEM_CLIENT"
#define QEM_SMI_SHARED_MUTEX      "/QEM_SMI_MUTEX"
#define QEM_SMI_SHARED_COND_READ  "/QEM_SMI_COND_READ"
#define QEM_SMI_SHARED_COND_WRITE "/QEM_SMI_COND_WRITE"

#define QEM_SMI_HANDSHAKE_MAGIC   0xDEADCAFE

/* Buffer representation */
/*
 * #----------|-----------|-----------|-----------------------#
 * | BCNT 16b | BMASK 16b | BSIZE 32b | Buffer 1 ... Buffer n |
 * #----------|-----------|-----------|-----------------------#
 */

/** WARNING Block count must be set accordingly to the size of the block mask.
 * defined as QEM_SMI_META_BLOCK_MASK_SIZE (1 block per bit).
 */
#define QEM_SMI_DATA_FIRST_BUFFER_OFFSET SMI_DATA_BUFFER_META_DATA_SIZE

#define QEM_SMI_META_BLOCK_COUNT_OFFSET 0
#define QEM_SMI_META_BLOCK_COUNT_SIZE   2

#define QEM_SMI_META_BLOCK_MASK_OFFSET \
(QEM_SMI_META_BLOCK_COUNT_OFFSET + QEM_SMI_META_BLOCK_COUNT_SIZE)
#define QEM_SMI_META_BLOCK_MASK_SIZE    2

#define QEM_SMI_META_BLOCK_SIZE_OFFSET  \
(QEM_SMI_META_BLOCK_MASK_OFFSET + QEM_SMI_META_BLOCK_MASK_SIZE)
#define QEM_SMI_META_BLOCK_SIZE_SIZE    4

#define QEM_SMI_DATA_BUFFER_META_DATA_SIZE \
(QEM_SMI_META_BLOCK_SIZE_OFFSET + QEM_SMI_META_BLOCK_SIZE_SIZE)

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/** SMI status code enumeration */
typedef enum 
{
    QEM_SMI_UNINIT,
    QEM_SMI_NOT_CONNECTED,
    QEM_SMI_CONNECTED,
    QEM_SMI_ERR_ABORTED,
    QEM_SMI_FAILED_CREATE_PIPE
} QEM_SMI_STATUS_CODE_E;

/* SMI direction */
typedef enum 
{
    QEM_SMI_RECEIVER,
    QEM_SMI_SENDER
} QEM_SMI_DIRECTION_E;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the SMI. 
 * 
 * @returns -1 is returned in case of error during the communication. 0 is 
 * returned otherwise.
 */
int32_t qem_smi_client_init(void);

/**
 * @brief Connect the SMI server with the client. 
 * 
 * @param[in] timeout The time in milliseconds at which should occur
 * a timeout when waiting for the server to connect.
 * @param[in] max_attempt The maximal number of time a timeout can 
 * occur before stoping the connection process.
 * 
 * @returns -1 is returned in case of error during the communication. 0 is 
 * returned otherwise.
 */
int32_t qem_smi_client_connect(const uint32_t timeout, 
                               const uint32_t max_attempt);

/**
 * @brief Disconnects the client from the server.
 */
void qem_smi_client_disconnect(void);

/**
 * @brief Receive a message from the SMI. The function will
 * block the process until a message is received and the buffer is
 * fully filled or an error occured.
 * 
 * @param[out] buffer The buffer in which the received data will be
 * copied.
 * @param[in] size The size of the data to fill in the buffer in 
 * bytes.
 * 
 * @returns -1 is returned in case of error during the communication. 0 is 
 * returned otherwise.
 */
int32_t qem_smi_client_receive(void* buffer, const uint32_t size);

/**
 * @brief Sent a message to the SMI. The function will
 * copy the buffer to an internal buffer in order to let the caller
 * run. If the internal buffer is full, the function will then be
 * blocking.
 * 
 * @param[in] buffer The buffer containing the data to be sent.
 * @param[in] size The size of the data to be sent in bytes.
 * 
 * @returns -1 is returned in case of error during the communication. 0 is 
 * returned otherwise.
 */
int32_t qem_smi_client_send(const void* buffer, const uint32_t size);

/**
 * @brief Flushed the content of the local buffer to the shared 
 * buffer.
 * This function will block until the next block in the buffer is 
 * free.
 */
int32_t qem_smi_client_flush(void);

#endif /* QEM_TRACE_TYPE == QEM_TRACE_SMI */

#endif /* QEM_TRACE_ENABLED */

#endif /* _QEM_TRACE_SMI_ENGINE_H_ */
