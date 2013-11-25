#ifndef __LOG_MODULE_H__
#define __LOG_MODULE_H__

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

// Define log level
#define LOG_LEVEL_EMERGENCY LogModule::eEmergency
#define LOG_LEVEL_ALERT     LogModule::eAlert
#define LOG_LEVEL_CRITICAL  LogModule::eCritical
#define LOG_LEVEL_ERROR     LogModule::eError
#define LOG_LEVEL_WARNING   LogModule::eWarning
#define LOG_LEVEL_INFO      LogModule::eInfo
#define LOG_LEVEL_VERBOSE   LogModule::eVerbose
#define LOG_LEVEL_DEBUG     LogModule::eDebug

// Define log level assert equal to emergency
#define LOG_LEVEL_ASSERT    LOG_LEVEL_EMERGENCY

// Define log module control interface
#define LOG_INITIALIZE(file, max)           LogModule::Instance().Initialize(file, max)
#define LOG_DISABLE_WRITE()                 LogModule::Instance().SetWriteLevel(LogModule::eNone)
#define LOG_DISABLE_DISPLAY()               LogModule::Instance().SetDisplayLevel(LogModule::eNone)
#define LOG_SET_WRITE_LEVEL(level)          LogModule::Instance().SetWriteLevel(LOG_LEVEL_##level)
#define LOG_SET_DISPLAY_LEVEL(level)        LogModule::Instance().SetDisplayLevel(LOG_LEVEL_##level)

// Define basic log interface
#define PRINT_LOG(level, ...)               LogModule::Instance().Print(LOG_LEVEL_##level, __FILE__, __LINE__, __VA_ARGS__)
#define ASSERT(cond, ...)                   do { if (cond) break; PRINT_LOG(ASSERT, __VA_ARGS__); HALT_PROCESS(); } while(0)

// Define halt function
#define HALT_PROCESS()                      do { raise(SIGKILL); } while(0)

// Define assert
#define ASSERT_NULL_POINT(ptr)              ASSERT((ptr) == NULL, #ptr" MUST be non-pointer");
#define ASSERT_VALID_POINT(ptr)             ASSERT((ptr) != NULL, #ptr" MUST NOT be non-pointer");
#define ASSERT_EQUAL(var1, var2)            ASSERT((var1) == (var2), #var1" MUST equal to "#var2);
#define ASSERT_NOT_EQUAL(var1, var2)        ASSERT((var1) != (var2), #var1" MUST not equal to "#var2);
#define ASSERT_GREATER(var1, var2)          ASSERT((var1) > (var2), #var1" MUST greater than "#var2);
#define ASSERT_GREATER_EQUAL(var1, var2)    ASSERT((var1) >= (var2), #var1" MUST greater than or equal to "#var2);
#define ASSERT_LESS(var1, var2)             ASSERT((var1) < (var2), #var1" MUST less than to"#var2);
#define ASSERT_LESS_EQUAL(var1, var2)       ASSERT((var1) <= (var2), #var1" MUST less than or equal to "#var2);

// Define variable dump function
#define DUMP_VARIABLE(var)                  DumpVariable(LOG_LEVEL_DEBUG, #var, var, __FILE__, __LINE__);
#define DUMP_BUFFER(ptr, length)            LogModule::Instance().DumpBuffer(LOG_LEVEL_DEBUG, __FILE__, __LINE__, #ptr, ptr, length);

class LogModule
{
public:
    static LogModule & Instance();

    typedef enum tagLogLevel
    {
        eNone = 0,

        eEmergency,
        eAlert,
        eCritical,
        eError,
        eWarning,
        eInfo,
        eVerbose,
        eDebug,
    } LogLevel;

    inline void SetWriteLevel(LogLevel Level) { m_LogWriteLevel = Level; }
    inline LogLevel GetWriteLevel() const { return m_LogWriteLevel; }

    inline void SetDisplayLevel(LogLevel Level) { m_LogDispLevel = Level; }
    inline LogLevel GetDisplayLevel() const { return m_LogDispLevel; }

    void Initialize(const char * LogFile, unsigned int MaxLength);

    void Print(LogLevel Level, const char * File, unsigned int Line, const char * Fmt, ...);

    void DumpBuffer(LogLevel Level, const char * File, unsigned int Line, const char * Variable, const void * Buf, unsigned int BufLength);

private:
    LogModule();
    ~LogModule();

    void CheckFileLength();

    char           m_LogFileName[128];
    FILE         * m_LogFile;
    unsigned int   m_LogFileMaxLength;
    LogLevel       m_LogWriteLevel;
    LogLevel       m_LogDispLevel;
    bool           m_Updating;
};

// Define variable dump functions for all basic variable type
inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, char Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = DEC(%d), HEX(0x%02x)", Variable, Value, static_cast<unsigned int>(Value));
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, short Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = DEC(%d), HEX(0x%04x)", Variable, Value, static_cast<unsigned int>(Value));
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, int Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = DEC(%d), HEX(0x%08x)", Variable, Value, static_cast<unsigned int>(Value));
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, long Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = DEC(%ld), HEX(0x%08lx)", Variable, Value, static_cast<unsigned int>(Value));
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, long long Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = DEC(%lld), HEX(0x%016llx)", Variable, Value, static_cast<unsigned int>(Value));
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, unsigned char Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = DEC(%u), HEX(0x%02x)", Variable, Value, Value);
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, unsigned short Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = DEC(%u), HEX(0x%04x)", Variable, Value, Value);
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, unsigned int Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = DEC(%u), HEX(0x%08x)", Variable, Value, Value);
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, unsigned long Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = DEC(%lu), HEX(0x%08lx)", Variable, Value, Value);
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, unsigned long long Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = DEC(%llu), HEX(0x%016llx)", Variable, Value, Value);
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, bool Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = %s", Variable, Value ? "true" : "false");
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, const char * Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = %s", Variable, Value);
}

inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, char * Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = %s", Variable, Value);
}

template <typename T>
inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, T * Value, const char * File, unsigned int Line)
{
    LogModule::Instance().Print(Level, File, Line, "%s = %p", Variable, Value);
}

template <typename T>
inline void DumpVariable(LogModule::LogLevel Level, const char * Variable, const T & Value, const char * File, unsigned int Line)
{
    LogModule::Instance().DumpBuffer(Level, File, Line, Variable, &Value, sizeof(Value));
}

#endif // __LOG_MODULE_H__

