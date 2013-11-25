#pragma once

#include "base_reader_writer_lock.h"

class LinuxReaderWriterLock : public BaseReaderWriterLock
{
public:
    LinuxReaderWriterLock(void);
    virtual ~LinuxReaderWriterLock(void);

#if defined( __linux__ )
    virtual GMI_RESULT Create( const char_t *Name );
    virtual GMI_RESULT Destroy();
    virtual GMI_RESULT AcquireReadLock( long_t Timeout );
    virtual GMI_RESULT ReleaseReadLock();
    virtual GMI_RESULT AcquireWriteLock( long_t Timeout );
    virtual GMI_RESULT ReleaseWriteLock();
private:
    pthread_rwlock_t   m_ReaderWriterLock;
#endif
};
