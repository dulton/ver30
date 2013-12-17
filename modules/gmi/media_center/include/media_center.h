#pragma once

#include "gmi_system_headers.h"

#define MEDIA_CONFIGURATION_MAX_LENGTH 1024

#define MONITOR_ZOOM_OPERATION_TIME 0

class MediaDecodePipeline;
class MediaEncodePipeline;

class MediaCenter
{
public:
    MediaCenter(void);
    ~MediaCenter(void);

    GMI_RESULT OpenVinVoutDevice     ( uint16_t SensorId, uint16_t ChannelId, FD_HANDLE *VinVoutHandle );
    GMI_RESULT CloseVinVoutDevice    ( FD_HANDLE& VinVoutHandle );
    GMI_RESULT GetVinVoutConfig      ( FD_HANDLE VinVoutHandle, void_t *VinParameter, size_t *VinParameterLength, void_t *VoutParameter, size_t *VoutParameterLength );
    GMI_RESULT SetVinVoutConfig      ( FD_HANDLE VinVoutHandle, const void_t *VinParameter, size_t VinParameterLength, const void_t *VoutParameter, size_t VoutParameterLength );

    GMI_RESULT CreateCodec           ( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *CodecHandle );
    GMI_RESULT DestroyCodec          ( FD_HANDLE& CodecHandle );
    GMI_RESULT StartCodec            ( FD_HANDLE CodecHandle );
    GMI_RESULT StopCodec             ( FD_HANDLE CodecHandle );
    GMI_RESULT GetCodecConfig        ( FD_HANDLE CodecHandle, void_t *CodecParameter, size_t *CodecParameterLength );
    GMI_RESULT SetCodecConfig        ( FD_HANDLE CodecHandle, const void_t *CodecParameter, size_t CodecParameterLength );
    GMI_RESULT GetOsdConfig          ( FD_HANDLE CodecHandle, void_t *OsdParameter, size_t *OsdParameterLength );
    GMI_RESULT SetOsdConfig          ( FD_HANDLE CodecHandle, const void_t *OsdParameter, size_t OsdParameterLength );
    GMI_RESULT ForceGenerateIdrFrame ( FD_HANDLE CodecHandle );
    // this method is called when media center attempt to exit, which will stop codec and release related resource if required.
    GMI_RESULT ReleaseCodecResource  ();

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
    GMI_RESULT NotifyAutoFocus       ( FD_HANDLE	AutoFocusHandle, int32_t EventType, uint8_t *ExtData, uint32_t Length );
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

private:
    std::vector< SafePtr<MediaEncodePipeline> > m_EncodePipelines;
    std::vector< SafePtr<MediaDecodePipeline> > m_DecodePipelines;
};
