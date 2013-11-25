#include "sys_base_command_executor.h"
#include "gmi_system_headers.h"


SysBaseCommandExecutor::SysBaseCommandExecutor( uint32_t CommandId, enum CommandType Type )
    : BaseCommandExecutor( CommandId, Type )
{
}


SysBaseCommandExecutor::~SysBaseCommandExecutor()
{
}


GMI_RESULT SysBaseCommandExecutor::SetParameter( ReferrencePtr<SystemServiceManager> SystemServiceManagerPtr, void_t *Argument, size_t ArgumentSize)
{
    //GMI_RESULT Result = GMI_SUCCESS;

    if ( SystemServiceManagerPtr.GetPtr() == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    m_SystemServiceManager = SystemServiceManagerPtr;
    m_Argument = Argument;
    m_ArgumentSize = ArgumentSize;

    return GMI_SUCCESS;
}


