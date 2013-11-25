// application_packet_header.h

#if !defined( APPLICATION_PACKET_HEADER )
#define APPLICATION_PACKET_HEADER

#include "gmi_system_headers.h"

// ------------------------------------------------------------------------ //

#if defined( __X86__ )
#define UINT64_TO_BIGENDIAN(x,b)           (b)[0]=(uint8_t)(((x)>>56)&0xFF);(b)[1]=(uint8_t)(((x)>>48)&0xFF);(b)[2]=(uint8_t)(((x)>>40)&0xFF);(b)[3]=(uint8_t)(((x)>>32)&0xFF);(b)[4]=(uint8_t)(((x)>>24)&0xFF);(b)[5]=(uint8_t)(((x)>>16)&0xFF);(b)[6]=(uint8_t)(((x)>>8)&0xFF);(b)[7]=(uint8_t)((x)&0xFF)
#define UINT_TO_BIGENDIAN(x,b)             (b)[0]=(uint8_t)(((x)>>24)&0xFF);(b)[1]=(uint8_t)(((x)>>16)&0xFF);(b)[2]=(uint8_t)(((x)>>8)&0xFF);(b)[3]=(uint8_t)((x)&0xFF)
#define USHORT_TO_BIGENDIAN(x,b)           (b)[0]=(uint8_t)(((x)>>8)&0xFF);(b)[1]=(uint8_t)((x)&0xFF)
#define BIGENDIAN_TO_UINT64(b,x)           (x=((uint64_t)(b)[0]<<56)+((uint64_t)(b)[1]<<48)+((uint64_t)(b)[2]<<40)+((uint64_t)(b)[3]<<32)+((uint64_t)(b)[4]<<24)+((uint64_t)(b)[5]<<16)+((uint64_t)(b)[6]<<8)+(uint64_t)(b)[7])
#define BIGENDIAN_TO_UINT(b,x)             (x=((b)[0]<<24)+((b)[1]<<16)+((b)[2]<<8)+(b)[3])
#define BIGENDIAN_TO_USHORT(b,x)           (x=((b)[0]<<8)+(b)[1])
#elif defined( __arm__ )
// memcpy can not handle case that x is const, for example passed parameter is 31
#define UINT64_TO_BIGENDIAN(x,b)           (b)[0]=(uint8_t)(((x)>>56)&0xFF);(b)[1]=(uint8_t)(((x)>>48)&0xFF);(b)[2]=(uint8_t)(((x)>>40)&0xFF);(b)[3]=(uint8_t)(((x)>>32)&0xFF);(b)[4]=(uint8_t)(((x)>>24)&0xFF);(b)[5]=(uint8_t)(((x)>>16)&0xFF);(b)[6]=(uint8_t)(((x)>>8)&0xFF);(b)[7]=(uint8_t)((x)&0xFF)
#define UINT_TO_BIGENDIAN(x,b)             (b)[0]=(uint8_t)(((x)>>24)&0xFF);(b)[1]=(uint8_t)(((x)>>16)&0xFF);(b)[2]=(uint8_t)(((x)>>8)&0xFF);(b)[3]=(uint8_t)((x)&0xFF)
#define USHORT_TO_BIGENDIAN(x,b)           (b)[0]=(uint8_t)(((x)>>8)&0xFF);(b)[1]=(uint8_t)((x)&0xFF)
#define BIGENDIAN_TO_UINT64(b,x)           (x=((uint64_t)(b)[0]<<56)+((uint64_t)(b)[1]<<48)+((uint64_t)(b)[2]<<40)+((uint64_t)(b)[3]<<32)+((uint64_t)(b)[4]<<24)+((uint64_t)(b)[5]<<16)+((uint64_t)(b)[6]<<8)+(uint64_t)(b)[7])
#define BIGENDIAN_TO_UINT(b,x)             (x=((b)[0]<<24)+((b)[1]<<16)+((b)[2]<<8)+(b)[3])
#define BIGENDIAN_TO_USHORT(b,x)           (x=((b)[0]<<8)+(b)[1])
#else
#define UINT64_TO_BIGENDIAN(x,b)           (b)[0]=(uint8_t)(((x)>>56)&0xFF);(b)[1]=(uint8_t)(((x)>>48)&0xFF);(b)[2]=(uint8_t)(((x)>>40)&0xFF);(b)[3]=(uint8_t)(((x)>>32)&0xFF);(b)[4]=(uint8_t)(((x)>>24)&0xFF);(b)[5]=(uint8_t)(((x)>>16)&0xFF);(b)[6]=(uint8_t)(((x)>>8)&0xFF);(b)[7]=(uint8_t)((x)&0xFF)
#define UINT_TO_BIGENDIAN(x,b)             (b)[0]=(uint8_t)(((x)>>24)&0xFF);(b)[1]=(uint8_t)(((x)>>16)&0xFF);(b)[2]=(uint8_t)(((x)>>8)&0xFF);(b)[3]=(uint8_t)((x)&0xFF)
#define USHORT_TO_BIGENDIAN(x,b)           (b)[0]=(uint8_t)(((x)>>8)&0xFF);(b)[1]=(uint8_t)((x)&0xFF)
#define BIGENDIAN_TO_UINT64(b,x)           (x=((uint64_t)(b)[0]<<56)+((uint64_t)(b)[1]<<48)+((uint64_t)(b)[2]<<40)+((uint64_t)(b)[3]<<32)+((uint64_t)(b)[4]<<24)+((uint64_t)(b)[5]<<16)+((uint64_t)(b)[6]<<8)+(uint64_t)(b)[7])
#define BIGENDIAN_TO_UINT(b,x)             (x=((b)[0]<<24)+((b)[1]<<16)+((b)[2]<<8)+(b)[3])
#define BIGENDIAN_TO_USHORT(b,x)           (x=((b)[0]<<8)+(b)[1])
#endif

#define HOST_TO_NETWORK_USHORT(x)           htons(x)
#define HOST_TO_NETWORK_UINT(x)             htonl(x)
#define NETWORK_TO_HOST_USHORT(x)           ntohs(x)
#define NETWORK_TO_HOST_UINT(x)             ntohl(x)

// ------------------------------------------------------------------------ //

// the following definition will be defined by concrete application, include tag, major and minor version number
//#define GMI_XXX_MESSAGE_TAG              "GXMT"
#define GMI_XXX_MESSAGE_TAG_LENGTH         4

//#define GMI_XXX_MESSAGE_MAJOR_VERSION    1
//#define GMI_XXX_MESSAGE_MINOR_VERSION    0

#define GMI_MESSAGE_FLAG_ENCRYPT_BODY      (1<<7)
#define GMI_MESSAGE_FLAG_FIXED_CHECKSUM    (1<<6)
#define GMI_MESSAGE_FLAG_NO_CHECKSUM_FIELD (1<<5)
#define GMI_MESSAGE_FLAG_LITTLE_ENDIAN     (1<<4)

#define GMI_MESSAGE_TYPE_REQUEST           (1<<0)
#define GMI_MESSAGE_TYPE_REPLY             (1<<1)
#define GMI_MESSAGE_TYPE_NOTIFY            (1<<2)
#define GMI_MESSAGE_TYPE_PARTIAL           (1<<3)

#define GMI_MESSAGE_FIXED_CHECKSUM_VALUE   0x12563478

// ------------------------------------------------------------------------ //

// pakcet comprise:
// base header + [extension header] + message body + checksum

// Base Message Header
struct BaseMessageHeader
{
    uint8_t      s_MessageTag[GMI_XXX_MESSAGE_TAG_LENGTH];
    // bit8: if or not encrypt message body, encryption scheme defined in extension header or other communication tunel
    // bit7: if or not check checksum field, for audio/video stream, checksum field can be igored;
    // bit6: if or not transport checksum field, for audio/video stream, no checksum is acceptable;
    // bit5: if or not data sequence is little-endian, zero indicates is network sequence.
    uint8_t      s_HeaderFlags;
    uint8_t      s_HeaderExtensionSize;//one implies four byte unit in extension section;
    uint8_t      s_MajorVersion;
    uint8_t      s_MinorVersion;
    uint16_t     s_SequenceNumber;
    uint8_t      s_PaddingByteNumber;//7-8 bits for header extension padding byte number, 5-6 bits for message padding byte number
    uint8_t      s_MessageType;
    uint16_t     s_MessageId;
    uint16_t     s_MessageLength;//actual application data length, not including message header, extension header, padding byte and checksum
};

// the following is an entire packet definition sample:
// struct QueryUserLog
// {
//	  uint8_t      s_MessageTag[GMI_XXX_MESSAGE_TAG_LENGTH];
//    uint8_t      s_HeaderFlags;
//    uint8_t      s_HeaderExtensionSize;
//    uint8_t      s_MajorVersion;
//    uint8_t      s_MinorVersion;
//    uint16_t     s_SequenceNumber;
//    uint8_t      s_PaddingByteNumber;
//    uint8_t      s_MessageType;
//    uint16_t     s_MessageId;
//    uint16_t     s_MessageLength;
//
//    no extension header
//
//    uint16_t     s_Type;
//    uint16_t     s_Subtype;
//    uint64_t     s_StartTime;
//    uint64_t     s_EndTime;
//
//    uint32_t     s_Checksum;//sum of big endian sequence in four byte uint of the above content, excluding this field, user can use other checksum way
// };

#endif//APPLICATION_PACKET_HEADER
