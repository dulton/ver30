#pragma once

#include "cross_platform_headers.h"
#include "smart_pointer_utils.h"

template<typename T, typename Deallocator = DefaultObjectDeleter >
class SafePtr
{
public:
    explicit SafePtr( T *Buffer = NULL )
        : m_Buffer( Buffer )
    {}

    template<typename U>
    explicit SafePtr( U *Buffer )
        : m_Buffer( Buffer )
    {}

    ~SafePtr(void)
    {
        _Release();
    }

    SafePtr( const SafePtr<T,Deallocator>& Ptr )
    {
        m_Buffer = (const_cast<SafePtr<T,Deallocator>&>(Ptr)).Release();
    }

    template<typename U>
    SafePtr( const SafePtr<U,Deallocator>& Ptr )
    {
        m_Buffer = (const_cast<SafePtr<U,Deallocator>&>(Ptr)).Release();
    }

    SafePtr<T,Deallocator >& operator = ( T *Buffer )
    {
        _Release();
        m_Buffer = Buffer;
        return (*this);
    }

    template<typename U>
    SafePtr<T,Deallocator >& operator = (U *Buffer )
    {
        _Release();
        m_Buffer = Buffer;
        return (*this);
    }

    SafePtr<T,Deallocator >& operator = ( const SafePtr<T,Deallocator >& Ptr )
    {
        if ( &Ptr != this )
        {
            _Release();
            m_Buffer = (const_cast<SafePtr<T,Deallocator>&>(Ptr)).Release();
        }
        return (*this);
    }

    template<typename U>
    SafePtr<T,Deallocator >& operator = ( const SafePtr<U,Deallocator>& Ptr )
    {
        if ( (void_t*) &Ptr != (void_t*) this )
        {
            _Release();
            m_Buffer = (const_cast<SafePtr<U,Deallocator>&>(Ptr)).Release();
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

    T* Release()
    {
        T* Buffer = m_Buffer;
        m_Buffer = NULL;
        return Buffer;
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
    void _Release()
    {
        if ( NULL != m_Buffer )
        {
            Deallocator::Free( m_Buffer );
            m_Buffer = NULL;
        }
    }

    T	*m_Buffer;
};
