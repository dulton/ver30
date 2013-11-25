#pragma once

#include "base_mutex_ipc.h"

class LinuxMutexIPC : public BaseMutexIPC
{
public:
    LinuxMutexIPC(void);
    virtual ~LinuxMutexIPC(void);

#if defined( __linux__ )
    virtual GMI_RESULT Create( long_t Key );
    virtual GMI_RESULT Open( long_t Key );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Lock( long_t Timeout );
    virtual GMI_RESULT Unlock();

private:
    GMI_RESULT WaitSemaphore( long_t SemaphoreId );
    GMI_RESULT PostSemahpore( long_t SemaphoreId );

    // copy from /usr/include/linux/sem.h
    union semun
    {
        int             val;     /* value for SETVAL */
        struct semid_ds *buf;    /* buffer for IPC_STAT & IPC_SET */
        unsigned short  *array;  /* array for GETALL & SETALL */
        struct seminfo  *__buf;  /* buffer for IPC_INFO */
        void_t          *__pad;
    };

private:
    key_t       m_Key;
    long_t	    m_MutexId;
    boolean_t   m_IsCreator;
    long_t	    m_InternalMutexId;
#endif
};
