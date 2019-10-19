/*******************************************************************************
 * File: PosixCommunicatorClient.cpp
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 19/09/2018
 *
 * Version: 1.0
 *
 * This class is an implementation of the communicator interface. This
 * communicator is developped for POSIX compliant environments.
 * A PosixCommunicatorClient is the server part of a 1 to 1 unidirectionnal pipe
 * between two process.
 * Its behaviour will not be correct if more than one process is using it as a
 * sender or as a receiver. At any time only one sender and only one receiver is
 * authorized.
 * The server can only send and the client can only receive.
 ******************************************************************************/

#include <stdint.h>    /* uint32_t */
#include <stdlib.h>    /* malloc */
#include <string.h>    /* memcpy */
#include <fcntl.h>     /* open */
#include <sys/stat.h>  /* mkfifo */
#include <sys/types.h> /* posix types */
#include <unistd.h>    /* read, write */
#include <errno.h>     /* errno */
#include <semaphore.h> /* semaphores */
#include <pthread.h>   /* pthread_mutex */
#include <time.h>      /* nanosleep */
#include <sys/mman.h>  /* mmap */

/* nsCommunicator::CommunicatorException */
#include "qem_posix_smi.h"

/** Communication status code. */
QEM_SMI_STATUS_CODE_E error_code = QEM_SMI_UNINIT;

/** The shared memory zone */
uint8_t* shared_buffer = NULL;

/** The shared data aprt of the buffer */
uint8_t* shared_buffer_data = NULL;

/** Memory zone size. */
uint32_t shared_buffer_size = 0;

/** The shared memory zone associated FD */
int32_t shm_fd = -1;

/** Local buffer */
uint8_t* local_buffer = NULL;

/** Local buffer index */
uint32_t local_buffer_index = 0;

/** Server's synchronization semaphore. */
sem_t* server_sem = SEM_FAILED;

/** Client's synchronization semaphore. */
sem_t* client_sem = SEM_FAILED;

/** Monitor's mutex */
pthread_mutex_t* mon_lock = (pthread_mutex_t*)MAP_FAILED;
/** Monitor mutex's file descriptor. */
int32_t mon_lock_fd = -1;

/** Read block condition variable. */
pthread_cond_t* cond_read = (pthread_cond_t*)MAP_FAILED;
/** Read block condition's file descriptor. */
int32_t cond_read_fd = -1;

/** Write block condition variable. */
pthread_cond_t* cond_write = (pthread_cond_t*)MAP_FAILED;
/** Write block condition's file descriptor. */
int32_t cond_write_fd = -1;

/** Next block to use to flush the local buffer. */
uint32_t next_block = 0;

/** Number of blocks in the buffer */
uint32_t block_count = 0;

/** Size of a buffer block. */
uint32_t block_size = 0;


static int32_t mapp_buffer(void)
{
    /* Creates the SHM file descriptor */
    shm_fd = shm_open(QEM_SMI_SHM_NAME, O_RDWR, 0666);
    if(shm_fd < 0)
    {
        qem_smi_disconnect();
        return QEM_SMI_SHM_FD_ERROR;
    }

    /* Just map the metadata to get the information */
    shared_buffer = (uint8_t*)mmap(NULL, QEM_SMI_DATA_BUFFER_META_DATA_SIZE,
                                         PROT_READ | PROT_WRITE, MAP_SHARED,
                                         shm_fd, 0);
    if (shared_buffer == MAP_FAILED)
	{
        qem_smi_disconnect();
        return QEM_SMI_SHM_MMAP_ERROR;
	}

    /* Get the metadata */
    memcpy(&block_count,
           shared_buffer + QEM_SMI_META_BLOCK_COUNT_OFFSET,
           QEM_SMI_META_BLOCK_COUNT_SIZE);
    memcpy(&block_size,
           shared_buffer + QEM_SMI_META_BLOCK_SIZE_OFFSET,
           QEM_SMI_META_BLOCK_SIZE_SIZE);

    shared_buffer_size = block_count * block_size +
                             QEM_SMI_DATA_BUFFER_META_DATA_SIZE;

    /* Unmap the temporary metadata */
    munmap(shared_buffer, QEM_SMI_DATA_BUFFER_META_DATA_SIZE);

    /* Map the shared memory region */
    shared_buffer = (uint8_t*)mmap(NULL, shared_buffer_size,
                                         PROT_READ | PROT_WRITE, MAP_SHARED,
                                         shm_fd, 0);
    if (shared_buffer == MAP_FAILED)
	{
        qem_smi_disconnect();
        return QEM_SMI_SHM_MMAP_ERROR;
	}

    /* Set the data part start */
    shared_buffer_data = shared_buffer + QEM_SMI_DATA_BUFFER_META_DATA_SIZE;

    return 0;
}

static int32_t createlocal_buffer(void)
{
    /* Create local buffer */
    local_buffer_index = 0;
    local_buffer       = (uint8_t*)malloc(shared_buffer_size);
    if (local_buffer == NULL)
	{
        qem_smi_disconnect();
        return QEM_SMI_CREATE_LBUFFER_ERROR;
	}

    return 0;
}

static int32_t create_sync(void)
{
    /* Open the file descriptors */
    mon_lock_fd   = shm_open(QEM_SMI_SHARED_MUTEX, O_RDWR, 0666);
    if(mon_lock_fd < 0)
    {
        qem_smi_disconnect();
        return QEM_SMI_SHM_LOCK_ERROR;
    }

    cond_read_fd  = shm_open(QEM_SMI_SHARED_COND_READ, O_RDWR, 0666);
    if(cond_read_fd < 0)
    {
        qem_smi_disconnect();
        return QEM_SMI_SHM_RCOND_ERROR;
    }

    cond_write_fd = shm_open(QEM_SMI_SHARED_COND_WRITE, O_RDWR, 0666);
    if(cond_write_fd < 0)
    {
        qem_smi_disconnect();
        return QEM_SMI_SHM_WCOND_ERROR;
    }

    /* Mmap the memory */
    mon_lock = (pthread_mutex_t*)mmap(NULL, sizeof(pthread_mutex_t),
                                           PROT_READ | PROT_WRITE, MAP_SHARED,
                                           mon_lock_fd, 0);
    if(mon_lock == MAP_FAILED)
    {
        qem_smi_disconnect();
        return QEM_SMI_MMAP_LOCK_ERROR;
    }
    cond_read = (pthread_cond_t*)mmap(NULL, sizeof(pthread_cond_t),
                                        PROT_READ | PROT_WRITE, MAP_SHARED,
                                        cond_read_fd, 0);
    if(cond_read == MAP_FAILED)
    {
        qem_smi_disconnect();
        return QEM_SMI_MMAP_RCOND_ERROR;
    }
    cond_write = (pthread_cond_t*)mmap(NULL, sizeof(pthread_cond_t),
                                         PROT_READ | PROT_WRITE, MAP_SHARED,
                                         cond_write_fd, 0);
    if(cond_write == MAP_FAILED)
    {
        qem_smi_disconnect();
        return QEM_SMI_MMAP_WCOND_ERROR;
    }

    return 0;
}

void qem_smi_init(void)
{
    shared_buffer_data = shared_buffer;
    error_code = QEM_SMI_NOT_CONNECTED;
}

int32_t qem_smi_connect(const uint32_t timeout, const uint32_t max_attempt)
{
    uint32_t buffer;
    uint32_t attempt;
    int32_t  error;

    struct timespec ts;
    ts.tv_sec = timeout;
    ts.tv_nsec = 0;

    if(error_code != QEM_SMI_NOT_CONNECTED)
    {
        qem_smi_disconnect();
        return QEM_SMI_NOT_CONNECTED_ERROR;
    }

    /* Initialize named semaphore for handhake */
    attempt = 0;
    while(attempt < max_attempt)
    {
        server_sem = sem_open(QEM_SMI_SHARED_SEM_SERVER, O_RDWR);
        if(server_sem != SEM_FAILED)
        {
            break;
        }

        ++attempt;
        nanosleep(&ts, NULL);
    }
    
    if(attempt == max_attempt)
    {
        qem_smi_disconnect();
        return QEM_SMI_TIMEOUT_ERROR;
    }
    attempt = 0;
    while(attempt < max_attempt)
    {
        client_sem = sem_open(QEM_SMI_SHARED_SEM_CLIENT, O_RDWR);
        if(client_sem != SEM_FAILED)
        {
            break;
        }
        
        ++attempt;
        nanosleep(&ts, NULL);
    }

    if(attempt == max_attempt)
    {
        qem_smi_disconnect();
        return QEM_SMI_TIMEOUT_ERROR;
    }

    /* Handshake with the client */
    sem_wait(client_sem);

    /* Retreive the server data */
    error = mapp_buffer();
    if(error != 0)
    {
        return error;
    }

    error = createlocal_buffer();
    if(error != 0)
    {
        return error;
    }

    error = create_sync();
    if(error != 0)
    {
        return error;
    }

    memcpy(&buffer, shared_buffer_data, sizeof(uint32_t));
    if(buffer != QEM_SMI_HANDSHAKE_MAGIC)
    {
        qem_smi_disconnect();
        return QEM_SMI_WRONG_MAGIC_ERROR;
    }

    sem_post(server_sem);

    /* The first time we will read we will need to populate the buffer */
    local_buffer_index = block_size;

    error_code = QEM_SMI_CONNECTED;

    return 0;
}

void qem_smi_disconnect(void)
{
    if(shared_buffer != NULL)
    {
        munmap(shared_buffer, shared_buffer_size);
        shared_buffer = NULL;
    }

    if(shm_fd != -1)
    {
        close(shm_fd);
        shm_unlink(QEM_SMI_SHM_NAME);
        shm_fd = -1;
    }

    if(local_buffer != NULL)
    {
        free(local_buffer);
        local_buffer = NULL;
    }

    if(server_sem != SEM_FAILED)
    {
        sem_close(server_sem);
        sem_unlink(QEM_SMI_SHARED_SEM_SERVER);
        server_sem = SEM_FAILED;
    }
    if(client_sem != SEM_FAILED)
    {
        sem_close(client_sem);
        sem_unlink(QEM_SMI_SHARED_SEM_CLIENT);
        client_sem = SEM_FAILED;
    }

    if(mon_lock != MAP_FAILED)
    {
        munmap(mon_lock, sizeof(pthread_mutex_t));
        mon_lock = (pthread_mutex_t*)MAP_FAILED;
    }
    if(mon_lock_fd != -1)
    {
        close(mon_lock_fd);
        shm_unlink(QEM_SMI_SHARED_MUTEX);
        mon_lock_fd = -1;
    }
    if(cond_read != MAP_FAILED)
    {
        munmap(cond_read, sizeof(pthread_cond_t));
        cond_read = (pthread_cond_t*)MAP_FAILED;
    }
    if(cond_read_fd != -1)
    {
        close(cond_read_fd);
        shm_unlink(QEM_SMI_SHARED_COND_READ);
        cond_read_fd = -1;
    }
    if(cond_write != MAP_FAILED)
    {
        munmap(cond_write, sizeof(pthread_cond_t));
        cond_write = (pthread_cond_t*)MAP_FAILED;
    }
    if(cond_write_fd != -1)
    {
        close(cond_write_fd);
        shm_unlink(QEM_SMI_SHARED_COND_WRITE);
        cond_write_fd = -1;
    }

    error_code = QEM_SMI_UNINIT;
}

int32_t qem_smi_receive(void* buffer, const uint32_t size)
{
    uint32_t readSize;
    uint32_t toRead;
    uint32_t read = 0;
    uint32_t blocks;

    if(error_code != QEM_SMI_CONNECTED)
    {
        return QEM_SMI_NOT_CONNECTED_ERROR;
    }

    if(buffer == NULL)
    {
        return QEM_SMI_NULL_BUFFER_ERROR;
    }

    readSize  = size;
    while(local_buffer_index + readSize > block_size)
    {
        /* Read as much as we can */
        toRead = block_size - local_buffer_index;

        if(toRead > 0)
        {
            memcpy((uint8_t*)buffer + read,
                local_buffer + local_buffer_index,
                toRead);

            read += toRead;
            readSize -= toRead;
        }

        local_buffer_index = 0;

        pthread_mutex_lock(mon_lock);

        /* Check if the block is free */
        memcpy(&blocks,
               shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET,
               QEM_SMI_META_BLOCK_MASK_SIZE);

        while((blocks & (1 << next_block)) == 0)
        {
            pthread_cond_wait(cond_read, mon_lock);
            memcpy(&blocks,
               shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET,
               QEM_SMI_META_BLOCK_MASK_SIZE);
        }

        pthread_mutex_unlock(mon_lock);

        /* This part does not need to be locked as the other end cannot access
         * it before it is set as used in the block mask.
         */
        /* Copy the shared buffer to the local buffer */
        memcpy(local_buffer,
               shared_buffer_data +
                    next_block * block_size,
               block_size);

        /* Update metadata */
        pthread_mutex_lock(mon_lock);

        memcpy(&blocks,
               shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET,
               QEM_SMI_META_BLOCK_MASK_SIZE);
        blocks &= ~(1 << next_block);
        memcpy(shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET,
               &blocks,
               QEM_SMI_META_BLOCK_MASK_SIZE);
        next_block = (next_block + 1) % block_count;

        pthread_cond_signal(cond_write);

        pthread_mutex_unlock(mon_lock);
    }

    /* If there is some rest to read */
    if(readSize != 0)
    {
        memcpy((uint8_t*)buffer + read,
               local_buffer + local_buffer_index,
               readSize);
        local_buffer_index += readSize;
    }

    return 0;
}

void qem_smi_post_server_flush(void) 
{
    local_buffer_index = block_size;
}