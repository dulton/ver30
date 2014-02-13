#include <stdio.h>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include "file_lock.h"



typedef enum {
    ERROR_LEVEL = 0,
    WARNING_LEVEL,
    INFO_LEVEL,
} DEBUG_LEVEL;

static int g_log_level = ERROR_LEVEL;

#define PREFIX_NONE   "\033[0m"
#define PREFIX_RED    "\033[0;31m"
#define PREFIX_GREEN  "\033[0;32m"
#define PREFIX_YELLOW "\033[1;33m"

#define PRINT(mylog, LOG_LEVEL, format, args...)    do { if (mylog >= LOG_LEVEL) {printf(format, ##args); fflush(stdout);}} while (0)

#define APP_ERROR(format, args...)   PRINT(g_log_level, ERROR_LEVEL,      PREFIX_RED"Error   [%-20s][%05d][%-20s]: " format PREFIX_NONE"\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#define APP_WARNING(format, args...) PRINT(g_log_level, WARNING_LEVEL, PREFIX_YELLOW"Warning [%-20s][%05d][%-20s]: " format PREFIX_NONE"\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#define APP_INFO(format, args...)    PRINT(g_log_level, INFO_LEVEL,     PREFIX_GREEN"Info    [%-20s][%05d][%-20s]: " format PREFIX_NONE"\n", __FILE__, __LINE__, __FUNCTION__, ##args)


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
    if ( m_created )
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


