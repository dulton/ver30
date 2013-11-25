#include "sys_close_unix_tcp_cmd.h"
#include "log.h"
#include "system_packet.h"


SysCloseUnixTcpCommandExecutor::SysCloseUnixTcpCommandExecutor(void)
    : BaseCommandExecutor(0, CT_ShortTask )
{
}


SysCloseUnixTcpCommandExecutor::~SysCloseUnixTcpCommandExecutor(void)
{
}


GMI_RESULT SysCloseUnixTcpCommandExecutor::SetParameter( ReferrencePtr<UnixTcpSession, DefaultObjectDeleter, MultipleThreadModel> UnixTcpSessionPtr, void_t *Argument, size_t ArgumentSize)
{
    if ( UnixTcpSessionPtr.GetPtr() == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    m_UnixTcpSession = UnixTcpSessionPtr;
    m_Argument = Argument;
    m_ArgumentSize = ArgumentSize;

    return GMI_SUCCESS;
}


GMI_RESULT SysCloseUnixTcpCommandExecutor::Create( ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor )
{
    return GMI_SUCCESS;
}


GMI_RESULT  SysCloseUnixTcpCommandExecutor::Execute()
{
    return GMI_SUCCESS;
}


GMI_RESULT  SysCloseUnixTcpCommandExecutor::Reply()
{
    int32_t SocketFd = m_UnixTcpSession->GetFDHandle();
    close(SocketFd);
    SYS_INFO("close unix tcp socket %d\n", SocketFd);
    return GMI_SUCCESS;
}



