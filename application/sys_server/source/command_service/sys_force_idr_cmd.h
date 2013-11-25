#ifndef __SYS_FORCE_IDR_CMD_H__
#define __SYS_FORCE_IDR_CMD_H__

#include "sys_base_command_executor.h"


class SysForceIdrCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysForceIdrCommandExecutor(void);
    virtual ~SysForceIdrCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
private:
    int32_t m_StreamId;
};


#endif




