#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include <gmi_daemon_heartbeat_api.h>

#include "common_def.h"

class HeartBeat : public Instance<HeartBeat>
{
friend class Instance<HeartBeat>;

public:
    GMI_RESULT Initialize();
    GMI_RESULT Uninitialize();

    GMI_RESULT Report(boolean_t & NeedQuit);

    GMI_RESULT RebootSystem();

    inline boolean_t Initialized() const { return m_Initialized; }

private:
    HeartBeat();
    ~HeartBeat();

    DaemonData_t m_DaemonData;
    boolean_t    m_Initialized;
};

#endif // __HEARTBEAT_H__

