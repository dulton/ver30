#include <unistd.h>
#include <string.h>
#include <time.h>

#include "log_module.h"

#define DISP_BLACK             "\033[30m"
#define DISP_RED               "\033[31m"
#define DISP_GREEN             "\033[32m"
#define DISP_YELLOW            "\033[33m"
#define DISP_BLUE              "\033[34m"
#define DISP_PURPLE            "\033[35m"
#define DISP_LIGHT_BLUE        "\033[36m"
#define DISP_WHITE             "\033[37m"

#define DISP_BACK_BLACK        "\033[40m"
#define DISP_BACK_RED          "\033[41m"
#define DISP_BACK_GREEN        "\033[42m"
#define DISP_BACK_YELLOW       "\033[43m"
#define DISP_BACK_BLUE         "\033[44m"
#define DISP_BACK_PURPLE       "\033[45m"
#define DISP_BACK_LIGHT_BLUE   "\033[46m"
#define DISP_BACK_WHITE        "\033[47m"

#define DISP_RESET             "\033[0m"

// (sizeof(DISP_xxx) - 1)
#define PREFIX_SIZE            5
// (sizeof(DISP_RESET) - 1)
#define SUFFIX_SIZE            4

// Get the level color prefix string
static inline const char * Level2Color(LogModule::LogLevel Level)
{
    switch (Level)
    {
        case LogModule::eEmergency:
            return DISP_BACK_RED;

        case LogModule::eAlert:
            return DISP_BACK_PURPLE;

        case LogModule::eCritical:
            return DISP_RED;

        case LogModule::eError:
            return DISP_PURPLE;

        case LogModule::eWarning:
            return DISP_YELLOW;

        case LogModule::eInfo:
            return DISP_GREEN;

        case LogModule::eVerbose:
            return DISP_BLUE;

        case LogModule::eDebug:
            return DISP_LIGHT_BLUE;

        case LogModule::eNone:
        default:
            break;
    }

    return DISP_RESET;
}

// Get the level string
static inline const char * Level2String(LogModule::LogLevel Level)
{
    switch (Level)
    {
        case LogModule::eEmergency:
            return "Emergency";

        case LogModule::eAlert:
            return "Alert";

        case LogModule::eCritical:
            return "Critical";

        case LogModule::eError:
            return "Error";

        case LogModule::eWarning:
            return "Warning";

        case LogModule::eInfo:
            return "Info";

        case LogModule::eVerbose:
            return "Verbose";

        case LogModule::eDebug:
            return "Debug";

        case LogModule::eNone:
        default:
            break;
    }

    return "Unknown";
}

// Help to get the file name without path
static inline const char * CutFilePath(const char * File)
{
    const char * p = strrchr(File, '/');
    if (p != NULL)
    {
        return p + 1;
    }

    return File;
}

LogModule & LogModule::Instance()
{
    static LogModule Module;
    return Module;
}

LogModule::LogModule()
    : m_LogFile(NULL)
    , m_LogFileMaxLength(0)
    , m_LogWriteLevel(eInfo)
    , m_LogDispLevel(eInfo)
    , m_Updating(false)
{
}

LogModule::~LogModule()
{
    // Close log file
    if (m_LogFile != NULL)
    {
        fclose(m_LogFile);
        m_LogFile = NULL;
    }
}

void LogModule::Print(LogLevel Level, const char * File, unsigned int Line, const char * Fmt, ...)
{
    // Check if we need to precess this log
    if ((Level > m_LogWriteLevel && Level > m_LogDispLevel) || Level == eNone)
    {
        // Nothing to do
        return;
    }

    va_list     Args;
    time_t      Now   = time(NULL);
    struct tm * LTime = localtime(&Now);
    char        FmtBuf[256];

    va_start(Args, Fmt);

    // Make the format string
    snprintf(FmtBuf + PREFIX_SIZE, sizeof(FmtBuf) - PREFIX_SIZE - SUFFIX_SIZE, "%4d-%02d-%02d %02d:%02d:%02d %9s %20s line %05d: %s\n",
        LTime->tm_year + 1900, LTime->tm_mon + 1, LTime->tm_mday, LTime->tm_hour, LTime->tm_min, LTime->tm_sec,
        Level2String(Level), CutFilePath(File), Line, Fmt);

    // Check if we need to write this log to log file
    if (m_LogFile != NULL && Level <= m_LogWriteLevel)
    {
        // Wrtie this log into file
        vfprintf(m_LogFile, FmtBuf + PREFIX_SIZE, Args);

        // Ensure length of log file is less than m_LogFileMaxLength
        CheckFileLength();
    }

    // Check if we need to print this log
    if (Level <= m_LogDispLevel)
    {
        // Prepare the color prefix and suffix
        memcpy(FmtBuf, Level2Color(Level), PREFIX_SIZE);
        memcpy(FmtBuf + strlen(FmtBuf) - 1, DISP_RESET"\n", SUFFIX_SIZE + 2);

        // Print this log to stderr
        vfprintf(stderr, FmtBuf, Args);
    }

    va_end(Args);
}

void LogModule::DumpBuffer(LogLevel Level, const char * File, unsigned int Line, const char * Variable, const void * Buf, unsigned int BufLength)
{
    // Check if we need to precess this log
    if ((Level > m_LogWriteLevel && Level > m_LogDispLevel) || Level == eNone)
    {
        // Nothing to do
        return;
    }

    Print(Level, File, Line, "%s =", Variable);

    const unsigned char * Ch = reinterpret_cast<const unsigned char *>(Buf);
    while (BufLength > 0)
    {
        char BufString[128];
        for (unsigned int i = 0; i < 16 && BufLength > 0; ++ i)
        {
            snprintf(BufString + (i * 5), 7, "0x%02x \n", *Ch);
            Ch ++;
            BufLength --;
        }

        // Check if we need to write this log to log file
        if (m_LogFile != NULL && Level <= m_LogWriteLevel)
        {
            // Wrtie this log into file
            fprintf(m_LogFile, "%s", BufString);

            // Ensure length of log file is less than m_LogFileMaxLength
            CheckFileLength();
        }

        // Check if we need to print this log
        if (Level <= m_LogDispLevel)
        {
            // Print this log to stderr
            fprintf(stderr, "%s%s"DISP_RESET, Level2Color(Level), BufString);
        }
    }
}

void LogModule::Initialize(const char * LogFile, unsigned int MaxLength)
{
    // Check parameter
    if (NULL == LogFile || 0 == MaxLength)
    {
        // Invalid parameter, do nothing
        return;
    }

    // Close old log file
    if (m_LogFile != NULL)
    {
        fclose(m_LogFile);
        m_LogFile = NULL;
    }

    // Open log file with append mode
    m_LogFile = fopen(LogFile, "a");
    if (NULL == m_LogFile)
    {
        // Failed to open log file, do nothing
        return;
    }

    m_LogFileMaxLength = MaxLength;
    snprintf(m_LogFileName, sizeof(m_LogFileName), "%s", LogFile);
}

void LogModule::CheckFileLength()
{
    // Get the file length of the log file
    unsigned int FileLength = ftell(m_LogFile);

    // Check if the file size if out of range
    if (FileLength >= m_LogFileMaxLength)
    {
        char BackUpFileName[132];

        // Prepare the backup log file name
        snprintf(BackUpFileName, sizeof(BackUpFileName), "%s.bak", m_LogFileName);

        // Remove the backup file if it exists
        unlink(BackUpFileName);

        // Rename the log file to backup file name
        if (m_Updating || rename(m_LogFileName, BackUpFileName) < 0)
        {
            // Log file is already updated
            return;
        }

        m_Updating = true;

        // Open log file with append mode again
        FILE * NewLogFile = fopen(m_LogFileName, "w");

        FILE * OldLogFile = m_LogFile;
        m_LogFile = NewLogFile;

        // Close the old log file
        fclose(OldLogFile);

        m_Updating = false;
    }
}

