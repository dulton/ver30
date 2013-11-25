#pragma once

#include "gmi_system_headers.h"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "H264VideoFileServerMediaSubsession.hh"
#include "liveMedia.hh"
#include "MyH264VideoStreamFramer.hh"
#include "MyH264VideoRTPSink.hh"
#include "MyRTSPServer.hh"

class GMI_RtspServer
{
public:
    GMI_RtspServer(void);
    ~GMI_RtspServer(void);

    GMI_RESULT Initialize  ( uint16_t RTSP_Port );
    GMI_RESULT Deinitialize();

    GMI_RESULT Add   ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *TransportHandle );
    GMI_RESULT Remove( FD_HANDLE Transport );
    GMI_RESULT Start ();
    GMI_RESULT Stop  ();

    void_t SetCallback( RTSP_EVENT_CALLBACK Callback, void_t *UserData );

private:
    static void_t* RtspThreadEntry( void_t *Parameter );
    void_t* RtspEventLoop();

    GMI_RESULT GetServerMulticastAddress( int32_t StreamId, uint32_t *MulticastAddress );
    GMI_RESULT GetServer_RTP_UDP_Port( int32_t StreamId, uint16_t *UDP_Port );
    GMI_RESULT GetServer_RTCP_UDP_Port( int32_t StreamId, uint16_t *UDP_Port );
    GMI_RESULT GetClient_RTP_UDP_Port( int32_t StreamId, uint16_t *UDP_Port );
    GMI_RESULT GetClient_RTCP_UDP_Port( int32_t StreamId, uint16_t *UDP_Port );

    GMI_RESULT GetIPCMediaDataDispatchServerPort( int32_t StreamId, uint16_t *UDP_Port );
    GMI_RESULT GetIPCMediaDataDispatchClientPort( int32_t StreamId, uint16_t *UDP_Port );

private:
    TaskScheduler	            *m_Scheduler;
    UsageEnvironment            *m_Environment;
    UserAuthenticationDatabase  *m_AuthenticationDB;
    MyRTSPServer                *m_RtspServer;
    GMI_Thread                  m_RtspEventLoopThread;
    char_t                      m_RtspEventLoopThreadExitFlag;
    boolean_t                   m_RtspEventLoopThreadWorking;

    struct StreamInfo
    {
        TaskScheduler           *s_Scheduler;
        UsageEnvironment        *s_Environment;
        ServerMediaSession      *s_MediaSession;
    };
    std::vector<StreamInfo*>    m_Streams;
};
