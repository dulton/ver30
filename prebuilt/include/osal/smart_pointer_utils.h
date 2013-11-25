// smart_pointer_util.h
// define support classes for smart pointer classes
#pragma once

#include "cross_platform_headers.h"
#include "base_memory_manager.h"

// allocator
struct DefaultBufferAllocator
{
    static inline void_t* Allocate( size_t Size )
    {
        return BaseMemoryManager::Instance().Allocate( Size );
    }
};

struct DefaultObjectAllocator
{
    template<typename T>
    static inline T* Allocate()
    {
        return BaseMemoryManager::Instance().New<T>();
    }
};

struct DefaultObjectsAllocator
{
    template<typename T>
    static inline T* Allocate( size_t Number )
    {
        return BaseMemoryManager::Instance().News<T>( Number );
    }
};

// deallocator
struct DefaultBufferDeallocator
{
    static inline void_t Free( void_t *Buffer )
    {
        BaseMemoryManager::Instance().Free( Buffer );
    }
};

struct DefaultObjectDeleter
{
    template<typename T>
    static inline void_t Free( T *Object )
    {
        BaseMemoryManager::Instance().Delete( Object );
    }
};

struct DefaultObjectsDeleter
{
    template<typename T>
    static inline void_t Free( T *Objects )
    {
        BaseMemoryManager::Instance().Deletes( Objects );
    }
};

// locker
struct SingleThreadModel
{
    template<typename T>
    static inline GMI_RESULT Lock( T *Object )
    {
        return GMI_SUCCESS;
    }

    template<typename T>
    static inline GMI_RESULT Unlock( T *Object )
    {
        return GMI_SUCCESS;
    }
};

struct MultipleThreadModel
{
    template<typename T>
    static inline GMI_RESULT Lock( T *Object )
    {
        if ( NULL == Object )
        {
            return GMI_SUCCESS;
        }
        return Object->Lock();
    }

    template<typename T>
    static inline GMI_RESULT Unlock( T *Object )
    {
        if ( NULL == Object )
        {
            return GMI_SUCCESS;
        }
        return Object->Unlock();
    }
};
