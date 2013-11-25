#pragma once

#include "cross_platform_headers.h"
#include "smart_pointer_utils.h"

template<typename T, typename Deallocator = DefaultObjectDeleter, typename ThreadModel = SingleThreadModel >
class ReferrencePtr
{
public:
    explicit ReferrencePtr( T *Buffer = NULL )
        : m_Buffer( Buffer )
    {
        m_RefCount = DefaultObjectAllocator::Allocate<long_t>();
        if ( NULL == m_RefCount )
        {
            assert( false );
        }
        *m_RefCount = 1;
    }

    template<typename U>
    explicit ReferrencePtr( U *Buffer )
        : m_Buffer( Buffer )
    {
        m_RefCount = DefaultObjectAllocator::Allocate<long_t>();
        if ( NULL == m_RefCount )
        {
            assert( false );
        }
        *m_RefCount = 1;
    }

    ~ReferrencePtr(void)
    {
        ReleaseRef();
    }

    ReferrencePtr( const ReferrencePtr<T,Deallocator,ThreadModel>& Ptr )
    {
        m_Buffer = Ptr.m_Buffer;
        m_RefCount = Ptr.m_RefCount;
        AddRef();
    }

    template<typename U>
    ReferrencePtr( const ReferrencePtr<U,Deallocator,ThreadModel>& Ptr )
    {
        m_Buffer = Ptr.GetPtr();
        m_RefCount = (reinterpret_cast<ReferrencePtr<T,Deallocator,ThreadModel>&> (const_cast<ReferrencePtr<U,Deallocator,ThreadModel>&>(Ptr))).m_RefCount;
        AddRef();
    }

    ReferrencePtr<T,Deallocator,ThreadModel>& operator = ( T *Buffer )
    {
        ReleaseRef();
        m_Buffer = Buffer;
        m_RefCount = DefaultObjectAllocator::Allocate<long_t>();
        if ( NULL == m_RefCount )
        {
            assert( false );
        }
        *m_RefCount = 1;
        return (*this);
    }

    template<typename U>
    ReferrencePtr<T,Deallocator,ThreadModel>& operator = ( U *Buffer )
    {
        ReleaseRef();
        m_Buffer = Buffer;
        m_RefCount = DefaultObjectAllocator::Allocate<long_t>();
        if ( NULL == m_RefCount )
        {
            assert( false );
        }
        *m_RefCount = 1;
        return (*this);
    }

    ReferrencePtr<T,Deallocator,ThreadModel>& operator = ( const ReferrencePtr<T,Deallocator,ThreadModel>& Ptr )
    {
        if ( &Ptr != this )
        {
            ReleaseRef();
            m_Buffer = Ptr.m_Buffer;
            m_RefCount = Ptr.m_RefCount;
            AddRef();
        }
        return (*this);
    }

    template<typename U>
    ReferrencePtr<T,Deallocator,ThreadModel>& operator = ( const ReferrencePtr<U,Deallocator,ThreadModel>& Ptr )
    {
        if ( (void_t*) &Ptr != (void_t*) this )
        {
            ReleaseRef();
            m_Buffer = Ptr.GetPtr();
            m_RefCount = (reinterpret_cast<ReferrencePtr<T,Deallocator,ThreadModel>&> (const_cast<ReferrencePtr<U,Deallocator,ThreadModel>&>(Ptr))).m_RefCount;
            AddRef();
        }
        return (*this);
    }

    T* operator ->()
    {
        return m_Buffer;
    }

    operator const void_t *()
    {
        return m_Buffer;
    }

    T* GetPtr()
    {
        return m_Buffer;
    }

    T* GetPtr() const
    {
        return m_Buffer;
    }

private:
    long_t  AddRef()
    {
        ThreadModel::Lock( m_Buffer );
        long_t Result = ++(*m_RefCount);
        ThreadModel::Unlock( m_Buffer );
        return Result;
    }

    long_t  ReleaseRef()
    {
        ThreadModel::Lock( m_Buffer );
        if ( 0 < *m_RefCount )
        {
            --(*m_RefCount);

            if ( 0 == *m_RefCount )
            {
                ThreadModel::Unlock( m_Buffer );
                if ( NULL != m_Buffer )
                {
                    Deallocator::Free( m_Buffer );
                    m_Buffer = NULL;
                }

                DefaultObjectDeleter::Free( m_RefCount );
                m_RefCount = NULL;
                return 0;
            }
        }

        ThreadModel::Unlock( m_Buffer );
        return *m_RefCount;
    }

private:
    T       *m_Buffer;
    long_t  *m_RefCount;
};
