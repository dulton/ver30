#ifndef __SYS_GET_IP_INFO_CMD_H__
#define __SYS_GET_IP_INFO_CMD_H__

#include "sys_base_command_executor.h"


class SysGetIpInfoCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetIpInfoCommandExecutor(void);
    virtual ~SysGetIpInfoCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
private:
    static	GMI_RESULT	GetPeerPipeName( char_t *PeerPipeName, long_t PeerPipeNameBufferLength, const char_t *ModulePipeName );
    static	GMI_RESULT	GetPeerPipeMutexId( long_t *PeerPipeMutexId, long_t ModulePipeMutexId );

};


#endif


