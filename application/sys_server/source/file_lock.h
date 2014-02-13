#ifndef __FILE_LOCK_H__
#define __FILE_LOCK_H__

class CSemaphoreMutex
{
public:
    CSemaphoreMutex();
    ~CSemaphoreMutex();

    bool Create(long key);
    void Destroy();

    void Lock() const;
    void Unlock() const;
    bool TryLock() const;
    bool IsLocked() const;

private:
    bool m_created;
    int m_id;
};

#endif



