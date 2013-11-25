#pragma once

#include "gmi_system_headers.h"
#include "log_handler.h"

class LogPublisher : public GMI_LogHandler
{
public:
    LogPublisher(void);
    virtual ~LogPublisher(void);

    virtual GMI_RESULT  Initialize( const void_t *Parameter, size_t ParameterLength );
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT  UserLog( uint16_t Type, uint16_t Subtype, uint64_t LogTime, const char_t *UserName, uint32_t UserNameLength, const void_t *SpecificData, uint32_t SpecificDataLength );
    virtual GMI_RESULT  DebugLog( uint16_t Level, uint64_t LogTime, uint64_t ProcessId, uint64_t ThreadId, uint32_t ModuleId, const char_t *ModuleName, const char_t *FileName, const char_t *FunctionName, uint32_t LineNumber, const void_t *SpecificData, uint32_t SpecificDataLength );
    virtual boolean_t   IsPublisher()
    {
        return true;
    }
};
