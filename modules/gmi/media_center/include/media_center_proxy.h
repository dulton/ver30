#pragma once

#include "application_packet.h"
#include "gmi_media_ctrl.h" // avoid user include this file
#include "udp_session.h"

#define MEDIA_CENTER_PROXY_THREADS_SYNC 0

class MediaCenterProxy
{
public:
    MediaCenterProxy(void);
    ~MediaCenterProxy(void);

    GMI_RESULT Initialize  ( long_t ClientIP, uint16_t ClientPort, size_t SessionBufferSize, long_t ServerIP, uint16_t ServerPort );
    GMI_RESULT Deinitialize();

    GMI_RESULT OpenVinVoutDevice     ( uint16_t SensorId, uint16_t ChannelId, FD_HANDLE *VinVoutHandle );
    GMI_RESULT CloseVinVoutDevice    ( FD_HANDLE& VinVoutHandle );
    GMI_RESULT GetVinVoutConfig      ( FD_HANDLE VinVoutHandle, void_t *VinParameter, size_t *VinParameterLength, void_t *VoutParameter, size_t *VoutParameterLength );
    GMI_RESULT SetVinVoutConfig      ( FD_HANDLE VinVoutHandle, const void_t *VinParameter, size_t VinParameterLength, const void_t *VoutParameter, size_t VoutParameterLength );
    GMI_RESULT StartCodec            ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *CodecHandle );
    GMI_RESULT StopCodec             ( FD_HANDLE& CodecHandle );

    GMI_RESULT CreateCodec           ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *CodecHandle );
    GMI_RESULT DestroyCodec          ( FD_HANDLE& CodecHandle );
    GMI_RESULT StartCodec2           ( FD_HANDLE CodecHandle );
    GMI_RESULT StopCodec2            ( FD_HANDLE CodecHandle );
    GMI_RESULT GetCodecConfig        ( FD_HANDLE CodecHandle, void_t *CodecParameter, size_t *CodecParameterLength );
    GMI_RESULT SetCodecConfig        ( FD_HANDLE CodecHandle, const void_t *CodecParameter, size_t CodecParameterLength );
    GMI_RESULT GetOsdConfig          ( FD_HANDLE CodecHandle, void_t *OsdParameter, size_t *OsdParameterLength );
    GMI_RESULT SetOsdConfig          ( FD_HANDLE CodecHandle, const void_t *OsdParameter, size_t OsdParameterLength );
    GMI_RESULT ForceGenerateIdrFrame ( FD_HANDLE CodecHandle );

    GMI_RESULT OpenImageDevice       ( uint16_t SensorId, uint16_t ChannelId, FD_HANDLE *ImageHandle );
    GMI_RESULT CloseImageDevice      ( FD_HANDLE& ImageHandle );
    GMI_RESULT GetBaseImageConfig    ( FD_HANDLE ImageHandle, void_t *ImageParameter, size_t *ImageParameterLength );
    GMI_RESULT SetBaseImageConfig    ( FD_HANDLE ImageHandle, const void_t *ImageParameter, size_t ImageParameterLength );
    GMI_RESULT GetAdvancedImageConfig( FD_HANDLE ImageHandle, void_t *ImageParameter, size_t *ImageParameterLength );
    GMI_RESULT SetAdvancedImageConfig( FD_HANDLE ImageHandle, const void_t *ImageParameter, size_t ImageParameterLength );
    GMI_RESULT GetAutoFocusConfig    ( FD_HANDLE ImageHandle, void_t *AutoFocusParameter, size_t *AutoFocusParameterLength );
    GMI_RESULT SetAutoFocusConfig    ( FD_HANDLE ImageHandle, const void_t *AutoFocusParameter, size_t AutoFocusParameterLength );
    GMI_RESULT GetDaynightConfig     ( FD_HANDLE ImageHandle, void_t *DaynightParameter, size_t *DaynightParameterLength );
    GMI_RESULT SetDaynightConfig     ( FD_HANDLE ImageHandle, const void_t *DaynightParameter, size_t DaynightParameterLength );
    GMI_RESULT GetWhiteBalanceConfig ( FD_HANDLE ImageHandle, void_t *WhiteBalanceParameter, size_t *WhiteBalanceParameterLength );
    GMI_RESULT SetWhiteBalanceConfig ( FD_HANDLE ImageHandle, const void_t *WhiteBalanceParameter, size_t WhiteBalanceParameterLength );

    GMI_RESULT OpenAutoFocusDevice   ( uint16_t SensorId, FD_HANDLE *AutoFocusHandle );
    GMI_RESULT CloseAutoFocusDevice  ( FD_HANDLE& AutoFocusHandle );
    GMI_RESULT StartAutoFocus        ( FD_HANDLE AutoFocusHandle );
    GMI_RESULT StopAutoFocus         ( FD_HANDLE AutoFocusHandle );
    GMI_RESULT PauseAutoFocus        ( FD_HANDLE AutoFocusHandle, int8_t ControlStatus );
    GMI_RESULT AutoFocusGlobalScan   ( FD_HANDLE AutoFocusHandle );
    GMI_RESULT SetAutoFocusMode      ( FD_HANDLE AutoFocusHandle, int32_t AFMode );
    GMI_RESULT NotifyAutoFocus       ( FD_HANDLE AutoFocusHandle, int32_t EventType, uint8_t *ExtData, uint32_t Length );
    GMI_RESULT GetFocusPosition      ( FD_HANDLE AutoFocusHandle, int32_t *FocusPos );
    GMI_RESULT SetFocusPosition      ( FD_HANDLE AutoFocusHandle, int32_t FocusPos );
    GMI_RESULT GetFocusRange         ( FD_HANDLE AutoFocusHandle, int32_t *MinFocusPos, int32_t *MaxFocusPos );
    GMI_RESULT ResetFocusMotor       ( FD_HANDLE AutoFocusHandle );
    GMI_RESULT ControlAutoFocus      ( FD_HANDLE AutoFocusHandle, int8_t AfDirMode );
    GMI_RESULT SetAutoFocusStep      ( FD_HANDLE AutoFocusHandle, int32_t AfStep );

    GMI_RESULT OpenZoomDevice        ( uint16_t SensorId, FD_HANDLE *ZoomHandle );
    GMI_RESULT CloseZoomDevice       ( FD_HANDLE& ZoomHandle );
    GMI_RESULT GetZoomPosition       ( FD_HANDLE ZoomHandle, int32_t *ZoomPos );
    GMI_RESULT SetZoomPosition       ( FD_HANDLE ZoomHandle, int32_t ZoomPos );
    GMI_RESULT GetZoomRange          ( FD_HANDLE ZoomHandle, int32_t *MinZoomPos, int32_t *MaxZoomPos );
    GMI_RESULT ResetZoomMotor        ( FD_HANDLE ZoomHandle );
    GMI_RESULT ControlZoom           ( FD_HANDLE ZoomHandle, int8_t ZoomMode );
    GMI_RESULT SetZoomStep           ( FD_HANDLE ZoomHandle, int32_t ZoomStep );

    GMI_RESULT StartZoom             ( FD_HANDLE AutoFocusHandle, int8_t ControlStatus, FD_HANDLE ZoomHandle, int8_t ZoomMode );
    GMI_RESULT StopZoom              ( FD_HANDLE AutoFocusHandle, int8_t ControlStatus, FD_HANDLE ZoomHandle, int8_t ZoomMode, int32_t *ZoomPos );

private:
    GMI_RESULT Receive      ( long_t Timeout );

    GMI_RESULT OpenDevice1  ( uint16_t MessageId, uint16_t SensorId, uint16_t ChannelId, FD_HANDLE *Handle );
    GMI_RESULT OpenDevice2  ( uint16_t MessageId, uint16_t SensorId, FD_HANDLE *Handle );
    GMI_RESULT CloseDevice  ( FD_HANDLE& Handle, uint16_t MessageId );
    GMI_RESULT GetXXXConfig ( FD_HANDLE Handle, uint16_t MessageId, void_t *Parameter, size_t *ParameterLength );
    GMI_RESULT SetXXXConfig ( FD_HANDLE Handle, uint16_t MessageId, const void_t *Parameter, size_t ParameterLength );

    GMI_RESULT OperationWithoutParameter    ( FD_HANDLE Handle, uint16_t MessageId );
    GMI_RESULT GetOperationWithOneParameter ( FD_HANDLE Handle, uint16_t MessageId, int32_t *Parameter1 );
    GMI_RESULT GetOperationWithTwoParameter ( FD_HANDLE Handle, uint16_t MessageId, int32_t *Parameter1, int32_t *Parameter2 );
    GMI_RESULT SetOperationWithOneParameter ( FD_HANDLE Handle, uint16_t MessageId, int32_t Parameter1 );

private:
#if MEDIA_CENTER_PROXY_THREADS_SYNC
    GMI_Mutex                                                           m_OperationLock;
#endif
    long_t                                                              m_Client_UDP_IP;
    uint16_t                                                            m_Client_UDP_Port;
    size_t                                                              m_SessionBufferSize;
    long_t                                                              m_Server_UDP_IP;
    uint16_t                                                            m_Server_UDP_Port;
    ReferrencePtr<GMI_Socket>                                           m_UDP_Socket;
    ReferrencePtr<UDPSession,DefaultObjectDeleter,MultipleThreadModel>  m_UDP_Session;
    uint16_t                                                            m_SequenceNumber;
    ApplicationPacket                                                   m_MediaCenterProxyRequestPacket;
    ApplicationPacket                                                   m_MediaCenterProxyReplyPacket;
};
