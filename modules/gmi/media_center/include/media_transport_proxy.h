#pragma once

#include "gmi_system_headers.h"

#if !defined( RTSP_EVENT_PROCESS_DEF )
#define RTSP_EVENT_PROCESS_DEF 1
enum RTSP_EventType
{
    RTSP_ET_OPTIONS       = 1,
    RTSP_ET_DESCRIBE      = 2,
    RTSP_ET_SETUP         = 3,
    RTSP_ET_PLAY          = 4,
    RTSP_ET_PAUSE         = 5,
    RTSP_ET_GET_PARAMETER = 6,
    RTSP_ET_SET_PARAMETER = 7,
    RTSP_ET_TEARDOWN      = 8
};

typedef void_t (*RTSP_EVENT_CALLBACK)( void_t *UserData, uint32_t EventType, ReferrencePtr<uint8_t,DefaultObjectsDeleter>& EventData, size_t EventDataLength );
#endif//RTSP_EVENT_PROCESS_DEF

class GMI_RtspServer;

class MediaTransportProxy
{
public:
    MediaTransportProxy(void);
    ~MediaTransportProxy(void);

    GMI_RESULT Initialize( uint16_t ServerPort );
    GMI_RESULT Deinitialize();

    GMI_RESULT Start( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *TransportHandle );
    GMI_RESULT Stop ( FD_HANDLE TransportHandle );

    void_t SetCallback( RTSP_EVENT_CALLBACK Callback, void_t *UserData );

private:
    SafePtr<GMI_RtspServer> m_RtspServer;
};
