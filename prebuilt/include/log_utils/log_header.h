// log_header.h

#if !defined( GMI_LOG_HEADER )
#define GMI_LOG_HEADER

#include "application_packet_header.h"

enum LogType
{
    e_LogType_User                = 1,// user
    e_LogType_Debug               = 2,// debug
};

enum UserLogType
{
    e_UserLogType_Exception       = 1,// system exception
    e_UserLogType_Warning         = 2,// only warning, for example, remote user access IPC and passport error happen
    e_UserLogType_Operation       = 3,// operation log
};

enum DebugLogLevel
{
    e_DebugLogLevel_None          = 0,// disable log, forbid to be used log code
    e_DebugLogLevel_Exception     = 1,// program exception, including real exception case and error
    e_DebugLogLevel_Warning       = 2,// warning, for example, rare case happen, or data overwrite in ring buffer
    e_DebugLogLevel_Info          = 3,// only print, for example, call trace, or I/O data print
    e_DebugLogLevel_Loop          = 4,// frequent log info, for example, loop and multimedia frame trace
};

enum LogTransportMode
{
    e_LogTransportMode_Push            = 1,// log service module push log to receiver
    e_LogTransportMode_Pull            = 2,// user pull log from log service module
};

enum LogStorageLimitMode
{
    e_LogStorageLimitMode_RecordNumber = 1,// in record unit
    e_LogStorageLimitMode_ByteSize     = 2,// in byte unit
    e_LogStorageLimitMode_Duration     = 3,// in day unit
};

// used to transport between processes in IPC, and log server and log tool
struct UserLogInfo
{
    uint16_t    s_Type;
    uint16_t    s_Subtype;
    uint64_t    s_LogTime;
    uint16_t    s_UserNameLength;
    char_t      s_UserName[1];
    uint16_t    s_SpecificDataLength;
    uint8_t     s_SpecificData[1];
};

struct DebugLogInfo
{
    uint16_t    s_Level;
    uint16_t    s_Reserved;
    uint64_t    s_LogTime;
    uint64_t    s_ProcessId;
    uint64_t    s_ThreadId;
    uint32_t    s_ModuleId;
    uint16_t    s_ModuleNameLength;
    char_t      s_ModuleName[1];
    uint16_t    s_FileNameLength;
    char_t      s_FileName[1];
    uint16_t    s_FunctionNameLength;
    char_t      s_FunctionName[1];
    uint32_t    s_LineNumber;
    uint16_t    s_SpecificDataLength;
    uint8_t     s_SpecificData[1];
};

// ------------------------------------------------------------------------ //

#define GMI_LOG_MESSAGE_TAG             "GLMT"

#define GMI_LOG_MESSAGE_MAJOR_VERSION   1
#define GMI_LOG_MESSAGE_MINOR_VERSION   0

// command
#define GMI_LOG_REGISTER                10
#define GMI_LOG_UNREGISTER              11
#define GMI_CONFIGURE_LOG               12
#define GMI_SETUP_DEBUG_LOG             13
#define GMI_QUERY_USER_LOG              14
#define GMI_QUERY_DEBUG_LOG             15
// notify
#define GMI_REPORT_USER_LOG             1
#define GMI_REPORT_DEBUG_LOG            2

// ------------------------------------------------------------------------ //

// Log Register/Unregister
struct LogRegister : public BaseMessageHeader
{
    uint32_t	 s_Token;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct LogRegisterReply : public BaseMessageHeader
{
    uint32_t	 s_Token;
    uint32_t     s_Result;
    uint8_t      s_MAC[6];
    uint8_t      s_Padding[2];          //fill padding with zero
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct LogUnregister : public BaseMessageHeader
{
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct LogUnregisterReply : public BaseMessageHeader
{
    uint32_t	 s_Token;
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct ConfigLogInfo : public BaseMessageHeader
{
    uint16_t     s_LogType;
    uint16_t     s_TransportMode;
    uint16_t     s_StorageMediaType;
    uint16_t     s_StorageLimitMode;
    uint32_t     s_StorageLimitParameter;
    uint32_t     s_Checksum;             //checksum of other data except this feild
};
struct ConfigLogInfoReply : public BaseMessageHeader
{
    uint32_t	 s_Token;
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct SetupDebugLogInfo : public BaseMessageHeader
{
    boolean_t    s_Enable;
    uint8_t      s_Reserved;
    uint16_t     s_WarningLevel;
    uint32_t     s_BufferingRecordNumber;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};
struct SetupDebugLogInfoReply : public BaseMessageHeader
{
    uint32_t	 s_Token;
    uint32_t     s_Result;
    uint32_t     s_Checksum;			//checksum of other data except this feild
};

struct QueryUserLogInfo : public BaseMessageHeader
{
    uint16_t     s_Type;
    uint16_t     s_SubType;
    uint64_t     s_LogStartTime;
    uint64_t     s_LogEndTime;
    uint32_t     s_Checksum;            //checksum of other data except this feild
};

struct QueryUserLogInfoReply : public BaseMessageHeader
{
    uint32_t	 s_Token;
    uint32_t     s_Result;
    uint32_t     s_TotalLogNumber;
    uint32_t     s_Checksum;            //checksum of other data except this feild
};

struct UserLogReport : public BaseMessageHeader
{
    uint32_t	 s_Token;
    uint32_t     s_LogNumber;
    UserLogInfo  s_Info[1];
    uint32_t     s_Checksum;         //checksum of other data except this feild
};

struct QueryDebugLogInfo : public BaseMessageHeader
{
    uint64_t     s_LogStartTime;
    uint64_t     s_LogEndTime;
    uint32_t     s_Checksum;            //checksum of other data except this feild
};

struct QueryDebugLogInfoReply : public BaseMessageHeader
{
    uint32_t	 s_Token;
    uint32_t     s_Result;
    uint32_t     s_TotalLogNumber;
    uint32_t     s_Checksum;            //checksum of other data except this feild
};

struct DebugLogReport : public BaseMessageHeader
{
    uint32_t	 s_Token;
    uint32_t     s_LogNumber;
    DebugLogInfo s_Info[1];
    uint32_t     s_Checksum;         //checksum of other data except this feild
};

// ------------------------------------------------------------------------ //

#if !defined( __FILE__ )
#define __FILE__ "unknown file"
#endif

#if !defined( __LINE__ )
#define __LINE__ 0
#endif

#if !defined( __FUNCTION__ )
#define __FUNCTION__ "unknown function"
#endif

#endif//GMI_LOG_HEADER
