/*******************************************************************************
 * File: qem_posix_smi.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 19/09/2018
 *
 * Version: 1.0
 *
 * This class is an implementation of the SMI. This
 * SMI is developped for POSIX compliant environments.
 * The client is part of a 1 to 1 unidirectionnal pipe
 * between two process.
 * Its behaviour will not be correct if more than one process is using it as a
 * sender or as a receiver. At any time only one sender and only one receiver is
 * authorized.
 * The client can only receive data.
 ******************************************************************************/

#ifndef __QEM_POSIX_SMI_H_
#define __QEM_POSIX_SMI_H_

#include <stdint.h>    /* Genetric types */
#include <semaphore.h> /* semaphores */


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

/* Errors */
#define QEM_SMI_SHM_FD_ERROR         100
#define QEM_SMI_SHM_MMAP_ERROR       101
#define QEM_SMI_CREATE_LBUFFER_ERROR 102
#define QEM_SMI_SHM_LOCK_ERROR       103
#define QEM_SMI_SHM_RCOND_ERROR      104
#define QEM_SMI_SHM_WCOND_ERROR      105
#define QEM_SMI_MMAP_LOCK_ERROR      106
#define QEM_SMI_MMAP_RCOND_ERROR     107
#define QEM_SMI_MMAP_WCOND_ERROR     108
#define QEM_SMI_NOT_CONNECTED_ERROR  109
#define QEM_SMI_TIMEOUT_ERROR        110
#define QEM_SMI_WRONG_MAGIC_ERROR    111
#define QEM_SMI_NULL_BUFFER_ERROR    112

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
 * @brief Initializes the SMI memory space.
 */
void qem_smi_init(void);

            /**
 * @brief Connect the client to the server. This function will block
 * the thread until the client is connected.
 *
 * @param[in] timeout The time in seconds at which should occur
 * a timeout when waiting for the server to connect.
 * @param[in] max_attempt The maximal number of time a timeout can
 * occur before stoping the connection process.
 *
 * @warning This function should be called only once the server has
 * called its init function. Otherwise there might be old SHM junks
 * in the system that could break the communicator.
 */
int32_t qem_smi_connect(const uint32_t timeout, const uint32_t max_attempt);

/**
 * @brief Disconnects the client from the server.
 */
void qem_smi_disconnect(void);

/**
 * @brief Receive a message from the communicator. The function will
 * block the caller until all the data are received.
 *
 * @param[out] buffer The buffer to fill.
 * @param[in] size The size of the data to be received in bytes.
 */
int32_t qem_smi_receive(void* buffer, const uint32_t size);

/**
 * @brief Performs post server flush cleanup. This is to be used when
 * the server performs a flush before filling its buffer (usually on end SMI
 * sequence). Quen End SMI sequence occurs you should call this function
 * as end smi sequenc is user defined and the client cannot detect this 
 * sequence.
 */
void qem_smi_post_server_flush(void);

#endif /* __QEM_POSIX_SMI_H_ */