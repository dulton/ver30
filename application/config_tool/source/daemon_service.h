#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include <gmi_daemon_heartbeat_api.h>

#include "common_def.h"

class DaemonService : public Instance<DaemonService>
{
friend class Instance<DaemonService>;

public:
    typedef enum tagSystemStatus
    {
        eOffLine = 0,
        eOnLine,
        ePreparing,
        eUnexpect,
    } SystemStatus;

    static inline const char_t * SystemStatus2String(SystemStatus Status)
    {
        switch (Status)
        {
        case eOffLine:   return "OffLine";
        case eOnLine:    return "OnLine";
        case ePreparing: return "Preparing";
        case eUnexpect:  return "Unexpect";
        }

        return "Unknown";
    }

    GMI_RESULT Initialize();
    GMI_RESULT Uninitialize();

    GMI_RESULT Report(boolean_t & NeedQuit);

    GMI_RESULT RebootSystem();

    GMI_RESULT QuerySystemStatus(SystemStatus & Status);

    inline boolean_t Initialized() const { return m_Initialized; }

private:
    DaemonService();
    ~DaemonService();

    DaemonData_t m_DaemonData;
    boolean_t    m_Initialized;
};

#endif // __HEARTBEAT_H__

