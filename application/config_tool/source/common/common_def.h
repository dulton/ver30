#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

// Include the globe definitions
#include <gmi_system_headers.h>

// Include the debug log header file
#include "log_module.h"

#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))

inline int RetVal2Int(GMI_RESULT Value)
{
    return static_cast<int>(static_cast<uint32_t>(Value));
}

/*
 * Interface for reference
 * You can use as below:
 *
 * class ExampleClass : public IReference
 * {
 * public:
 *     ExampleClass();
 *
 * private:
 *     ~ExampleClass();
 *
 *     ... ...
 * };
 *
 * ... ...
 *
 * // Create instance and initialize reference count to 1
 * ExampleClass * Example = new ExampleClass();
 *
 * // Add one reference count to 2
 * Example->AddRef();
 *
 * // Release one reference count to 1
 * Example->Release();
 *
 * // Release one reference count to 0 and delete this instance
 * Example->Release();
 */
class IReference
{
public:
    IReference() : m_RefCount(1) {}

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
    virtual ~IReference() {}

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
 * new ExampleClass();
 *
 * ExampleClass::GetSingleton().ExampleMethod();
 * ExampleClass::GetSingletonPtr()->ExampleMethod();
 *
 * delete ExampleClass::GetSingletonPtr();
 */
template <typename T>
class Singleton
{
public:
    inline static T & GetSingleton()
    {
        ASSERT_VALID_POINT(Singleton::ms_InstancePtr);
        return *Singleton::ms_InstancePtr;
    }

    inline static T * GetSingletonPtr()
    {
        return Singleton::ms_InstancePtr;
    }

protected:
    Singleton()
    {
        ASSERT_NULL_POINT(Singleton::ms_InstancePtr);

        // Get the address of this singleton
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
 * Base class for thread
 * You can use as below:
 *
 * class ExampleClass : public CThread
 * {
 * public:
 *     ExampleClass();
 *     ~ExampleClass();
 *
 * private:
 *     virtual GMI_RESULT ThreadEntry();
 *
 *     ... ...
 * };
 *
 * ExampleClass Example;
 * GMI_RESULT   RetVal = GMI_SUCCESS;
 *
 * Example.Start();
 *
 * ... ...
 *
 * Example.Stop(&RetVal);
 */
class CThread
{
public:
    CThread() : m_Thread(0), m_Running(false), m_Stopped(true) {}
    virtual ~CThread() { Wait(); }

    inline boolean_t IsRunning() const { return m_Running; }
    inline boolean_t IsStopped() const { return m_Stopped; }

    virtual GMI_RESULT Create()
    {
        if (IsRunning())
        {
            return GMI_ALREADY_OPERATED;
        }

        m_Running = true;
        m_Stopped = false;

        if (pthread_create(&m_Thread, NULL, CThread::ThreadEntryProc, static_cast<void_t *>(this)) != 0)
        {
            PRINT_LOG(ERROR, "Failed to create thread");
            m_Running = false;
            m_Stopped = true;
            return GMI_FAIL;
        }

        return GMI_SUCCESS;
    }

    virtual GMI_RESULT Wait(GMI_RESULT * RetVal = NULL)
    {
        if (IsRunning())
        {
            m_Running = false;

            void_t * ThreadRetVal = NULL;
            pthread_join(m_Thread, &ThreadRetVal);

            if (RetVal != NULL)
            {
                *RetVal = reinterpret_cast<GMI_RESULT>(ThreadRetVal);
            }

            return GMI_SUCCESS;
        }

        return GMI_INVALID_OPERATION;
    }

protected:
    virtual GMI_RESULT ThreadEntry() = 0;

private:
    static void_t * ThreadEntryProc(void_t * Data)
    {
        ASSERT_VALID_POINT(Data);
        CThread * This   = static_cast<CThread *>(Data);
        void_t  * RetVal = reinterpret_cast<void_t *>(This->ThreadEntry());

        This->m_Stopped = true;

        return RetVal;
    }

    pthread_t m_Thread;
    boolean_t m_Running;
    boolean_t m_Stopped;
};

#endif // __COMMON_DEF_H__

