// gmi_type_definitions.h
// define basic data type
#if !defined( GMI_TYPE_DEFINITIONS )
#define GMI_TYPE_DEFINITIONS

#if defined( __linux__ )

//typedef signed char                 int8_t;
typedef unsigned char                 uint8_t;

typedef short int                     int16_t;
typedef unsigned short int            uint16_t;

typedef int                           int32_t;
typedef unsigned int                  uint32_t;

#if ( __WORDSIZE == 64 )
//typedef long int                    int64_t;
typedef unsigned long int             uint64_t;
#else
//typedef long long int               int64_t;
typedef unsigned long long int        uint64_t;
#endif//( __WORDSIZE == 64 )

#if defined( __cplusplus )
typedef bool                          boolean_t;
#else
typedef int32_t                       boolean_t;
#endif
typedef char                          char_t;
//typedef wchar_t                     wchar_t;

typedef signed long                   long_t;
typedef unsigned long                 ulong_t;
typedef long long                     longlong_t;
typedef unsigned long long            ulonglong_t;

typedef void                          void_t;
//typedef size_t                      size_t;

#ifndef FLT_EVAL_METHOD /* avoid type redefinition error in arm-elf-3.4.4/arm-elf/include/math.h on amba platorm during compiling */
#define FLT_EVAL_METHOD 0

//#if defined __USE_ISOC99 && defined _MATH_H && !defined _MATH_H_MATHDEF /* avoid type redefinition error in /usr/include/bits/mathdef.h on ubuntu platorm during compiling */
#ifndef _MATH_H_MATHDEF
#define _MATH_H_MATHDEF 1

typedef float                         float_t;
typedef double                        double_t;

#endif /* _MATH_H_MATHDEF */
#endif /* FLT_EVAL_METHOD */

#elif defined( _WIN32 )

typedef signed char                   int8_t;
typedef unsigned char                 uint8_t;

typedef short int                     int16_t;
typedef unsigned short int            uint16_t;

typedef int                           int32_t;
typedef unsigned int                  uint32_t;

#if ( __WORDSIZE == 64 )
typedef long int                      int64_t;
typedef unsigned long int             uint64_t;
#else
typedef long long int                 int64_t;
typedef unsigned long long int        uint64_t;
#endif//( __WORDSIZE == 64 )

#if defined( __cplusplus )
typedef bool                          boolean_t;
#else
typedef int32_t                       boolean_t;
#endif
typedef char                          char_t;
//typedef wchar_t                     wchar_t;

typedef signed long                   long_t;
typedef unsigned long                 ulong_t;
typedef long long                     longlong_t;
typedef unsigned long long            ulonglong_t;

typedef void                          void_t;
//typedef size_t                      size_t;

typedef float                         float_t;
typedef double                        double_t;

#else
#error not support current platform
#endif

typedef void_t  *FD_HANDLE;

#if !defined( __cplusplus )
#define false   0
#define true    1
#define FALSE   0
#define TRUE    1
#endif

#endif//GMI_TYPE_DEFINITIONS
