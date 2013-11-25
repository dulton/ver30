#pragma once

#include "application_packet.h"
#include "gmi_system_headers.h"

class IpcMediaDataUDPSession;

class IPC_MediaDataServer4
{
public:
    IPC_MediaDataServer4(void);
    ~IPC_MediaDataServer4(void);

    GMI_RESULT Initialize  ( uint16_t Server_UDP_Port, uint32_t MediaType, uint32_t MediaId, long_t ShareMemoryKey, size_t ShareMemorySize, size_t MaxDataBlockSize, size_t MinDataBlockSize );
    GMI_RESULT Deinitialize();

    GMI_RESULT Write( const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );
    GMI_RESULT Read ( void_t *Frame, size_t *FrameSize, struct timeval *FrameTS, void_t *ExtraData, size_t *ExtraDataLength );

private:
    static void_t* SessionProcessEntry ( void_t *Argument );
    void_t*        SessionProcess      ();

    GMI_RESULT     NotifySesssion      ( size_t Offset, size_t Length );
    GMI_RESULT     ProcessSession      ( uint16_t Client_UDP_Port, uint8_t *Buffer, size_t BufferLength );
    GMI_RESULT     ProcessCommand      ( ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel>& Session  );

    GMI_RESULT     OnRegister          ( ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel>& Session, uint16_t SequenceNumber, uint32_t ApplicationId, uint32_t MediaType, uint32_t MediaId, boolean_t PullMode );
    GMI_RESULT     OnRegisterReply     ( ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel>& Session, uint16_t SequenceNumber, GMI_RESULT RegisterResult, long_t ShareMemoryKey, size_t ShareMemorySize );
    GMI_RESULT     OnUnregister        ( ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel>& Session, uint16_t SequenceNumber, uint32_t ApplicationId );
    GMI_RESULT     OnUnregisterReply   ( ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel>& Session, uint16_t SequenceNumber, GMI_RESULT UnregisterResult );
    GMI_RESULT     OnNextBlock         ( ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel>& Session, uint16_t SequenceNumber );
    GMI_RESULT     OnNextBlockReply    ( ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel>& Session, uint16_t SequenceNumber, GMI_RESULT NextResult, size_t Offset, size_t Length, uint32_t Flag );
    GMI_RESULT     OnReleaseBlock      ( ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel>& Session, uint16_t SequenceNumber );
    GMI_RESULT     OnReleaseBlockReply ( ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel>& Session, uint16_t SequenceNumber, GMI_RESULT ReleaseResult );

private:
    ReferrencePtr<GMI_Socket>                                                                      m_ServerSocket;
    ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel>                 m_UDP_Session;
    GMI_Mutex                                                                                      m_SessionLock;
    std::vector< ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel> >  m_ClientSessions;
    std::vector< ReferrencePtr<IpcMediaDataUDPSession,DefaultObjectDeleter,MultipleThreadModel> >  m_ClientApplications;
    uint32_t                                                                                       m_MediaType;
    uint32_t                                                                                       m_MediaId;
    long_t                                                                                         m_ShareMemoryKey;
    GMI_ShareMemory                                                                                m_ShareMemory;
    void_t                                                                                         *m_MemoryPool;
    GMI_Thread                                                                                     m_ReleaseProcessThread;
    boolean_t                                                                                      m_ReleaseProcessThreadWorking;
    boolean_t                                                                                      m_ReleaseProcessThreadExitFlag;
};
