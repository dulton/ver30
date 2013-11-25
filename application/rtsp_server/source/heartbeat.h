#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include <gmi_daemon_heartbeat_api.h>

#include "common_def.h"

class HeartBeat : public Instance<HeartBeat>
{
public:
    HeartBeat();
    ~HeartBeat();

    GMI_RESULT Initialize();
    GMI_RESULT Uninitialize();

    inline boolean_t Initialized() const { return m_Initialized; }

    GMI_RESULT Report(boolean_t & NeedQuit);

    GMI_RESULT RebootSystem();

private:
    DaemonData_t m_DaemonData;
    boolean_t    m_Initialized;
};

#endif // __HEARTBEAT_H__

