// memory_extractor_unitest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gmi_system_headers.h"

#if defined( _WIN32 )
int32_t _tmain( int32_t argc, _TCHAR* argv[] )
#elif defined( __linux__ )
int32_t main( int32_t argc, char_t* argv[] )
#endif
{
    const size_t MemoryPageSize = 4096;
    void_t *Temp = NULL;
    uint64_t TotalAllocatedSize = 0;

    for (;;)
    {
        Temp = BaseMemoryManager::Instance().Allocate( MemoryPageSize );
        if ( NULL == Temp )
        {
            break;
        }

        memset( Temp, 0, MemoryPageSize );

        TotalAllocatedSize += MemoryPageSize;
        printf( "Allocated %d bytes memory, total allocated = %lld\n", MemoryPageSize, TotalAllocatedSize );
    }

    for (;;)
    {
        GMI_Sleep( 1000 );
    }

    return 0;
}
