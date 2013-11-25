#ifndef __SYS_GET_SYSCFG_COMMAND_H__
#define __SYS_GET_SYSCFG_COMMAND_H__

//#include "../../../../../module/gmi/command_pipeline/include/base_command_executor.h"
#include "sys_base_command_executor.h"

class SysGetSysCfgCommandExecutor : public SysBaseCommandExecutor
{
public:
    SysGetSysCfgCommandExecutor(void);
    virtual ~SysGetSysCfgCommandExecutor(void);

    virtual GMI_RESULT	Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();

private:
    static  GMI_RESULT  GetPeerPipeName( char_t *PeerPipeName, long_t PeerPipeNameBufferLength, const char_t *ModulePipeName );
    static  GMI_RESULT  GetPeerPipeMutexId( long_t *PeerPipeMutexId, long_t ModulePipeMutexId );
};

#endif

