#ifndef __STREAM_CENTER_CLIENT_H__
#define __STREAM_CENTER_CLIENT_H__

#include "gmi_system_headers.h"
//#include "media_center_proxy.h"
#include "media_transport_proxy.h"
#include "gmi_media_ctrl.h"
#include "media_center.h"

typedef struct tagMediaHandle
{
    uint32_t  s_SourceId;
    uint32_t  s_StreamId;
    uint32_t  s_MediaType;
    uint32_t  s_CodecType;
    boolean_t s_EncMode;//true-encode, false-decode
    FD_HANDLE s_Encode;
    FD_HANDLE s_Transport;
} MediaHandle;

//Zoom & Focus Cmd
typedef struct
{
	int8_t   s_ZoomMode;
	int32_t  s_ZoomStep;
}ZoomCmd;

class StreamCenterClient
{
public:
    StreamCenterClient();
    virtual ~StreamCenterClient();
    GMI_RESULT Initialize(uint16_t LocalClientPort, uint16_t LocalServerPort, uint16_t RTSP_ServerPort, size_t SessionBufferSize);
    GMI_RESULT Start(uint32_t SourceId, uint32_t StreamId, uint32_t MediaType, uint32_t CodeType, boolean_t EncMode, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *Handle);
    GMI_RESULT Stop(FD_HANDLE Handle);
    GMI_RESULT Create(uint32_t SourceId, uint32_t StreamId, uint32_t MediaType, uint32_t CodeType, boolean_t EncMode, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *Handle);
    GMI_RESULT Destroy(FD_HANDLE Handle);
    GMI_RESULT Start2(FD_HANDLE Handle);
    GMI_RESULT Stop2(FD_HANDLE Handle);
    GMI_RESULT Deinitialize();
    GMI_RESULT GeneralParamSet(General_Param ParamType, void_t *GeneralParamPtr);
    
    GMI_RESULT OpenVideoInOutDevice(uint32_t VidSourceId, uint32_t VidChanId, FD_HANDLE *Handle);
    GMI_RESULT CloseVideoInOutDevice(FD_HANDLE Handle);
    GMI_RESULT OpenImageDevice(uint32_t VidSourceId, uint32_t VidChanId, FD_HANDLE *Handle);
    GMI_RESULT CloseImageDevice(FD_HANDLE Handle);
    GMI_RESULT GetCodecConfiguration(FD_HANDLE Handle, void_t *CodecParameter, size_t CodecParameterLength);
    GMI_RESULT SetCodecConfiguration(FD_HANDLE Handle, void_t *CodecParameter, size_t CodecParameterLength);
    GMI_RESULT SetOsdConfiguration(FD_HANDLE Handle,  VideoOSDParam *OsdParamPtr);
    GMI_RESULT ForceGenerateIdrFrame( FD_HANDLE Handle);
    GMI_RESULT SetImageConfiguration(FD_HANDLE Handle, ImageBaseParam *ImageParamPtr);
    GMI_RESULT SetImageAdvanceConfiguration(FD_HANDLE Handle, ImageAdvanceParam *ImageAdvanceParamPtr);
    GMI_RESULT SetVinVoutConfiguration(FD_HANDLE Handle, VideoInParam *VidInParamPtr, VideoOutParam *VidOutParamPtr);
    GMI_RESULT GetVinVoutConfiguration(FD_HANDLE Handle, VideoInParam *VidInParamPtr, VideoOutParam *VidOutParamPtr);
    GMI_RESULT SetDaynightConfiguration(FD_HANDLE Handle, ImageDnParam *ImageDnParamPtr);
    GMI_RESULT GetDaynightConfiguration(FD_HANDLE Handle, ImageDnParam *ImageDnParamPtr);
    GMI_RESULT SetWhiteBalanceConfiguration(FD_HANDLE Handle, ImageWbParam *ImageWbParamPtr);
    GMI_RESULT GetWhiteBalanceConfiguration(FD_HANDLE Handle, ImageWbParam *ImageWbParamPtr);   

    GMI_RESULT OpenAutoFocusDevice(uint32_t VidSourceId, FD_HANDLE *Handle);
    GMI_RESULT CloseAutoFocusDevice(FD_HANDLE Handle);
    GMI_RESULT StartAutoFocusDevice(FD_HANDLE Handle);
    GMI_RESULT StopAutoFocusDevice(FD_HANDLE Handle);
    GMI_RESULT PauseAutoFocus(FD_HANDLE Handle, boolean_t Pause);
    GMI_RESULT AutoFocusGlobalScan(FD_HANDLE Handle);
    GMI_RESULT SetAutoFocusMode(FD_HANDLE Handle, int32_t Mode);
    GMI_RESULT ControlAutoFocus(FD_HANDLE Handle, int32_t Mode);
    
    GMI_RESULT OpenZoomDevice(uint32_t VidSourceId, FD_HANDLE *Handle);
    GMI_RESULT CloseZoomDevice(FD_HANDLE Handle);
    GMI_RESULT SetZoomPosition(FD_HANDLE Handle, int32_t Position);
    GMI_RESULT GetZoomPosition(FD_HANDLE Handle, int32_t *PositionPtr);
    GMI_RESULT GetZoomRange(FD_HANDLE Handle, int32_t *MinPositionPtr, int32_t *MaxPositionPtr);
    GMI_RESULT ControlZoom(FD_HANDLE Handle, int8_t Mode);
    GMI_RESULT SetZoomStep(FD_HANDLE Handle, int32_t Step);
    GMI_RESULT StartZoom(FD_HANDLE FocusHandle, FD_HANDLE ZoomHandle, int8_t Mode);
    GMI_RESULT StopZoom(FD_HANDLE FocusHandle, FD_HANDLE ZoomHandle, int32_t *PositionPtr);
public:
	GMI_RESULT CheckVideoEncodeConfiguration(VideoEncodeParam *EncParamPtr);
    
private:    
    GMI_RESULT CheckOsdConfiguration(VideoOSDParam *OsdParamPtr);
    GMI_RESULT CheckImageConfiguration(ImageBaseParam *ImageParamPtr);
    GMI_RESULT CheckVideoReslution(uint16_t Height, uint16_t Width);
    GMI_RESULT CheckImageAdvancedConfiguration(ImageAdvanceParam *ImageAdvancedParamPtr);
    GMI_RESULT CheckWhiteBalanceConfiguration(ImageWbParam *ImageWbParamPtr);
    GMI_RESULT CheckDaynightConfiguration(ImageDnParam *ImgDnParamPtr);
    GMI_RESULT CheckVinVoutConfiguration(VideoInParam *VidInParamPtr, VideoOutParam *VidOutParamPtr);
    GMI_RESULT CheckAudioEncodeConfiguration(AudioEncParam *AudioEncParamPtr);
    GMI_RESULT CheckAudioDecodeConfiguration(AudioDecParam *AudioDecParamPtr);
    GMI_RESULT GetAdaptedOsdFontSize(uint32_t Id, uint8_t *FontSize);
    GMI_RESULT StartCodec(boolean_t EncodeMode, uint32_t SourceId, uint32_t MeidaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *CodecHandle);
    GMI_RESULT StopCodec(FD_HANDLE& CodecHandle);
private:
    uint16_t            m_RTSP_ServerPort;
    uint16_t            m_ClientPort;
    uint16_t            m_ServerPort;
    uint16_t            m_UDP_SessionBufferSize;
    MediaCenter         m_MediaCenter;
    MediaTransportProxy m_MediaTransport; 
    VideoEncodeParam    m_VideoEncParam[MAX_VIDEO_STREAM_NUM];
};

#endif
