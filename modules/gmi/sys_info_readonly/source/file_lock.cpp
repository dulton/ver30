#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include "file_lock.h"

CSemaphoreMutex::CSemaphoreMutex()
    : m_created(false), m_id(-1)
{
}

CSemaphoreMutex::~CSemaphoreMutex()
{
}

bool CSemaphoreMutex::Create(long key)
{
	int ret;
    if (m_created)
    {
        return true;
    }

    m_id = semget(key, 1, 0666);
    if (m_id == -1)
    {
        m_id = semget(key, 1, IPC_CREAT | 0666);
        if (m_id == -1)
        {
            APP_ERROR("Create semaphore failed");
            return false;
        }
		ret = semctl(m_id, 0, SETVAL, 1);
		if (ret < 0)
		{
			/*we should remove the id when failed to set*/
			semctl(m_id, 0, IPC_RMID);
			m_id = -1;
			return false;
		}
    }    

    m_created = true;
    APP_INFO("CSemaphoreMutex is Created");
    return true;
}

void CSemaphoreMutex::Destroy()
{
    if ( m_created )
    {
		/*we need not to make the sem id destroy ,because other process will use it*/
    	// semctl(m_id, 0, IPC_RMID);
    }

    m_created = false;
}

void CSemaphoreMutex::Lock() const
{
    if ( m_created )
    {
        struct sembuf op;
        op.sem_num = 0;
        op.sem_op = -1;
        op.sem_flg = SEM_UNDO;

        semop(m_id, &op, 1);
    }
}

void CSemaphoreMutex::Unlock() const
{
    if ( m_created )
    {
        struct sembuf op;
        op.sem_num = 0;
        op.sem_op = 1;
        op.sem_flg = SEM_UNDO;

        semop(m_id, &op, 1);
    }
}

bool CSemaphoreMutex::TryLock() const
{
    if ( m_created )
    {
        struct sembuf op;
        op.sem_num = 0;
        op.sem_op = -1;
        op.sem_flg = SEM_UNDO | IPC_NOWAIT;

        return (semop(m_id, &op, 1) == 0);
    }

    return false;
}

bool CSemaphoreMutex::IsLocked() const
{
    if ( m_created )
    {
        return semctl(m_id, 0, GETVAL) <= 0;
    }

    return false;
}


