#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#include <gmi_system_headers.h>

#include "log_module.h"

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

#define BITS_PER_BYTE   8

/*
 * Base class that for reference
 */
class IBase
{
public:
    IBase() : m_RefCount(1) {}

    inline uint32_t AddRef() { return ++ m_RefCount; }
    inline uint32_t Release()
    {
        uint32_t RefCount = -- m_RefCount;
        if (0 == RefCount)
        {
            delete this;
        }

        return RefCount;
    }

protected:
    virtual ~IBase() {}

private:
    uint32_t m_RefCount;
};

/*
 * Template class for instance
 * You can use as below:
 *
 * class ExampleClass : public Instance<ExampleClass>
 * {
 * friend class Instance<ExampleClass>;
 * 
 * private:
 *     ExampleClass();
 *     ~ExampleClass();
 *
 * public:
 *     void ExampleMethod();
 *
 *     ... ...
 * };
 *
 * ... ...
 *
 * ExampleClass::GetInstance().ExampleMethod();
 * ExampleClass::GetInstancePtr()->ExampleMethod();
 *
 */
template <typename T>
class Instance
{
public:
    static T & GetInstance()
    {
        static T instance;
        return instance;
    }

    static T * GetInstancePtr()
    {
        return & GetInstance();
    }
};

/*
 * Template class for singleton
 * You can use as below:
 *
 * class ExampleClass : public Singleton<ExampleClass>
 * {
 * public:
 *     ExampleClass();
 *     ~ExampleClass();
 *
 *     void ExampleMethod();
 *
 *     ... ...
 * };
 *
 * template <> ExampleClass * Singleton<ExampleClass>::s_InstancePtr = NULL;
 *
 * ... ...
 *
 * if (!ExampleClass::IsCreated()) new ExampleClass();
 *
 * ExampleClass::GetSingleton().ExampleMethod();
 * ExampleClass::GetSingletonPtr()->ExampleMethod();
 *
 * if (ExampleClass::IsCreated()) delete ExampleClass::GetSingletonPtr();
 */
template <typename T>
class Singleton
{
public:
    inline static T & GetSingleton()
    {
        ASSERT(IsCreated(), "You MUST create this class first");
        return *Singleton::ms_InstancePtr;
    }

    inline static T * GetSingletonPtr()
    {
        return Singleton::ms_InstancePtr;
    }

    inline static boolean_t IsCreated()
    {
        return (Singleton::ms_InstancePtr != NULL);
    }

protected:
    Singleton()
    {
        ASSERT(Singleton::ms_InstancePtr == NULL, "Singleton is already created");

        int32_t offset = (int32_t)(Singleton *)(T *)4 - 4;
        Singleton::ms_InstancePtr = (T *)((uint8_t *)this - offset);
    }

    virtual ~Singleton()
    {
        Singleton::ms_InstancePtr = NULL;
    }

private:

    static T * ms_InstancePtr;
};

/*
 * Basic entry for queue, list, vector, stack or other data structure
 */
class Entry {
public:
    Entry() : m_Next(this), m_Prev(this) {}
    virtual ~Entry() {}

    inline void InsertBefore(Entry * Node)
    {
        m_Prev = Node->m_Prev;
        m_Next = Node;
        m_Prev->m_Next = this;
        Node->m_Prev = this;
    }

    inline void InsertAfter(Entry * Node)
    {
        m_Next = Node->m_Next;
        m_Prev = Node;
        m_Next->m_Prev = this;
        Node->m_Next = this;
    }

    inline void Detach()
    {
        m_Next->m_Prev = m_Prev;
        m_Prev->m_Next = m_Next;
        m_Next = m_Prev = this;
    }

    inline Entry * Next() const { return m_Next; }
    inline Entry * Prev() const { return m_Prev; }

protected:
    Entry * m_Next;
    Entry * m_Prev;
};

/*
 * GMI Auto lock
 */
class GMI_AutoLock
{
public:
    GMI_AutoLock(GMI_Mutex & Mutex) : m_Mutex(Mutex) { m_Mutex.Lock(TIMEOUT_INFINITE); }
    ~GMI_AutoLock() { m_Mutex.Unlock(); }

private:
    GMI_Mutex & m_Mutex;
};

#define GMI_AUTO_LOCK(lock) GMI_AutoLock __auto_lock__(lock)

/*
 * Auto lock
 */
class AutoLock
{
public:
    AutoLock(pthread_mutex_t & Mutex) : m_Mutex(Mutex) { pthread_mutex_lock(&m_Mutex); }
    ~AutoLock() { pthread_mutex_unlock(&m_Mutex); }

private:
    pthread_mutex_t & m_Mutex;
};

#define AUTO_LOCK(lock) AutoLock __auto_lock__(lock)

#endif // __COMMON_DEF_H__

