#ifndef __FILE_LOCK_H__
#define __FILE_LOCK_H__

typedef enum {
    ERROR_LEVEL = 0,
    WARNING_LEVEL,
    INFO_LEVEL,
} DEBUG_LEVEL;

static int g_log_level = ERROR_LEVEL;

#define PREFIX_NONE   "\033[0m"
#define PREFIX_RED    "\033[0;31m"
#define PREFIX_GREEN  "\033[0;32m"
#define PREFIX_YELLOW "\033[1;33m"

#define PRINT(mylog, LOG_LEVEL, format, args...)    do { if (mylog >= LOG_LEVEL) {printf(format, ##args); fflush(stdout);}} while (0)

#define APP_ERROR(format, args...)   PRINT(g_log_level, ERROR_LEVEL,      PREFIX_RED"Error   [%-20s][%05d][%-20s]: " format PREFIX_NONE"\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#define APP_WARNING(format, args...) PRINT(g_log_level, WARNING_LEVEL, PREFIX_YELLOW"Warning [%-20s][%05d][%-20s]: " format PREFIX_NONE"\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#define APP_INFO(format, args...)    PRINT(g_log_level, INFO_LEVEL,     PREFIX_GREEN"Info    [%-20s][%05d][%-20s]: " format PREFIX_NONE"\n", __FILE__, __LINE__, __FUNCTION__, ##args)

class CSemaphoreMutex
{
public:
    CSemaphoreMutex();
    ~CSemaphoreMutex();

    bool Create(long key);
    void Destroy();

    void Lock() const;
    void Unlock() const;
    bool TryLock() const;
    bool IsLocked() const;

private:
    bool m_created;
    int m_id;
};

#endif



