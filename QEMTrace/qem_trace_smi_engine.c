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
#include "qem_trace_config.h" /* QEM Trace configuration */

#if QEM_TRACE_ENABLED

#if QEM_TRACE_TYPE == QEM_TRACE_SMI

#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>     /* open */
#include <sys/stat.h>  /* mkfifo */
#include <sys/types.h> /* posix types */
#include <unistd.h>    /* read, write */
#include <errno.h>     /* errno */
#include <sys/mman.h>  /* mmap */
#include <semaphore.h> /* semaphores */
#include <pthread.h>   /* pthread_mutex_t pthread_cond_t */

#include "qem_trace_smi_engine.h"
#include "qem_trace_logger.h"

/*******************************************************************************
 * GLOBAL VARS
 ******************************************************************************/

/** Communication status code. */
static QEM_SMI_STATUS_CODE_E error_code = QEM_SMI_UNINIT;

/** The shared memory zone */
static uint8_t* shared_buffer = NULL;

/** The shared data part of the buffer */
static uint8_t* shared_buffer_data = NULL;

/** Memory zone size. */
static uint32_t shared_buffer_size = 0;

/** The shared memory zone associated FD */
static int32_t shm_fd = -1;

/** Local buffer */
static uint8_t* local_buffer = NULL;

/** Local buffer index */
static uint32_t local_buffer_index = 0;

/** Server's synchronization semaphore. */
static sem_t* server_sem = SEM_FAILED;

/** Client's synchronization semaphore. */
static sem_t* client_sem = SEM_FAILED;

/** Monitor's mutex */
static pthread_mutex_t* mon_lock = (pthread_mutex_t*)MAP_FAILED;
/** Monitor mutex's file descriptor. */
static int32_t mon_lock_fd = -1;

/** Read block condition variable. */
static pthread_cond_t* cond_read = (pthread_cond_t*)MAP_FAILED;
/** Read block condition's file descriptor. */
static int32_t cond_read_fd = -1;

/** Write block condition variable. */
static pthread_cond_t* cond_write = (pthread_cond_t*)MAP_FAILED;
/** Write block condition's file descriptor. */
static int32_t cond_write_fd = -1;

/** Next block to use to flush the local buffer. */
uint32_t next_block = 0;

/** Number of blocks in the buffer */
static uint32_t block_count = 0;

/** Size of a buffer block. */
static uint32_t block_size = 0;

/** The communication's direction of the communicator. */
static QEM_SMI_DIRECTION_E direction = QEM_SMI_SENDER;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static int32_t mmap_buffer(void)
{
    /* Creates the SHM file descriptor */
    shm_fd = shm_open(QEM_SMI_SHM_NAME, O_RDWR, 0666);
    if(shm_fd < 0)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Could not open shared memory region", errno, 0);
        return -1;
    }

    /* Just map the metadata to get the information */
    shared_buffer = (uint8_t*)mmap(NULL, QEM_SMI_DATA_BUFFER_META_DATA_SIZE, 
                                   PROT_READ | PROT_WRITE, MAP_SHARED, 
                                   shm_fd, 0);
    if (shared_buffer == MAP_FAILED)
	{
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Could not mmap communicator header shm", errno, 0);
        return -1;
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
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Could not mmap communicator shm: %d\n", errno, 0);
        return -1;
	}

    /* Set the data part start */
    shared_buffer_data = shared_buffer + QEM_SMI_DATA_BUFFER_META_DATA_SIZE;

    return 0;
}

static int32_t create_local_buffer(void)
{
    /* Create local buffer */
    local_buffer_index = 0;
    local_buffer       = (uint8_t*)malloc(shared_buffer_size);
    if(local_buffer == NULL)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Could not create local buffer", errno, 0);
        return -1;
    }

    return 0;
}

static int32_t create_sync(void)
{
    /* Open the file descriptors */
    mon_lock_fd   = shm_open(QEM_SMI_SHARED_MUTEX, O_RDWR, 0666);
    if(mon_lock_fd < 0)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Could not create monitor mutex", errno, 0);
        return -1;
    } 

    cond_read_fd  = shm_open(QEM_SMI_SHARED_COND_READ, O_RDWR, 0666);
    if(cond_read_fd < 0)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Could not create monitor read cond", errno, 0);
        return -1;
    }

    cond_write_fd = shm_open(QEM_SMI_SHARED_COND_WRITE, O_RDWR, 0666);
    if(cond_write_fd < 0)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Could not create monitor write cond", errno, 0);
        return -1;
    }

    /* Mmap the memory */
    mon_lock = (pthread_mutex_t*)mmap(NULL, sizeof(pthread_mutex_t),
                                      PROT_READ | PROT_WRITE, MAP_SHARED, 
                                      mon_lock_fd, 0);
    if(mon_lock == MAP_FAILED)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Could not mmap monitor mutex", errno, 0);
        return -1;
    }
    cond_read = (pthread_cond_t*)mmap(NULL, sizeof(pthread_cond_t),
                                      PROT_READ | PROT_WRITE, MAP_SHARED, 
                                      cond_read_fd, 0);
    if(cond_read == MAP_FAILED)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Could not mmap monitor read cond", errno, 0);
        return -1;
    }
    cond_write = (pthread_cond_t*)mmap(NULL, sizeof(pthread_cond_t),
                                       PROT_READ | PROT_WRITE, MAP_SHARED, 
                                       cond_write_fd, 0);
    if(cond_write == MAP_FAILED)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Could not mmap monitor write cond", errno, 0);
        return -1;
    }
    
    return 0;
}

int32_t qem_smi_client_init(void)
{
    error_code = QEM_SMI_NOT_CONNECTED;
    return 0;
}

int32_t qem_smi_client_connect(const uint32_t timeout, const uint32_t max_attempt)
{
    uint32_t buffer;
    uint32_t attempt;
    int32_t  ret_val;

    struct timespec ts;
    ts.tv_sec = timeout;
    ts.tv_nsec = 0;

    if(error_code != QEM_SMI_NOT_CONNECTED)
    {
        QEM_TRACE_ERROR("You must initialize the communicator before calling"
                        " connect", 0, 0);
        qem_smi_client_disconnect();
        return -1;
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
        
        if(attempt == 0)
        {
            QEM_TRACE_INFO("Waiting for server to start", 0);
            fflush(stdout);
        }
        else 
        {
            printf(".");
            fflush(stdout);
        }
        ++attempt;
        nanosleep(&ts, NULL);
    }
    if(attempt != 0)
    {
        printf("\n");
    }
    if(attempt == max_attempt)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Cannot get server semaphores (TIMEOUT)",
                        0, 0);
        return -1;
    }
    attempt = 0;
    while(attempt < max_attempt)
    {
        client_sem = sem_open(QEM_SMI_SHARED_SEM_CLIENT, O_RDWR);
        if(client_sem != SEM_FAILED)
        {
            break;
        }
        if(attempt == 0)
        {
            QEM_TRACE_INFO("Waiting for client to start.", 0);
            fflush(stdout);
        }
        else 
        {
            printf(".");
            fflush(stdout);
        }
        ++attempt;
        nanosleep(&ts, NULL);
    }
    if(attempt != 0)
    {
        printf("\n");
    }
    if(attempt == max_attempt)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Cannot get client semaphores (TIMEOUT)",
                         0, 0);
        return -1;
    }

    /* Handshake with the client */
    sem_wait(client_sem);
    
    /* Retreive the server data */
    if((ret_val = mmap_buffer()) != 0)
    {
        qem_smi_client_disconnect();
        return ret_val;
    }

    if((ret_val = create_local_buffer()) != 0)
    {
        qem_smi_client_disconnect();
        return ret_val;
    }

    if((ret_val = create_sync()) != 0)
    {
        qem_smi_client_disconnect();
        return ret_val;
    }

    memcpy(&buffer, shared_buffer_data, sizeof(uint32_t));
    if(buffer != QEM_SMI_HANDSHAKE_MAGIC)
    {
        qem_smi_client_disconnect();
        QEM_TRACE_ERROR("Wrong Magic number detected while connecting", 0, 0);
        return -1;
    }

    sem_post(server_sem);

    if(direction == QEM_SMI_RECEIVER)
    {
        /* The first time we will read we will need to populate the buffer */
        local_buffer_index = block_size;
    }
    else 
    {
        local_buffer_index = 0;
    }

    error_code = QEM_SMI_CONNECTED;
    return 0;
}

void qem_smi_client_disconnect(void)
{
    if(error_code == QEM_SMI_CONNECTED && direction == QEM_SMI_SENDER)
    {
        qem_smi_client_flush();
    }
    
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

int32_t qem_smi_client_receive(void* buffer, const uint32_t size)
{
    uint32_t read_size;
    uint32_t to_read;
    uint32_t read = 0;
    uint32_t blocks;

    if(error_code != QEM_SMI_CONNECTED)
    {
        QEM_TRACE_ERROR("You must connect the SMI before calling "
                        "receive", 0, 0);
        return -1;
    }

    if(direction == QEM_SMI_SENDER)
    {
        QEM_TRACE_ERROR("You must create a receiver before calling "
                        "receive", 0, 0);
        return -1;
    }

    if(buffer == NULL)
    {
        QEM_TRACE_ERROR("Buffer cannot be NULL", 0, 0);
        return -1;
    }

    read_size  = size;
    while(local_buffer_index + read_size > block_size)
    {
        /* Read as much as we can */
        to_read = block_size - local_buffer_index;

        if(to_read > 0)
        {
            memcpy((uint8_t*)buffer + read,
                local_buffer + local_buffer_index,
                to_read);

            read += to_read;         
            read_size -= to_read;
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
    if(read_size != 0)
    {
        memcpy((uint8_t*)buffer + read, 
               local_buffer + local_buffer_index,
               read_size);
        local_buffer_index += read_size;
    }

    return 0;
}

int32_t qem_smi_client_send(const void* buffer, const uint32_t size)
{
    uint32_t write_size = 0;
    uint32_t to_write   = 0;
    uint32_t wrote     = 0;
    uint32_t blocks    = 0;

    if(error_code != QEM_SMI_CONNECTED)
    {
        QEM_TRACE_ERROR("You must connect the SMI before calling send",
                        0, 0);
        return -1;
    }

    if(direction == QEM_SMI_RECEIVER)
    {
        QEM_TRACE_ERROR("You must create a sender to use send", 0, 0);
        return -1;
    }

    if(buffer == NULL)
    {
        QEM_TRACE_ERROR("Buffer cannot be NULL", 0, 0);
        return -1;
    }

    /* Check if we can write to the local buffer */
    write_size = size;
    while(local_buffer_index + write_size > block_size)
    {
        /* Write as much as we can */
        to_write = block_size - local_buffer_index;
        memcpy(local_buffer + local_buffer_index, 
               (uint8_t*)buffer + wrote, 
               to_write);
        wrote += to_write;         
        write_size -= to_write;
        local_buffer_index = 0;

        
        pthread_mutex_lock(mon_lock);

        /* Check if the block is free */
        memcpy(&blocks, 
               shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET, 
               QEM_SMI_META_BLOCK_MASK_SIZE);

        while((blocks & (1 << next_block)) != 0)
        {
            pthread_cond_wait(cond_write, mon_lock);
            memcpy(&blocks, 
               shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET, 
               QEM_SMI_META_BLOCK_MASK_SIZE);
        }
        
        pthread_mutex_unlock(mon_lock);

        /* This part does not need to be locked as the other end cannot access
         * it before it is set as used in the block mask.
         */
        /* Copy the local buffer to the shared buffer */
        memcpy(shared_buffer_data + 
                    next_block * block_size,
              local_buffer,
              block_size);

        /* Update metadata */
        pthread_mutex_lock(mon_lock);

        memcpy(&blocks, 
               shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET, 
               QEM_SMI_META_BLOCK_MASK_SIZE);
        blocks |= 1 << next_block;
        memcpy(shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET,
               &blocks, 
               QEM_SMI_META_BLOCK_MASK_SIZE);
        next_block = (next_block + 1) % block_count;

        pthread_cond_signal(cond_read);

        pthread_mutex_unlock(mon_lock);
    }

    /* If there is some rest to write */
    if(write_size != 0)
    {
        memcpy(local_buffer + local_buffer_index, 
               (uint8_t*)buffer + wrote, 
               write_size);
        local_buffer_index += write_size;
    }

    return 0;
}

int32_t qem_smi_client_flush(void)
{
    uint32_t blocks;

    if(error_code != QEM_SMI_CONNECTED)
    {
        QEM_TRACE_ERROR("You must connect the SMI before calling flush", 0, 0);
        return -1;
    }

    if(direction == QEM_SMI_RECEIVER)
    {
        QEM_TRACE_ERROR("You must create a sender SMI to use flush()", 0, 0);
        return -1;
    }

    if(local_buffer_index == 0)
    {
        return 0;
    }
        
    pthread_mutex_lock(mon_lock);

    /* Check if the block is free */
    memcpy(&blocks, 
            shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET, 
            QEM_SMI_META_BLOCK_MASK_SIZE);
    while((blocks & (1 << next_block)) != 0)
    {
        pthread_cond_wait(cond_write, mon_lock);
        memcpy(&blocks, 
               shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET, 
               QEM_SMI_META_BLOCK_MASK_SIZE);
    }
    
    pthread_mutex_unlock(mon_lock);

    /* This part does not need to be locked as the other end cannot access
        * it before it is set as used in the block mask.
        */
    /* Copy the local buffer to the shared buffer */
    memset(shared_buffer_data + 
                next_block * block_size,
            0,
            block_size);

    memcpy(shared_buffer_data + 
                next_block * block_size,
            local_buffer,
            local_buffer_index);

    /* Update metadata */
    pthread_mutex_lock(mon_lock);

    memcpy(&blocks, 
            shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET, 
            QEM_SMI_META_BLOCK_MASK_SIZE);
    blocks |= 1 << next_block;
    memcpy(shared_buffer + QEM_SMI_META_BLOCK_MASK_OFFSET,
            &blocks, 
            QEM_SMI_META_BLOCK_MASK_SIZE);
    next_block = (next_block + 1) % block_count;

    pthread_cond_signal(cond_read);

    pthread_mutex_unlock(mon_lock);

    local_buffer_index = 0;

    return 0;
}

#endif /* QEM_TRACE_TYPE == QEM_TRACE_SMI */

#endif /* QEM_TRACE_ENABLED */