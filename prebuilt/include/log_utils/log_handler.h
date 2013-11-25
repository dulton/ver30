// log_handler.h

#if !defined( GMI_LOG_HANDLER )
#define GMI_LOG_HANDLER

#include "gmi_system_headers.h"
#include "log_header.h"

class GMI_LogHandler
{
public:
    GMI_LogHandler() : m_ProcessUserLog( false ), m_ProcessDebugLog( false ) {}
    virtual ~GMI_LogHandler() {}

    virtual GMI_RESULT  Initialize( const void_t *Parameter, size_t ParameterLength ) = 0;
    virtual GMI_RESULT  Deinitialize() = 0;
    virtual GMI_RESULT  UserLog( uint16_t Type, uint16_t Subtype, uint64_t LogTime, const char_t *UserName, uint32_t UserNameLength, const void_t *SpecificData, uint32_t SpecificDataLength ) = 0;
    virtual GMI_RESULT  DebugLog( uint16_t Level, uint64_t LogTime, uint64_t ProcessId, uint64_t ThreadId, uint32_t ModuleId, const char_t *ModuleName, const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const void_t *SpecificData, uint32_t SpecificDataLength ) = 0;
    virtual boolean_t   IsPrinter()
    {
        return false;
    }
    virtual boolean_t   IsPublisher()
    {
        return false;
    }
    virtual boolean_t   IsRepository()
    {
        return false;
    }

    void_t  ProcessUserLog( boolean_t Enable )
    {
        m_ProcessUserLog = Enable;
    }

    void_t  ProcessDebugLog( boolean_t Enable )
    {
        m_ProcessDebugLog = Enable;
    }

protected:
    boolean_t                     m_ProcessUserLog;
    boolean_t                     m_ProcessDebugLog;
};

#endif//GMI_LOG_HANDLER
