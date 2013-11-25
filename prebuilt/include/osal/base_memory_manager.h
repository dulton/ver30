#pragma once

#include "cross_platform_headers.h"

class BaseMemoryManager
{
protected:
    BaseMemoryManager(void);

public:
    virtual ~BaseMemoryManager(void);
    static BaseMemoryManager& Instance();

    void_t* Allocate( size_t Size );
    void_t Free( void_t *Buffer );

#if defined( __cplusplus )

    // new single object/struct
    template<typename T>
    T* New()
    {
        return new(std::nothrow) T;
    }

    template<typename T,typename P1>
    T* New( P1 p1 )
    {
        return new(std::nothrow) T(p1);
    }

    template<typename T,typename P1,typename P2>
    T* New( P1 p1, P2 p2 )
    {
        return new(std::nothrow) T(p1,p2);
    }

    template<typename T,typename P1,typename P2,typename P3>
    T* New( P1 p1, P2 p2, P3 p3 )
    {
        return new(std::nothrow) T(p1,p2,p3);
    }

    template<typename T,typename P1,typename P2,typename P3,typename P4>
    T* New( P1 p1, P2 p2, P3 p3, P4 p4 )
    {
        return new(std::nothrow) T(p1,p2,p3,p4);
    }

    template<typename T,typename P1,typename P2,typename P3,typename P4,typename P5>
    T* New( P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 )
    {
        return new(std::nothrow) T(p1,p2,p3,p4,p5);
    }

    // placement new to construct
    // new single object/struct
    template<typename T>
    T* Construct( void_t *Buffer )
    {
        return new ( Buffer ) T;
    }

    template<typename T,typename P1>
    T* Construct( void_t *Buffer, P1 p1 )
    {
        return new ( Buffer ) T(p1);
    }

    template<typename T,typename P1,typename P2>
    T* Construct( void_t *Buffer, P1 p1, P2 p2 )
    {
        return new ( Buffer ) T(p1,p2);
    }

    template<typename T,typename P1,typename P2,typename P3>
    T* Construct( void_t *Buffer, P1 p1, P2 p2, P3 p3 )
    {
        return new ( Buffer ) T(p1,p2,p3);
    }

    template<typename T,typename P1,typename P2,typename P3,typename P4>
    T* Construct( void_t *Buffer, P1 p1, P2 p2, P3 p3, P4 p4 )
    {
        return new ( Buffer ) T(p1,p2,p3,p4);
    }

    template<typename T,typename P1,typename P2,typename P3,typename P4,typename P5>
    T* Construct( void_t *Buffer, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 )
    {
        return new ( Buffer ) T(p1,p2,p3,p4,p5);
    }

    // new multiple objects/structs
    template<typename T>
    T* News( size_t Number )
    {
        return new(std::nothrow) T[Number];
    }

    template<typename T,typename P1>
    T* News( size_t Number, P1 p1 )
    {
        return new(std::nothrow) T[Number](p1);
    }

    template<typename T,typename P1,typename P2>
    T* News( size_t Number, P1 p1, P2 p2 )
    {
        return new(std::nothrow) T[Number](p1,p2);
    }

    template<typename T,typename P1,typename P2,typename P3>
    T* News( size_t Number, P1 p1, P2 p2, P3 p3 )
    {
        return new(std::nothrow) T[Number](p1,p2,p3);
    }

    template<typename T,typename P1,typename P2,typename P3,typename P4>
    T* News( size_t Number, P1 p1, P2 p2, P3 p3, P4 p4 )
    {
        return new(std::nothrow) T[Number](p1,p2,p3,p4);
    }

    template<typename T,typename P1,typename P2,typename P3,typename P4,typename P5>
    T* News( size_t Number, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 )
    {
        return new(std::nothrow) T[Number](p1,p2,p3,p4,p5);
    }

    // placement new to constructs
    // new multiple objects/structs
    template<typename T>
    T* Constructs( void_t *Buffer, size_t Number )
    {
        return new ( Buffer ) T[Number];
    }

    template<typename T,typename P1>
    T* Constructs( void_t *Buffer, size_t Number, P1 p1 )
    {
        return new ( Buffer ) T[Number](p1);
    }

    template<typename T,typename P1,typename P2>
    T* Constructs( void_t *Buffer, size_t Number, P1 p1, P2 p2 )
    {
        return new ( Buffer ) T[Number](p1,p2);
    }

    template<typename T,typename P1,typename P2,typename P3>
    T* Constructs( void_t *Buffer, size_t Number, P1 p1, P2 p2, P3 p3 )
    {
        return new ( Buffer ) T[Number](p1,p2,p3);
    }

    template<typename T,typename P1,typename P2,typename P3,typename P4>
    T* Constructs( void_t *Buffer, size_t Number, P1 p1, P2 p2, P3 p3, P4 p4 )
    {
        return new ( Buffer ) T[Number](p1,p2,p3,p4);
    }

    template<typename T,typename P1,typename P2,typename P3,typename P4,typename P5>
    T* Constructs( void_t *Buffer, size_t Number, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 )
    {
        return new ( Buffer ) T[Number](p1,p2,p3,p4,p5);
    }

    // delete single object/struct
    template<typename T>
    void_t Delete( T* Object )
    {
        delete Object;
    }

    // delete multiple objects/structs
    template<typename T>
    void_t Deletes( T* Objects )
    {
        delete [] Objects;
    }

#endif//__cplusplus
};
