#ifndef __SYS_CLOSE_UNIX_TCP_CMD_H__
#define __SYS_CLOSE_UNIX_TCP_CMD_H__
#include "unix_tcp_session.h"
#include "gmi_system_headers.h"
#include "base_command_executor.h"

class SysCloseUnixTcpCommandExecutor : public BaseCommandExecutor
{
public:
    SysCloseUnixTcpCommandExecutor(void);
    virtual ~SysCloseUnixTcpCommandExecutor(void);
    GMI_RESULT SetParameter(ReferrencePtr<UnixTcpSession, DefaultObjectDeleter, MultipleThreadModel> UnixTcpSessionPtr, void_t *Argument, size_t ArgumentSize);
    virtual GMI_RESULT	Create(ReferrencePtr<BasePacket>& Packet, SafePtr<BaseCommandExecutor>& CommandExecutor );
    virtual GMI_RESULT  Execute();
    virtual GMI_RESULT  Reply();
protected:
    ReferrencePtr<UnixTcpSession, DefaultObjectDeleter, MultipleThreadModel> m_UnixTcpSession;
    void_t *m_Argument;
    size_t  m_ArgumentSize;
};

#endif

