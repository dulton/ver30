#pragma once

#include "base_reader_writer_lock.h"

class WindowsReaderWriterLock : public BaseReaderWriterLock
{
protected:
    WindowsReaderWriterLock(void);

public:
    virtual ~WindowsReaderWriterLock(void);

    virtual GMI_RESULT Create( const char_t *Name )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT Destroy()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT AcquireReadLock( long_t Timeout )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT ReleaseReadLock()
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT AcquireWriteLock( long_t Timeout )
    {
        return GMI_NOT_IMPLEMENT;
    }
    virtual GMI_RESULT ReleaseWriteLock()
    {
        return GMI_NOT_IMPLEMENT;
    }
};
