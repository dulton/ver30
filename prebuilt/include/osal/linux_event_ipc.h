#pragma once

#include "base_event_ipc.h"

class LinuxEventIPC : public BaseEventIPC
{
public:
    LinuxEventIPC(void);
    virtual ~LinuxEventIPC(void);

#if defined( __linux__ )
    virtual GMI_RESULT Create( long_t Key );
    virtual GMI_RESULT Open( long_t Key );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT Wait( long_t Timeout );
    virtual GMI_RESULT Signal();

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
    long_t	    m_SemaphoreId;
    boolean_t   m_IsCreator;
    long_t	    m_InternalMutexId;
#endif
};
