#include "log.h"
#include "service_utilitly.h"
#include "service_media_configuration.h"
#include "service_ptz.h"
#include "soapH.h"
#include "sys_client.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"

//video source
static MediaVideoSource l_MediaVideoSource[DEFAULT_VIDEO_SOURCES] =
{
    {
        VIDEO_SOURCE_CONFIG_NAME,
        VIDEO_SOURCE_CONFIG_TOKEN,
        VIDEO_SOURCE_CONFIG_SOURCE_TOKEN,
        DEFAULT_VIDEO_SOURCE_RES_WIDTH,
        DEFAULT_VIDEO_SOURCE_RES_HEIGHT,
        DEFAULT_VIDEO_SOURCE_FRAME_RATE,
        {
            128.0,
            128.0,
            128.0,
            128.0,
            tt__IrCutFilterMode__ON, //IrCut
            {
                //Exposure
            },
            {
                //FocusConfiguration
            },
            {
                //BacklightCompensation
                tt__BacklightCompensationMode__OFF,
                20
            },
            {
                //WideDynamicRange
                tt__WideDynamicMode__OFF,
                20
            },
            {
                //WhiteBalance
                tt__WhiteBalanceMode__AUTO,
                0.0,
                0.0
            }
        }
    }
};


//video source cofiguration
static VideoSourceConfiguration l_VideoSourceConfiguration[DEFAULT_VIDEO_SOURCES] =
{
    {
        VIDEO_SOURCE_CONFIG_NAME,
        DEFAULT_STREAM_NUM,
        VIDEO_SOURCE_CONFIG_TOKEN,
        VIDEO_SOURCE_CONFIG_SOURCE_TOKEN,
        {
            0,
            0,
            DEFAULT_VIDEO_SOURCE_RES_WIDTH,
            DEFAULT_VIDEO_SOURCE_RES_HEIGHT
        }
    }
};


//video EncoderConfiguration
static VideoEncoderConfiguration l_VideoEncoderConfiguration[DEFAULT_STREAM_NUM] =
{
    {
        VIDEO_ENCODER_NAME_1,
        1,
        VIDEO_ENCODER_TOKEN_1,
        //Encoding
        tt__VideoEncoding__H264,
        3,
        {
            //Resolution
            DEFAULT_VIDEO_MAIN_STREAM_RES_WIDTH,
            DEFAULT_VIDEO_MAIN_STREAM_RES_HEIGHT
        },
        {
            //RateControl
            DEFAULT_VIDEO_MAIN_STREAM_FRAME_RATE,
            1,
            DEFAULT_VIDEO_MAIN_STREAM_BITRATE
        },
        {
            //MPEG4

        },
        {
            //h264
            DEFAULT_VIDEO_MAIN_STREAM_I_INTERVAL,
            tt__H264Profile__Main
        },
        {
            //multicast
            //IPAddress
            {
                tt__IPType__IPv4,
                "0.0.0.0"
            },
            8600,
            1,
            xsd__boolean__false_
        },
        DEFAULT_VIDEO_SESSION_TIMEOUT,
    }
    ,
    {
        VIDEO_ENCODER_NAME_2,
        1,
        VIDEO_ENCODER_TOKEN_2,
        //Encoding
        tt__VideoEncoding__H264,
        3,
        {
            //Resolution
            DEFAULT_VIDEO_SECOND_STREAM_RES_WIDTH,
            DEFAULT_VIDEO_SECOND_STREAM_RES_HEIGHT
        },
        {
            //RateControl
            DEFAULT_VIDEO_SECOND_STREAM_FRAME_RATE,
            1,
            DEFAULT_VIDEO_SECOND_STREAM_BITRATE
        },
        {
            //MPEG4

        },
        {
            //h264
            DEFAULT_VIDEO_SECOND_STREAM_I_INTERVAL,
            tt__H264Profile__Main
        },
        {
            //multicast
            //IPAddress
            {
                tt__IPType__IPv4,
                "0.0.0.0"
            },
            8600,
            1,
            xsd__boolean__false_
        },
        DEFAULT_VIDEO_SESSION_TIMEOUT
    }
};


//configuration options
static VideoEncoderConfigurationOptions l_VideoEncoderConfigOptions =
{
    {MIN_QUALITY, MAX_QUALITY },
    {
        4,
        {{RES_720P_WIDTH, RES_720_HEIGHT}, {RES_D1_WIDTH, RES_D1_HEIGHT}, {RES_CIF_WIDTH, RES_CIF_HEIGHT}, {RES_QCIF_WIDTH, RES_QCIF_HEIGHT}, },
        {MIN_I_INTERVAL, MAX_I_INTERVAL},
        {MIN_FRAME_RATE, MAX_FRAME_RATE},
        {MIN_ENCODING_INTERVAL, MAX_ENCODING_INTERVAL},
        1,
        {tt__H264Profile__Main,}
    }
};

//profile
static Profile l_Profile[MAX_STREAM_NUM] =
{
    {
        xsd__boolean__true_,
        PROFILE_NAME_1,
        PROFILE_TOKEN_1,
        l_VideoSourceConfiguration[0],
        l_VideoEncoderConfiguration[0],
        g_PTZ_Coniguration[0]
    }
    ,
    {
        xsd__boolean__true_,
        PROFILE_NAME_2,
        PROFILE_TOKEN_2,
        l_VideoSourceConfiguration[0],
        l_VideoEncoderConfiguration[1],
        g_PTZ_Coniguration[0]
    },
    {
        xsd__boolean__true_,
        PROFILE_NAME_3,
        PROFILE_TOKEN_3,
        l_VideoSourceConfiguration[0],
        l_VideoEncoderConfiguration[1],
        g_PTZ_Coniguration[0]
    }
    ,
    {
        xsd__boolean__true_,
        PROFILE_NAME_4,
        PROFILE_TOKEN_4,
        l_VideoSourceConfiguration[0],
        l_VideoEncoderConfiguration[1],
        g_PTZ_Coniguration[0]
    }
};


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetServiceCapabilities(struct soap *soap_ptr, struct _trt__GetServiceCapabilities *trt__GetServiceCapabilities, struct _trt__GetServiceCapabilitiesResponse *trt__GetServiceCapabilitiesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t    SessionId = 0;
    uint32_t    AuthValue = 0;
    uint32_t    StreamNum       = 0;
    GMI_RESULT  Result          = GMI_SUCCESS;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auhten fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        //should get from sys control server
        //get stream num
        Result = SysGetEncodeStreamNum(SessionId, AuthValue, &StreamNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeStreamNum fail, Result = 0x%lx\n", Result);
            break;
        }

        trt__GetServiceCapabilitiesResponse->Capabilities = (struct trt__Capabilities *)soap_malloc_zero(soap_ptr, sizeof(struct trt__Capabilities));
        trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities = (struct trt__ProfileCapabilities*)soap_malloc_zero(soap_ptr, sizeof(struct trt__ProfileCapabilities));
        trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->MaximumNumberOfProfiles  = (int*)soap_malloc_zero(soap_ptr, sizeof(int));
        *trt__GetServiceCapabilitiesResponse->Capabilities->ProfileCapabilities->MaximumNumberOfProfiles = StreamNum;

        //rtsp_tcp = true
        trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities = (struct trt__StreamingCapabilities*)soap_malloc_zero(soap_ptr, sizeof(struct trt__StreamingCapabilities));
        trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTPMulticast = (enum xsd__boolean *)soap_malloc_zero(soap_ptr, sizeof(int));
        *trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTPMulticast = xsd__boolean__false_;
        trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORETCP = (enum xsd__boolean *)soap_malloc_zero(soap_ptr, sizeof(int));
        *trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORETCP = xsd__boolean__true_;
        trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = (enum xsd__boolean *)soap_malloc_zero(soap_ptr, sizeof(int));
        *trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->RTP_USCORERTSP_USCORETCP = xsd__boolean__true_;
        trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->NonAggregateControl = (enum xsd__boolean *)soap_malloc_zero(soap_ptr, sizeof(int));
        *trt__GetServiceCapabilitiesResponse->Capabilities->StreamingCapabilities->NonAggregateControl = xsd__boolean__false_;
        trt__GetServiceCapabilitiesResponse->Capabilities->SnapshotUri = (enum xsd__boolean *)soap_malloc_zero(soap_ptr, sizeof(int));
        *trt__GetServiceCapabilitiesResponse->Capabilities->SnapshotUri = xsd__boolean__false_;

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSources(struct soap *soap_ptr, struct _trt__GetVideoSources *trt__GetVideoSources, struct _trt__GetVideoSourcesResponse *trt__GetVideoSourcesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t          SessionId = 0;
    uint32_t          AuthValue = 0;
    struct soap      *soap           = soap_ptr;
    int32_t           Id             = 0;
    GMI_RESULT        Result         = GMI_SUCCESS;
    uint8_t           VideoSources   = 0;
    MediaVideoSource *VideoSourcePtr = NULL;
    SysPkgVideoSource SysVideoSource = {0};
    SysPkgImaging     SysImaging     = {0};
    SysPkgAdvancedImaging SysAdvancedImaging = {0};
    SysPkgWhiteBalance    SysWhiteBalance = {0};


    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auhten fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        VideoSourcePtr = l_MediaVideoSource;
        //should get from sys control server
        //get video sources
        //get video source resolution
        //get video source frameRate
        //get video source brightness, colorStauration, sharpness, backlightCompensation, exposure, focus, wideDynamicRange, whiteBalance.
        VideoSources         = DEFAULT_VIDEO_SOURCES;

        memset(&SysVideoSource, 0, sizeof(SysPkgVideoSource));
        Result = SysGetVideoSource(SessionId, AuthValue, &SysVideoSource);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SysGetVideoSource fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetVideoSource fail, Result = 0x%lx\n", Result);
            break;
        }
        ONVIF_INFO("SrcWidth   = %d\n", SysVideoSource.s_SrcWidth);
        ONVIF_INFO("SrcHeight  = %d\n", SysVideoSource.s_SrcHeight);
        ONVIF_INFO("SrcFps     = %d\n", SysVideoSource.s_SrcFps);

        memset(&SysImaging, 0, sizeof(SysPkgImaging));
        Result = SysGetImaging(SessionId, AuthValue, &SysImaging);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SysGetImaging fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetImaging fail, Result = 0x%lx\n", Result);
            break;
        }
        ONVIF_INFO("Brightness = %d\n", SysImaging.s_Brightness);
        ONVIF_INFO("Contrast   = %d\n", SysImaging.s_Contrast);
        ONVIF_INFO("Hue        = %d\n", SysImaging.s_Hue);
        ONVIF_INFO("Saturation = %d\n", SysImaging.s_Saturation);
        ONVIF_INFO("Sharpness  = %d\n", SysImaging.s_Sharpness);
        ONVIF_INFO("ExposureMode     = %d\n", SysImaging.s_Exposure.s_ExposureMode);
        ONVIF_INFO("ExposureValueMax = %d\n", SysImaging.s_Exposure.s_ShutterMax);
        ONVIF_INFO("ExposureValueMin = %d\n", SysImaging.s_Exposure.s_ShutterMin);
        ONVIF_INFO("GainMax          = %d\n", SysImaging.s_Exposure.s_GainMax);

        memset(&SysAdvancedImaging, 0, sizeof(SysPkgAdvancedImaging));
        Result = SysGetAdvancedImaging(SessionId, AuthValue, &SysAdvancedImaging);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SysGetAdvancedImaging fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetAdvancedImaging fail, Result = 0x%lx\n", Result);
            break;
        }
        ONVIF_INFO("MeteringMode        = %d\n", SysAdvancedImaging.s_MeteringMode);
        ONVIF_INFO("BackLightCompFlag   = %d\n", SysAdvancedImaging.s_BackLightCompFlag);
        ONVIF_INFO("DcIrisFlag          = %d\n", SysAdvancedImaging.s_DcIrisFlag);
        ONVIF_INFO("LocalExposure       = %d\n", SysAdvancedImaging.s_LocalExposure);
        ONVIF_INFO("MctfStrength        = %d\n", SysAdvancedImaging.s_MctfStrength);
        ONVIF_INFO("DcIrisDuty          = %d\n", SysAdvancedImaging.s_DcIrisDuty);
        ONVIF_INFO("AeTargetRatio       = %d\n", SysAdvancedImaging.s_AeTargetRatio);

        memset(&SysWhiteBalance, 0, sizeof(SysPkgWhiteBalance));
        Result = SysGetWhiteBalance(SessionId, AuthValue, &SysWhiteBalance);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetWhiteBalance fail, Result = 0x%lx\n", Result);
            break;
        }
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "SysWhiteBalance.s_Mode  = %d\n", SysWhiteBalance.s_Mode);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "SysWhiteBalance.s_BGain = %d\n", SysWhiteBalance.s_BGain);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "SysWhiteBalance.s_RGain = %d\n", SysWhiteBalance.s_RGain);

        trt__GetVideoSourcesResponse->__sizeVideoSources = VideoSources;
        trt__GetVideoSourcesResponse->VideoSources = (struct tt__VideoSource *)soap_malloc_zero(soap, sizeof(struct tt__VideoSource) * trt__GetVideoSourcesResponse->__sizeVideoSources);
        trt__GetVideoSourcesResponse->VideoSources[Id].Framerate          = SysVideoSource.s_SrcFps;
        trt__GetVideoSourcesResponse->VideoSources[Id].Resolution         = (struct tt__VideoResolution *)soap_malloc_zero(soap, sizeof(struct tt__VideoResolution));
        trt__GetVideoSourcesResponse->VideoSources[Id].Resolution->Height = SysVideoSource.s_SrcHeight;
        trt__GetVideoSourcesResponse->VideoSources[Id].Resolution->Width  = SysVideoSource.s_SrcWidth;
        trt__GetVideoSourcesResponse->VideoSources[Id].token              = (char_t *)soap_strdup(soap, VideoSourcePtr[0].s_VideoSourceToken);

        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging                               = (struct tt__ImagingSettings*)soap_malloc_zero(soap, sizeof(struct tt__ImagingSettings));
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Brightness                   = (float*)soap_malloc_zero(soap, sizeof(float));
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Brightness[0]                = (float)SysImaging.s_Brightness;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->ColorSaturation              = (float*)soap_malloc_zero(soap, sizeof(float));
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->ColorSaturation[0]           = (float)SysImaging.s_Saturation;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Contrast                     = (float*)soap_malloc_zero(soap, sizeof(float));
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Contrast[0]                  = (float)SysImaging.s_Contrast;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->IrCutFilter                  = (enum tt__IrCutFilterMode*)soap_malloc_zero(soap, sizeof(int));
        *trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->IrCutFilter                 = VideoSourcePtr[0].s_Imaging.s_IrCutFilter;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Sharpness                    = (float*)soap_malloc_zero(soap, sizeof(float));
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Sharpness[0]                 = (float)SysImaging.s_Sharpness;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->BacklightCompensation        = (struct tt__BacklightCompensation*)soap_malloc_zero(soap, sizeof(struct tt__BacklightCompensation));

        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->BacklightCompensation->Mode  = \
                (SysAdvancedImaging.s_BackLightCompFlag == 1)? tt__BacklightCompensationMode__ON : tt__BacklightCompensationMode__OFF;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->BacklightCompensation->Level = VideoSourcePtr[0].s_Imaging.s_BacklightCompensation.Level;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Exposure                     = (struct tt__Exposure*)soap_malloc_zero(soap, sizeof(struct tt__Exposure));
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Exposure->Mode               = tt__ExposureMode__AUTO;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Exposure->MaxExposureTime    = (float)SysImaging.s_Exposure.s_ShutterMax;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Exposure->MinExposureTime    = (float)SysImaging.s_Exposure.s_ShutterMin;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Exposure->MaxGain            = (float)SysImaging.s_Exposure.s_GainMax;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Focus                        = NULL;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->WideDynamicRange             = (struct tt__WideDynamicRange*)soap_malloc_zero(soap, sizeof(struct tt__WideDynamicRange));
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->WideDynamicRange->Mode       = \
                (SysAdvancedImaging.s_LocalExposure == 0) ?  tt__WideDynamicMode__OFF : tt__WideDynamicMode__ON;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->WideDynamicRange->Level      = SysAdvancedImaging.s_LocalExposure;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->WhiteBalance                 = (struct tt__WhiteBalance*)soap_malloc_zero(soap, sizeof(struct tt__WhiteBalance));
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->WhiteBalance->Mode           = \
                (SysWhiteBalance.s_Mode == 0) ? tt__WhiteBalanceMode__AUTO : tt__WhiteBalanceMode__MANUAL;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->WhiteBalance->CrGain         = SysWhiteBalance.s_RGain;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->WhiteBalance->CbGain         = SysWhiteBalance.s_BGain;
        trt__GetVideoSourcesResponse->VideoSources[Id].Imaging->Extension                    = NULL;
        trt__GetVideoSourcesResponse->VideoSources[Id].Extension                             = NULL;

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while(0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSources(struct soap *soap_ptr, struct _trt__GetAudioSources *trt__GetAudioSources, struct _trt__GetAudioSourcesResponse *trt__GetAudioSourcesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputs(struct soap *soap_ptr, struct _trt__GetAudioOutputs *trt__GetAudioOutputs, struct _trt__GetAudioOutputsResponse *trt__GetAudioOutputsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__CreateProfile(struct soap *soap_ptr, struct _trt__CreateProfile *trt__CreateProfile, struct _trt__CreateProfileResponse *trt__CreateProfileResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfile(struct soap *soap_ptr, struct _trt__GetProfile *trt__GetProfile, struct _trt__GetProfileResponse *trt__GetProfileResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t          SessionId = 0;
    uint32_t          AuthValue = 0;
    Profile           *ProfilePtr = NULL;
    uint32_t          StreamNum   = DEFAULT_STREAM_NUM;
    GMI_RESULT        Result      = GMI_SUCCESS;
    SysPkgVideoSource SysVideoSource  = {0};
    SysPkgEncodeCfg   *SysEncodeCfgPtr = NULL;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auhten fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        //profiles
        ProfilePtr = l_Profile;
        Result = SysGetVideoSource(SessionId, AuthValue, &SysVideoSource);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetVideoSource fail, Result = 0x%lx\n", Result);
            break;
        }
        ONVIF_INFO("SrcWidth   = %d\n", SysVideoSource.s_SrcWidth);
        ONVIF_INFO("SrcHeight  = %d\n", SysVideoSource.s_SrcHeight);
        ONVIF_INFO("SrcFps     = %d\n", SysVideoSource.s_SrcFps);

        Result = SysGetEncodeStreamNum(SessionId, AuthValue, &StreamNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeStreamNum fail, Result = 0x%lx\n", Result);
            break;
        }

        if (StreamNum <= 0)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StreamNum %d incorrect\n", StreamNum);
            break;
        }

        SysEncodeCfgPtr = (SysPkgEncodeCfg*)malloc(StreamNum*sizeof(SysPkgEncodeCfg));
        if (NULL == SysEncodeCfgPtr)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "malloc fail\n");
            break;
        }

        uint32_t RespNum;
        Result = SysGetEncodeCfg(SessionId, AuthValue,SysEncodeCfgPtr,  StreamNum, &RespNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeCfg fail, Result = 0x%lx\n", Result);
            break;
        }

        uint32_t Id;
        for (Id = 0; Id < RespNum; Id++)
        {
            ONVIF_INFO("VideoId        = %d\n", SysEncodeCfgPtr[Id].s_VideoId);
            ONVIF_INFO("StreamId       = %d\n", SysEncodeCfgPtr[Id].s_Flag);
            ONVIF_INFO("PicHeight      = %d\n", SysEncodeCfgPtr[Id].s_PicHeight);
            ONVIF_INFO("PicWidth       = %d\n", SysEncodeCfgPtr[Id].s_PicWidth);
            ONVIF_INFO("Compression    = %d\n", SysEncodeCfgPtr[Id].s_Compression);
            ONVIF_INFO("BitrateCtrl    = %d\n", SysEncodeCfgPtr[Id].s_BitrateCtrl);
            ONVIF_INFO("BitRateAverage = %d\n", SysEncodeCfgPtr[Id].s_BitRateAverage);
            ONVIF_INFO("BitRateDown    = %d\n", SysEncodeCfgPtr[Id].s_BitRateDown);
            ONVIF_INFO("BitRateUp      = %d\n", SysEncodeCfgPtr[Id].s_BitRateUp);
            ONVIF_INFO("FPS            = %d\n", SysEncodeCfgPtr[Id].s_FPS);
            ONVIF_INFO("Gop            = %d\n", SysEncodeCfgPtr[Id].s_Gop);
            ONVIF_INFO("Quality        = %d\n", SysEncodeCfgPtr[Id].s_Quality);
            ONVIF_INFO("Rotate         = %d\n", SysEncodeCfgPtr[Id].s_Rotate);

            ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Height          = SysEncodeCfgPtr[Id].s_PicHeight;
            ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Width           = SysEncodeCfgPtr[Id].s_PicWidth;
            ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.FrameRateLimit = SysEncodeCfgPtr[Id].s_FPS;
            if (SYS_BRC_CBR == SysEncodeCfgPtr[Id].s_BitrateCtrl)
            {
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.BitrateLimit = SysEncodeCfgPtr[Id].s_BitRateAverage;
            }
            else
            {
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.BitrateLimit = SysEncodeCfgPtr[Id].s_BitRateUp;
            }

            switch (SysEncodeCfgPtr[Id].s_Compression)
            {
            case SYS_COMP_H264:
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_H264.GovLength  = SysEncodeCfgPtr[Id].s_Gop;
                break;
            case SYS_COMP_MPEG4:
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_MPEG4.GovLength = SysEncodeCfgPtr[Id].s_Gop;
                break;
            }
            ProfilePtr[Id].s_PTZ_Configuration.s_UseCount        = RespNum;
            ProfilePtr[Id].s_VideoSourceConfiguration.s_UseCount = RespNum;
        }

        for (Id = 0; Id < RespNum; Id++)
        {
            if (0 == strcmp(trt__GetProfile->ProfileToken, ProfilePtr[Id].s_Token))
            {
                trt__GetProfileResponse->Profile = (struct tt__Profile*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Profile));
                trt__GetProfileResponse->Profile->Name     = (char *)soap_strdup(soap_ptr, ProfilePtr[Id].s_Name);
                trt__GetProfileResponse->Profile->token    = (char *)soap_strdup(soap_ptr, ProfilePtr[Id].s_Token);
                trt__GetProfileResponse->Profile->fixed    = (enum xsd__boolean *)soap_malloc_zero(soap_ptr, sizeof(int));
                *(trt__GetProfileResponse->Profile->fixed) = ProfilePtr[Id].s_Fixed;

                trt__GetProfileResponse->Profile->VideoEncoderConfiguration              = (struct tt__VideoEncoderConfiguration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoEncoderConfiguration));
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution  = (struct tt__VideoResolution *)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoResolution));
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl*)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoRateControl));
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264        = (struct tt__H264Configuration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__H264Configuration));
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4       = NULL;
                switch (SysEncodeCfgPtr[Id].s_Compression)
                {
                case SYS_COMP_H264:
                    trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264 = (struct tt__H264Configuration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__H264Configuration));
                    trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->GovLength   = ProfilePtr[Id].s_VideoEncoderConfiguration.s_H264.GovLength;
                    trt__GetProfileResponse->Profile->VideoEncoderConfiguration->H264->H264Profile = ProfilePtr[Id].s_VideoEncoderConfiguration.s_H264.H264Profile;
                    break;
                case SYS_COMP_MPEG4:
                    trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4 = (struct tt__Mpeg4Configuration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Mpeg4Configuration));
                    trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4->GovLength    = ProfilePtr[Id].s_VideoEncoderConfiguration.s_MPEG4.GovLength ;
                    trt__GetProfileResponse->Profile->VideoEncoderConfiguration->MPEG4->Mpeg4Profile = ProfilePtr[Id].s_VideoEncoderConfiguration.s_MPEG4.Mpeg4Profile;
                    break;
                }
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast   = (struct tt__MulticastConfiguration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__MulticastConfiguration));
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address = (struct tt__IPAddress*)soap_malloc_zero(soap_ptr, sizeof(struct tt__IPAddress));
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address = (char **)soap_malloc_zero(soap_ptr, sizeof(char*));

                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Name                          = (char*)soap_strdup(soap_ptr, ProfilePtr[Id].s_VideoEncoderConfiguration.s_Name);
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->UseCount                      = ProfilePtr[Id].s_VideoEncoderConfiguration.s_UseCount;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->token                         = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Token;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Encoding                      = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Encoding;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Width             = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Width;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Resolution->Height            = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Height;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Quality                       = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Quality;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->FrameRateLimit   = ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.FrameRateLimit;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->EncodingInterval = ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.EncodingInterval;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->RateControl->BitrateLimit     = ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.BitrateLimit;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->SessionTimeout                = ProfilePtr[Id].s_VideoEncoderConfiguration.s_SessionTimeout;
                *(trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->IPv4Address)= soap_strdup(soap_ptr, ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_Address.IPv4Address);
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Address->Type      = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_Address.Type;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->Port               = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_Port;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->TTL                = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_TTL;
                trt__GetProfileResponse->Profile->VideoEncoderConfiguration->Multicast->AutoStart          = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_AutoStart;


                trt__GetProfileResponse->Profile->VideoSourceConfiguration                 = (struct tt__VideoSourceConfiguration *)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoSourceConfiguration));
                trt__GetProfileResponse->Profile->VideoSourceConfiguration->Name           = (char *)soap_strdup(soap_ptr, ProfilePtr[Id].s_VideoSourceConfiguration.s_Name);
                trt__GetProfileResponse->Profile->VideoSourceConfiguration->token          = (char *)soap_strdup(soap_ptr, ProfilePtr[Id].s_VideoSourceConfiguration.s_Token);
                trt__GetProfileResponse->Profile->VideoSourceConfiguration->SourceToken    = (char *)soap_strdup(soap_ptr, ProfilePtr[Id].s_VideoSourceConfiguration.s_SourceToken);
                trt__GetProfileResponse->Profile->VideoSourceConfiguration->UseCount       = ProfilePtr[Id].s_VideoSourceConfiguration.s_UseCount;
                trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds         = (struct tt__IntRectangle *)soap_malloc_zero(soap_ptr,sizeof(struct tt__IntRectangle));
                trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->height = SysVideoSource.s_SrcHeight;
                trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->width  = SysVideoSource.s_SrcWidth;
                trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->x      = ProfilePtr[Id].s_VideoSourceConfiguration.s_Bounds.x;
                trt__GetProfileResponse->Profile->VideoSourceConfiguration->Bounds->y      = ProfilePtr[Id].s_VideoSourceConfiguration.s_Bounds.y;


                trt__GetProfileResponse->Profile->PTZConfiguration                           = (struct tt__PTZConfiguration *)soap_malloc_zero(soap_ptr, sizeof(struct tt__PTZConfiguration));
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed          = (struct tt__PTZSpeed *)soap_malloc_zero(soap_ptr, sizeof(struct tt__PTZSpeed));
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt = (struct tt__Vector2D *)soap_malloc_zero(soap_ptr, sizeof(struct tt__Vector2D));
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->Zoom    = (struct tt__Vector1D *)soap_malloc_zero(soap_ptr, sizeof(struct tt__Vector1D));
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZTimeout        = (LONG64 *)soap_malloc_zero(soap_ptr, sizeof(LONG64));
                trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits                              = (struct tt__PanTiltLimits*)soap_malloc_zero(soap_ptr, sizeof(struct tt__PanTiltLimits));
                trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range                       = (struct tt__Space2DDescription*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Space2DDescription));
                trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->XRange               = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->YRange               = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits                                 = (struct tt__ZoomLimits*)soap_malloc_zero(soap_ptr, sizeof(struct tt__ZoomLimits));
                trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range                          = (struct tt__Space1DDescription*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Space1DDescription));
                trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range->XRange                  = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));

                trt__GetProfileResponse->Profile->PTZConfiguration->Name                                  = (char*)soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_Name);
                trt__GetProfileResponse->Profile->PTZConfiguration->UseCount                              = ProfilePtr[Id].s_PTZ_Configuration.s_UseCount;
                trt__GetProfileResponse->Profile->PTZConfiguration->token                                 = (char*)soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_Token);
                trt__GetProfileResponse->Profile->PTZConfiguration->NodeToken                             = (char*)soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_NodeToken);
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultAbsolutePantTiltPositionSpace  = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultAbsolutePantTiltPositionSpace);
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultAbsoluteZoomPositionSpace      = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultAbsoluteZoomPositionSpace);
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultRelativePanTiltTranslationSpace= soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultRelativePanTiltTranslationSpace);
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultRelativeZoomTranslationSpace   = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultRelativeZoomTranslationSpace);
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultContinuousPanTiltVelocitySpace = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultContinuousPanTiltVelocitySpace);
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultContinuousZoomVelocitySpace    = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultContinuousZoomVelocitySpace);
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt->x           = ProfilePtr[Id].s_PTZ_Configuration.s_PTZ_Speed.s_PanTilt.x;
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt->y           = ProfilePtr[Id].s_PTZ_Configuration.s_PTZ_Speed.s_PanTilt.y;
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->PanTilt->space       = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_PTZ_Speed.s_PanTilt.space);
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->Zoom->x              = ProfilePtr[Id].s_PTZ_Configuration.s_PTZ_Speed.s_Zoom.x;
                trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZSpeed->Zoom->space          = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_PTZ_Speed.s_Zoom.space);
                *(trt__GetProfileResponse->Profile->PTZConfiguration->DefaultPTZTimeout)                  = ProfilePtr[Id].s_PTZ_Configuration.s_DefaultTimeout;
                //PanTilt & Zoom limits
                trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->URI		     = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_PanTiltLimits.s_Range.s_URI);
                trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->XRange->Min    = ProfilePtr[Id].s_PTZ_Configuration.s_PanTiltLimits.s_Range.s_XRangeMin;
                trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->XRange->Max    = ProfilePtr[Id].s_PTZ_Configuration.s_PanTiltLimits.s_Range.s_XRangeMax;
                trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->YRange->Min    = ProfilePtr[Id].s_PTZ_Configuration.s_PanTiltLimits.s_Range.s_YRangeMin;
                trt__GetProfileResponse->Profile->PTZConfiguration->PanTiltLimits->Range->YRange->Max    = ProfilePtr[Id].s_PTZ_Configuration.s_PanTiltLimits.s_Range.s_YRangeMax;
                trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range->URI 		         = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_ZoomLimits.s_Range.s_URI);
                trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range->XRange->Min       = ProfilePtr[Id].s_PTZ_Configuration.s_ZoomLimits.s_Range.s_XRangeMin;
                trt__GetProfileResponse->Profile->PTZConfiguration->ZoomLimits->Range->XRange->Max       = ProfilePtr[Id].s_PTZ_Configuration.s_ZoomLimits.s_Range.s_XRangeMax;
                break;
            }
        }

        if (SysEncodeCfgPtr)
        {
            free(SysEncodeCfgPtr);
            SysEncodeCfgPtr = NULL;
        }
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    if (SysEncodeCfgPtr)
    {
        free(SysEncodeCfgPtr);
        SysEncodeCfgPtr = NULL;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetProfiles(struct soap *soap_ptr, struct _trt__GetProfiles *trt__GetProfiles, struct _trt__GetProfilesResponse *trt__GetProfilesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t          SessionId = 0;
    uint32_t          AuthValue = 0;
    GMI_RESULT        Result      = GMI_SUCCESS;
    uint32_t          StreamNum   = 0;
    Profile           *ProfilePtr = NULL;
    SysPkgVideoSource SysVideoSource  = {0};
    SysPkgEncodeCfg   *SysEncodeCfgPtr = NULL;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auhten fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        //should get from sys control server
        //get media capabilities,
        //get stream num,
        //get per stream encode params, resolution,bitrate, frameRate, quality,profile, gop, source resolution.
        //get ptz capabilities,
        //get ptz timeout,
        //get ptz num.

        //profiles
        ProfilePtr = l_Profile;
        Result = SysGetVideoSource(SessionId, AuthValue, &SysVideoSource);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetVideoSource fail, Result = 0x%lx\n", Result);
            break;
        }
        ONVIF_INFO("SrcWidth   = %d\n", SysVideoSource.s_SrcWidth);
        ONVIF_INFO("SrcHeight  = %d\n", SysVideoSource.s_SrcHeight);
        ONVIF_INFO("SrcFps     = %d\n", SysVideoSource.s_SrcFps);

        Result = SysGetEncodeStreamNum(SessionId, AuthValue, &StreamNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeStreamNum fail, Result = 0x%lx\n", Result);
            break;
        }

        if (StreamNum <= 0)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StreamNum %d incorrect\n", StreamNum);
            break;
        }

        SysEncodeCfgPtr = (SysPkgEncodeCfg*)malloc(StreamNum*sizeof(SysPkgEncodeCfg));
        if (NULL == SysEncodeCfgPtr)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "malloc fail\n");
            break;
        }

        uint32_t RespNum;
        Result = SysGetEncodeCfg(SessionId, AuthValue,  SysEncodeCfgPtr, StreamNum, &RespNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeCfg fail, Result = 0x%lx\n", Result);
            break;
        }

        uint32_t Id;
        for (Id = 0; Id < RespNum; Id++)
        {
            ONVIF_INFO("VideoId        = %d\n", SysEncodeCfgPtr[Id].s_VideoId);
            ONVIF_INFO("StreamId       = %d\n", SysEncodeCfgPtr[Id].s_Flag);
            ONVIF_INFO("PicHeight      = %d\n", SysEncodeCfgPtr[Id].s_PicHeight);
            ONVIF_INFO("PicWidth       = %d\n", SysEncodeCfgPtr[Id].s_PicWidth);
            ONVIF_INFO("Compression    = %d\n", SysEncodeCfgPtr[Id].s_Compression);
            ONVIF_INFO("BitrateCtrl    = %d\n", SysEncodeCfgPtr[Id].s_BitrateCtrl);
            ONVIF_INFO("BitRateAverage = %d\n", SysEncodeCfgPtr[Id].s_BitRateAverage);
            ONVIF_INFO("BitRateDown    = %d\n", SysEncodeCfgPtr[Id].s_BitRateDown);
            ONVIF_INFO("BitRateUp      = %d\n", SysEncodeCfgPtr[Id].s_BitRateUp);
            ONVIF_INFO("FPS            = %d\n", SysEncodeCfgPtr[Id].s_FPS);
            ONVIF_INFO("Gop            = %d\n", SysEncodeCfgPtr[Id].s_Gop);
            ONVIF_INFO("Quality        = %d\n", SysEncodeCfgPtr[Id].s_Quality);
            ONVIF_INFO("Rotate         = %d\n", SysEncodeCfgPtr[Id].s_Rotate);

            ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Height          = SysEncodeCfgPtr[Id].s_PicHeight;
            ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Width           = SysEncodeCfgPtr[Id].s_PicWidth;
            ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.FrameRateLimit = SysEncodeCfgPtr[Id].s_FPS;
            if (SYS_BRC_CBR == SysEncodeCfgPtr[Id].s_BitrateCtrl)
            {
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.BitrateLimit = SysEncodeCfgPtr[Id].s_BitRateAverage;
            }
            else
            {
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.BitrateLimit = SysEncodeCfgPtr[Id].s_BitRateUp;
            }

            switch (SysEncodeCfgPtr[Id].s_Compression)
            {
            case SYS_COMP_H264:
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_H264.GovLength  = SysEncodeCfgPtr[Id].s_Gop;
                break;
            case SYS_COMP_MPEG4:
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_MPEG4.GovLength = SysEncodeCfgPtr[Id].s_Gop;
                break;
            }
            ProfilePtr[Id].s_PTZ_Configuration.s_UseCount        = RespNum;
            ProfilePtr[Id].s_VideoSourceConfiguration.s_UseCount = RespNum;
        }

        trt__GetProfilesResponse->__sizeProfiles = RespNum;
        trt__GetProfilesResponse->Profiles = (struct tt__Profile*)soap_malloc_zero(soap_ptr, trt__GetProfilesResponse->__sizeProfiles * sizeof(struct tt__Profile));

        uint32_t __sizeProfiles = trt__GetProfilesResponse->__sizeProfiles;
        for (Id = 0; Id < __sizeProfiles; Id++)
        {
            trt__GetProfilesResponse->Profiles[Id].Name     = (char *)soap_strdup(soap_ptr, ProfilePtr[Id].s_Name);
            trt__GetProfilesResponse->Profiles[Id].token    = (char *)soap_strdup(soap_ptr, ProfilePtr[Id].s_Token);
            trt__GetProfilesResponse->Profiles[Id].fixed    = (enum xsd__boolean *)soap_malloc_zero(soap_ptr, sizeof(int));
            *(trt__GetProfilesResponse->Profiles[Id].fixed) = ProfilePtr[Id].s_Fixed;

            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration              = (struct tt__VideoEncoderConfiguration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoEncoderConfiguration));
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Resolution  = (struct tt__VideoResolution *)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoResolution));
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->RateControl = (struct tt__VideoRateControl*)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoRateControl));
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->H264        = (struct tt__H264Configuration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__H264Configuration));
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->MPEG4       = NULL;
            switch (SysEncodeCfgPtr[Id].s_Compression)
            {
            case SYS_COMP_H264:
                trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->H264 = (struct tt__H264Configuration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__H264Configuration));
                trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->H264->GovLength   = ProfilePtr[Id].s_VideoEncoderConfiguration.s_H264.GovLength;
                trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->H264->H264Profile = ProfilePtr[Id].s_VideoEncoderConfiguration.s_H264.H264Profile;
                break;
            case SYS_COMP_MPEG4:
                trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->MPEG4 = (struct tt__Mpeg4Configuration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Mpeg4Configuration));
                trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->MPEG4->GovLength    = ProfilePtr[Id].s_VideoEncoderConfiguration.s_MPEG4.GovLength ;
                trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->MPEG4->Mpeg4Profile = ProfilePtr[Id].s_VideoEncoderConfiguration.s_MPEG4.Mpeg4Profile;
                break;
            }
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Multicast   = (struct tt__MulticastConfiguration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__MulticastConfiguration));
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Multicast->Address = (struct tt__IPAddress*)soap_malloc_zero(soap_ptr, sizeof(struct tt__IPAddress));
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Multicast->Address->IPv4Address = (char **)soap_malloc_zero(soap_ptr, sizeof(char*));

            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Name                          = (char*)soap_strdup(soap_ptr, ProfilePtr[Id].s_VideoEncoderConfiguration.s_Name);
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->UseCount                      = ProfilePtr[Id].s_VideoEncoderConfiguration.s_UseCount;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->token                         = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Token;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Encoding                      = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Encoding;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Resolution->Width             = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Width;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Resolution->Height            = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Height;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Quality                       = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Quality;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->RateControl->FrameRateLimit   = ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.FrameRateLimit;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->RateControl->EncodingInterval = ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.EncodingInterval;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->RateControl->BitrateLimit     = ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.BitrateLimit;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->SessionTimeout                = ProfilePtr[Id].s_VideoEncoderConfiguration.s_SessionTimeout;
            *(trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Multicast->Address->IPv4Address)= soap_strdup(soap_ptr, ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_Address.IPv4Address);
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Multicast->Address->Type      = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_Address.Type;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Multicast->Port               = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_Port;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Multicast->TTL                = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_TTL;
            trt__GetProfilesResponse->Profiles[Id].VideoEncoderConfiguration->Multicast->AutoStart          = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_AutoStart;

            trt__GetProfilesResponse->Profiles[Id].VideoSourceConfiguration                 = (struct tt__VideoSourceConfiguration *)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoSourceConfiguration));
            trt__GetProfilesResponse->Profiles[Id].VideoSourceConfiguration->Name           = (char *)soap_strdup(soap_ptr, ProfilePtr[Id].s_VideoSourceConfiguration.s_Name);
            trt__GetProfilesResponse->Profiles[Id].VideoSourceConfiguration->token          = (char *)soap_strdup(soap_ptr, ProfilePtr[Id].s_VideoSourceConfiguration.s_Token);
            trt__GetProfilesResponse->Profiles[Id].VideoSourceConfiguration->SourceToken    = (char *)soap_strdup(soap_ptr, ProfilePtr[Id].s_VideoSourceConfiguration.s_SourceToken);
            trt__GetProfilesResponse->Profiles[Id].VideoSourceConfiguration->UseCount       = ProfilePtr[Id].s_VideoSourceConfiguration.s_UseCount;
            trt__GetProfilesResponse->Profiles[Id].VideoSourceConfiguration->Bounds         = (struct tt__IntRectangle *)soap_malloc_zero(soap_ptr,sizeof(struct tt__IntRectangle));
            trt__GetProfilesResponse->Profiles[Id].VideoSourceConfiguration->Bounds->height = SysVideoSource.s_SrcHeight;
            trt__GetProfilesResponse->Profiles[Id].VideoSourceConfiguration->Bounds->width  = SysVideoSource.s_SrcWidth;
            trt__GetProfilesResponse->Profiles[Id].VideoSourceConfiguration->Bounds->x      = ProfilePtr[Id].s_VideoSourceConfiguration.s_Bounds.x;
            trt__GetProfilesResponse->Profiles[Id].VideoSourceConfiguration->Bounds->y      = ProfilePtr[Id].s_VideoSourceConfiguration.s_Bounds.y;


            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration                           = (struct tt__PTZConfiguration *)soap_malloc_zero(soap_ptr, sizeof(struct tt__PTZConfiguration));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultPTZSpeed          = (struct tt__PTZSpeed *)soap_malloc_zero(soap_ptr, sizeof(struct tt__PTZSpeed));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultPTZSpeed->PanTilt = (struct tt__Vector2D *)soap_malloc_zero(soap_ptr, sizeof(struct tt__Vector2D));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultPTZSpeed->Zoom    = (struct tt__Vector1D *)soap_malloc_zero(soap_ptr, sizeof(struct tt__Vector1D));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultPTZTimeout        = (LONG64 *)soap_malloc_zero(soap_ptr, sizeof(LONG64));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->PanTiltLimits                              = (struct tt__PanTiltLimits*)soap_malloc_zero(soap_ptr, sizeof(struct tt__PanTiltLimits));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->PanTiltLimits->Range                       = (struct tt__Space2DDescription*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Space2DDescription));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->PanTiltLimits->Range->XRange               = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->PanTiltLimits->Range->YRange               = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->ZoomLimits                                 = (struct tt__ZoomLimits*)soap_malloc_zero(soap_ptr, sizeof(struct tt__ZoomLimits));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->ZoomLimits->Range                          = (struct tt__Space1DDescription*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Space1DDescription));
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->ZoomLimits->Range->XRange                  = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));

            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->Name                                  = (char*)soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_Name);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->UseCount                              = ProfilePtr[Id].s_PTZ_Configuration.s_UseCount;
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->token                                 = (char*)soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_Token);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->NodeToken                             = (char*)soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_NodeToken);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultAbsolutePantTiltPositionSpace  = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultAbsolutePantTiltPositionSpace);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultAbsoluteZoomPositionSpace      = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultAbsoluteZoomPositionSpace);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultRelativePanTiltTranslationSpace= soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultRelativePanTiltTranslationSpace);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultRelativeZoomTranslationSpace   = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultRelativeZoomTranslationSpace);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultContinuousPanTiltVelocitySpace = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultContinuousPanTiltVelocitySpace);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultContinuousZoomVelocitySpace    = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_DefaultContinuousZoomVelocitySpace);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultPTZSpeed->PanTilt->x           = ProfilePtr[Id].s_PTZ_Configuration.s_PTZ_Speed.s_PanTilt.x;
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultPTZSpeed->PanTilt->y           = ProfilePtr[Id].s_PTZ_Configuration.s_PTZ_Speed.s_PanTilt.y;
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultPTZSpeed->PanTilt->space       = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_PTZ_Speed.s_PanTilt.space);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultPTZSpeed->Zoom->x              = ProfilePtr[Id].s_PTZ_Configuration.s_PTZ_Speed.s_Zoom.x;
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultPTZSpeed->Zoom->space          = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_PTZ_Speed.s_Zoom.space);
            *(trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->DefaultPTZTimeout)                  = ProfilePtr[Id].s_PTZ_Configuration.s_DefaultTimeout;

            //PanTilt & Zoom limits
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->PanTiltLimits->Range->URI		     = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_PanTiltLimits.s_Range.s_URI);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->PanTiltLimits->Range->XRange->Min   = ProfilePtr[Id].s_PTZ_Configuration.s_PanTiltLimits.s_Range.s_XRangeMin;
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->PanTiltLimits->Range->XRange->Max   = ProfilePtr[Id].s_PTZ_Configuration.s_PanTiltLimits.s_Range.s_XRangeMax;
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->PanTiltLimits->Range->YRange->Min   = ProfilePtr[Id].s_PTZ_Configuration.s_PanTiltLimits.s_Range.s_YRangeMin;
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->PanTiltLimits->Range->YRange->Max   = ProfilePtr[Id].s_PTZ_Configuration.s_PanTiltLimits.s_Range.s_YRangeMax;
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->ZoomLimits->Range->URI 		     = soap_strdup(soap_ptr, ProfilePtr[Id].s_PTZ_Configuration.s_ZoomLimits.s_Range.s_URI);
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->ZoomLimits->Range->XRange->Min      = ProfilePtr[Id].s_PTZ_Configuration.s_ZoomLimits.s_Range.s_XRangeMin;
            trt__GetProfilesResponse->Profiles[Id].PTZConfiguration->ZoomLimits->Range->XRange->Max      = ProfilePtr[Id].s_PTZ_Configuration.s_ZoomLimits.s_Range.s_XRangeMax;
        }

        if (SysEncodeCfgPtr)
        {
            free(SysEncodeCfgPtr);
            SysEncodeCfgPtr = NULL;
        }
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    if (SysEncodeCfgPtr)
    {
        free(SysEncodeCfgPtr);
        SysEncodeCfgPtr = NULL;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoEncoderConfiguration(struct soap *soap_ptr, struct _trt__AddVideoEncoderConfiguration *trt__AddVideoEncoderConfiguration, struct _trt__AddVideoEncoderConfigurationResponse *trt__AddVideoEncoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoSourceConfiguration(struct soap *soap_ptr, struct _trt__AddVideoSourceConfiguration *trt__AddVideoSourceConfiguration, struct _trt__AddVideoSourceConfigurationResponse *trt__AddVideoSourceConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioEncoderConfiguration(struct soap *soap_ptr, struct _trt__AddAudioEncoderConfiguration *trt__AddAudioEncoderConfiguration, struct _trt__AddAudioEncoderConfigurationResponse *trt__AddAudioEncoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioSourceConfiguration(struct soap *soap_ptr, struct _trt__AddAudioSourceConfiguration *trt__AddAudioSourceConfiguration, struct _trt__AddAudioSourceConfigurationResponse *trt__AddAudioSourceConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__AddPTZConfiguration(struct soap *soap_ptr, struct _trt__AddPTZConfiguration *trt__AddPTZConfiguration, struct _trt__AddPTZConfigurationResponse *trt__AddPTZConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__AddVideoAnalyticsConfiguration(struct soap *soap_ptr, struct _trt__AddVideoAnalyticsConfiguration *trt__AddVideoAnalyticsConfiguration, struct _trt__AddVideoAnalyticsConfigurationResponse *trt__AddVideoAnalyticsConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__AddMetadataConfiguration(struct soap *soap_ptr, struct _trt__AddMetadataConfiguration *trt__AddMetadataConfiguration, struct _trt__AddMetadataConfigurationResponse *trt__AddMetadataConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioOutputConfiguration(struct soap *soap_ptr, struct _trt__AddAudioOutputConfiguration *trt__AddAudioOutputConfiguration, struct _trt__AddAudioOutputConfigurationResponse *trt__AddAudioOutputConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__AddAudioDecoderConfiguration(struct soap *soap_ptr, struct _trt__AddAudioDecoderConfiguration *trt__AddAudioDecoderConfiguration, struct _trt__AddAudioDecoderConfigurationResponse *trt__AddAudioDecoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoEncoderConfiguration(struct soap *soap_ptr, struct _trt__RemoveVideoEncoderConfiguration *trt__RemoveVideoEncoderConfiguration, struct _trt__RemoveVideoEncoderConfigurationResponse *trt__RemoveVideoEncoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoSourceConfiguration(struct soap *soap_ptr, struct _trt__RemoveVideoSourceConfiguration *trt__RemoveVideoSourceConfiguration, struct _trt__RemoveVideoSourceConfigurationResponse *trt__RemoveVideoSourceConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioEncoderConfiguration(struct soap *soap_ptr, struct _trt__RemoveAudioEncoderConfiguration *trt__RemoveAudioEncoderConfiguration, struct _trt__RemoveAudioEncoderConfigurationResponse *trt__RemoveAudioEncoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioSourceConfiguration(struct soap *soap_ptr, struct _trt__RemoveAudioSourceConfiguration *trt__RemoveAudioSourceConfiguration, struct _trt__RemoveAudioSourceConfigurationResponse *trt__RemoveAudioSourceConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__RemovePTZConfiguration(struct soap *soap_ptr, struct _trt__RemovePTZConfiguration *trt__RemovePTZConfiguration, struct _trt__RemovePTZConfigurationResponse *trt__RemovePTZConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveVideoAnalyticsConfiguration(struct soap *soap_ptr, struct _trt__RemoveVideoAnalyticsConfiguration *trt__RemoveVideoAnalyticsConfiguration, struct _trt__RemoveVideoAnalyticsConfigurationResponse *trt__RemoveVideoAnalyticsConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveMetadataConfiguration(struct soap *soap_ptr, struct _trt__RemoveMetadataConfiguration *trt__RemoveMetadataConfiguration, struct _trt__RemoveMetadataConfigurationResponse *trt__RemoveMetadataConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioOutputConfiguration(struct soap *soap_ptr, struct _trt__RemoveAudioOutputConfiguration *trt__RemoveAudioOutputConfiguration, struct _trt__RemoveAudioOutputConfigurationResponse *trt__RemoveAudioOutputConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__RemoveAudioDecoderConfiguration(struct soap *soap_ptr, struct _trt__RemoveAudioDecoderConfiguration *trt__RemoveAudioDecoderConfiguration, struct _trt__RemoveAudioDecoderConfigurationResponse *trt__RemoveAudioDecoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__DeleteProfile(struct soap *soap_ptr, struct _trt__DeleteProfile *trt__DeleteProfile, struct _trt__DeleteProfileResponse *trt__DeleteProfileResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurations(struct soap *soap_ptr, struct _trt__GetVideoSourceConfigurations *trt__GetVideoSourceConfigurations, struct _trt__GetVideoSourceConfigurationsResponse *trt__GetVideoSourceConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t                  SessionId = 0;
    uint32_t                  AuthValue = 0;
    uint32_t                  __VideoSource              = 0;
    GMI_RESULT                Result                     = GMI_SUCCESS;
    VideoSourceConfiguration *VideoSourceConfigurationPtr= NULL;
    SysPkgVideoSource SysVideoSource                     = {0};

    do
    {
        //user auth
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auhten fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        //should get device capabilities from sys control server.
        //get vide source num;
        //get media capabilities from sys control server
        //get video source resolution,
        //get video source frameRate
        VideoSourceConfigurationPtr = l_VideoSourceConfiguration;
        Result = SysGetVideoSource(SessionId, AuthValue, &SysVideoSource);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetVideoSource fail, Result = 0x%lx\n", Result);
            break;
        }
        ONVIF_INFO("SrcWidth   = %d\n", SysVideoSource.s_SrcWidth);
        ONVIF_INFO("SrcHeight  = %d\n", SysVideoSource.s_SrcHeight);
        ONVIF_INFO("SrcFps     = %d\n", SysVideoSource.s_SrcFps);

        __VideoSource = DEFAULT_VIDEO_SOURCES;

        trt__GetVideoSourceConfigurationsResponse->__sizeConfigurations = __VideoSource;
        trt__GetVideoSourceConfigurationsResponse->Configurations = (struct tt__VideoSourceConfiguration *)soap_malloc_zero(soap_ptr,__VideoSource*sizeof(struct tt__VideoSourceConfiguration));
        for (uint32_t Id = 0; Id < __VideoSource; Id++)
        {
            trt__GetVideoSourceConfigurationsResponse->Configurations[Id].UseCount       = VideoSourceConfigurationPtr[Id].s_UseCount;
            trt__GetVideoSourceConfigurationsResponse->Configurations[Id].Name           = (char *)soap_strdup(soap_ptr, VideoSourceConfigurationPtr[Id].s_Name);
            trt__GetVideoSourceConfigurationsResponse->Configurations[Id].token          = (char *)soap_strdup(soap_ptr, VideoSourceConfigurationPtr[Id].s_Token);
            trt__GetVideoSourceConfigurationsResponse->Configurations[Id].SourceToken    = (char *)soap_strdup(soap_ptr, VideoSourceConfigurationPtr[Id].s_SourceToken);
            trt__GetVideoSourceConfigurationsResponse->Configurations[Id].Bounds = (struct tt__IntRectangle *)soap_malloc_zero(soap_ptr, sizeof(struct tt__IntRectangle));
            trt__GetVideoSourceConfigurationsResponse->Configurations[Id].Bounds->height = SysVideoSource.s_SrcHeight;
            trt__GetVideoSourceConfigurationsResponse->Configurations[Id].Bounds->width  = SysVideoSource.s_SrcWidth;
            trt__GetVideoSourceConfigurationsResponse->Configurations[Id].Bounds->x      = VideoSourceConfigurationPtr[Id].s_Bounds.x;
            trt__GetVideoSourceConfigurationsResponse->Configurations[Id].Bounds->y      = VideoSourceConfigurationPtr[Id].s_Bounds.y;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurations(struct soap *soap_ptr, struct _trt__GetVideoEncoderConfigurations *trt__GetVideoEncoderConfigurations, struct _trt__GetVideoEncoderConfigurationsResponse *trt__GetVideoEncoderConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t                  SessionId = 0;
    uint32_t                  AuthValue = 0;
    uint32_t                   __sizeConfigurations        = 0;
    GMI_RESULT                 Result                      = GMI_SUCCESS;
    VideoEncoderConfiguration *VideoEncoderConfigurationPtr= NULL;
    uint32_t           StreamNum                           = 0;
    SysPkgEncodeCfg   *SysEncodeCfgPtr                     = NULL;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auhten fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        //should get media capabilities from sys control server
        //get stream resolution, frameRate, bitrate, profile, quality, I interval, session timeout
        VideoEncoderConfigurationPtr = l_VideoEncoderConfiguration;

        Result = SysGetEncodeStreamNum(SessionId, AuthValue, &StreamNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeStreamNum fail, Result = 0x%lx\n", Result);
            break;
        }

        if (StreamNum <= 0)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StreamNum %d incorrect\n", StreamNum);
            break;
        }

        SysEncodeCfgPtr = (SysPkgEncodeCfg*)malloc(StreamNum*sizeof(SysPkgEncodeCfg));
        if (NULL == SysEncodeCfgPtr)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "malloc fail\n");
            break;
        }

        uint32_t RespNum;
        Result = SysGetEncodeCfg(SessionId, AuthValue, SysEncodeCfgPtr, StreamNum, &RespNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeCfg fail, Result = 0x%lx\n", Result);
            break;
        }

        uint32_t Id;
        for (Id = 0; Id < RespNum; Id++)
        {
            ONVIF_INFO("VideoId        = %d\n", SysEncodeCfgPtr[Id].s_VideoId);
            ONVIF_INFO("StreamId       = %d\n", SysEncodeCfgPtr[Id].s_Flag);
            ONVIF_INFO("PicHeight      = %d\n", SysEncodeCfgPtr[Id].s_PicHeight);
            ONVIF_INFO("PicWidth       = %d\n", SysEncodeCfgPtr[Id].s_PicWidth);
            ONVIF_INFO("Compression    = %d\n", SysEncodeCfgPtr[Id].s_Compression);
            ONVIF_INFO("BitrateCtrl    = %d\n", SysEncodeCfgPtr[Id].s_BitrateCtrl);
            ONVIF_INFO("BitRateAverage = %d\n", SysEncodeCfgPtr[Id].s_BitRateAverage);
            ONVIF_INFO("BitRateDown    = %d\n", SysEncodeCfgPtr[Id].s_BitRateDown);
            ONVIF_INFO("BitRateUp      = %d\n", SysEncodeCfgPtr[Id].s_BitRateUp);
            ONVIF_INFO("FPS            = %d\n", SysEncodeCfgPtr[Id].s_FPS);
            ONVIF_INFO("Gop            = %d\n", SysEncodeCfgPtr[Id].s_Gop);
            ONVIF_INFO("Quality        = %d\n", SysEncodeCfgPtr[Id].s_Quality);
            ONVIF_INFO("Rotate         = %d\n", SysEncodeCfgPtr[Id].s_Rotate);

            VideoEncoderConfigurationPtr[Id].s_Resolution.Height          = SysEncodeCfgPtr[Id].s_PicHeight;
            VideoEncoderConfigurationPtr[Id].s_Resolution.Width           = SysEncodeCfgPtr[Id].s_PicWidth;
            VideoEncoderConfigurationPtr[Id].s_RateControl.FrameRateLimit = SysEncodeCfgPtr[Id].s_FPS;
            VideoEncoderConfigurationPtr[Id].s_RateControl.BitrateLimit   = SysEncodeCfgPtr[Id].s_BitRateUp;
            if (SYS_BRC_CBR == SysEncodeCfgPtr[Id].s_BitrateCtrl)
            {
                VideoEncoderConfigurationPtr[Id].s_RateControl.BitrateLimit = SysEncodeCfgPtr[Id].s_BitRateAverage;
            }
            else
            {
                VideoEncoderConfigurationPtr[Id].s_RateControl.BitrateLimit = SysEncodeCfgPtr[Id].s_BitRateUp;
            }

            switch (SysEncodeCfgPtr[Id].s_Compression)
            {
            case SYS_COMP_H264:
                VideoEncoderConfigurationPtr[Id].s_H264.GovLength  = SysEncodeCfgPtr[Id].s_Gop;
                break;
            case SYS_COMP_MPEG4:
                VideoEncoderConfigurationPtr[Id].s_MPEG4.GovLength = SysEncodeCfgPtr[Id].s_Gop;
                break;
            }
        }

        __sizeConfigurations = RespNum;

        trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations = __sizeConfigurations;
        trt__GetVideoEncoderConfigurationsResponse->Configurations = (struct tt__VideoEncoderConfiguration *)soap_malloc_zero(soap_ptr, trt__GetVideoEncoderConfigurationsResponse->__sizeConfigurations * sizeof(struct tt__VideoEncoderConfiguration));

        for (Id = 0; Id < __sizeConfigurations; Id++)
        {
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Resolution  = (struct tt__VideoResolution *)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoResolution));
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].RateControl = (struct tt__VideoRateControl*)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoRateControl));
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Multicast   = (struct tt__MulticastConfiguration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__MulticastConfiguration));
            switch (SysEncodeCfgPtr[Id].s_Compression)
            {
            case SYS_COMP_H264:
                trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].H264 = (struct tt__H264Configuration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__H264Configuration));
                trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].H264->GovLength   = VideoEncoderConfigurationPtr[Id].s_H264.GovLength;
                trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].H264->H264Profile = VideoEncoderConfigurationPtr[Id].s_H264.H264Profile;
                break;
            case SYS_COMP_MPEG4:
                trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].MPEG4 = (struct tt__Mpeg4Configuration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Mpeg4Configuration));
                trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].MPEG4->GovLength    = VideoEncoderConfigurationPtr[Id].s_MPEG4.GovLength ;
                trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].MPEG4->Mpeg4Profile = VideoEncoderConfigurationPtr[Id].s_MPEG4.Mpeg4Profile;
                break;
            }

            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Name        = (char*)soap_strdup(soap_ptr, VideoEncoderConfigurationPtr[Id].s_Name);
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].UseCount    = VideoEncoderConfigurationPtr[Id].s_UseCount;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].token       = (char*)soap_strdup(soap_ptr, VideoEncoderConfigurationPtr[Id].s_Token);
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Encoding    = VideoEncoderConfigurationPtr[Id].s_Encoding;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Resolution->Width  = VideoEncoderConfigurationPtr[Id].s_Resolution.Width;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Resolution->Height = VideoEncoderConfigurationPtr[Id].s_Resolution.Height;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Quality                       = VideoEncoderConfigurationPtr[Id].s_Quality;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].RateControl->FrameRateLimit   = VideoEncoderConfigurationPtr[Id].s_RateControl.FrameRateLimit;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].RateControl->EncodingInterval = VideoEncoderConfigurationPtr[Id].s_RateControl.EncodingInterval;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].RateControl->BitrateLimit     = VideoEncoderConfigurationPtr[Id].s_RateControl.BitrateLimit;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].SessionTimeout                = VideoEncoderConfigurationPtr[Id].s_SessionTimeout;

            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Multicast->Address = (struct tt__IPAddress*)soap_malloc_zero(soap_ptr, sizeof(struct tt__IPAddress));
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Multicast->Address->Type           = VideoEncoderConfigurationPtr[Id].s_Multicast.s_Address.Type;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Multicast->Address->IPv4Address    = (char_t**)soap_malloc_zero(soap_ptr, sizeof(char_t*));
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Multicast->Address->IPv4Address[0] = (char_t*)soap_strdup(soap_ptr, VideoEncoderConfigurationPtr[Id].s_Multicast.s_Address.IPv4Address);
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Multicast->Port      = VideoEncoderConfigurationPtr[Id].s_Multicast.s_Port;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Multicast->TTL       = VideoEncoderConfigurationPtr[Id].s_Multicast.s_TTL;
            trt__GetVideoEncoderConfigurationsResponse->Configurations[Id].Multicast->AutoStart = VideoEncoderConfigurationPtr[Id].s_Multicast.s_AutoStart;
        }

        if (SysEncodeCfgPtr)
        {
            free(SysEncodeCfgPtr);
            SysEncodeCfgPtr = NULL;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    if (SysEncodeCfgPtr)
    {
        free(SysEncodeCfgPtr);
        SysEncodeCfgPtr = NULL;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurations(struct soap *soap_ptr, struct _trt__GetAudioSourceConfigurations *trt__GetAudioSourceConfigurations, struct _trt__GetAudioSourceConfigurationsResponse *trt__GetAudioSourceConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurations(struct soap *soap_ptr, struct _trt__GetAudioEncoderConfigurations *trt__GetAudioEncoderConfigurations, struct _trt__GetAudioEncoderConfigurationsResponse *trt__GetAudioEncoderConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfigurations(struct soap *soap_ptr, struct _trt__GetVideoAnalyticsConfigurations *trt__GetVideoAnalyticsConfigurations, struct _trt__GetVideoAnalyticsConfigurationsResponse *trt__GetVideoAnalyticsConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurations(struct soap *soap_ptr, struct _trt__GetMetadataConfigurations *trt__GetMetadataConfigurations, struct _trt__GetMetadataConfigurationsResponse *trt__GetMetadataConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurations(struct soap *soap_ptr, struct _trt__GetAudioOutputConfigurations *trt__GetAudioOutputConfigurations, struct _trt__GetAudioOutputConfigurationsResponse *trt__GetAudioOutputConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurations(struct soap *soap_ptr, struct _trt__GetAudioDecoderConfigurations *trt__GetAudioDecoderConfigurations, struct _trt__GetAudioDecoderConfigurationsResponse *trt__GetAudioDecoderConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfiguration(struct soap *soap_ptr, struct _trt__GetVideoSourceConfiguration *trt__GetVideoSourceConfiguration, struct _trt__GetVideoSourceConfigurationResponse *trt__GetVideoSourceConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfiguration(struct soap *soap_ptr, struct _trt__GetVideoEncoderConfiguration *trt__GetVideoEncoderConfiguration, struct _trt__GetVideoEncoderConfigurationResponse *trt__GetVideoEncoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t                  SessionId = 0;
    uint32_t                  AuthValue = 0;
    GMI_RESULT                 Result                      = GMI_SUCCESS;
    VideoEncoderConfiguration *VideoEncoderConfigurationPtr= NULL;
    uint32_t            StreamNum                          = 0;
    SysPkgEncodeCfg    *SysEncodeCfgPtr                    = NULL;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auhten fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        //should get media capabilities from sys control server
        //get stream resolution, frameRate, bitrate, profile, quality, I interval, session timeout
        VideoEncoderConfigurationPtr = l_VideoEncoderConfiguration;

        Result = SysGetEncodeStreamNum(SessionId, AuthValue, &StreamNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeStreamNum fail, Result = 0x%lx\n", Result);
            break;
        }

        if (StreamNum <= 0)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StreamNum %d incorrect\n", StreamNum);
            break;
        }

        SysEncodeCfgPtr = (SysPkgEncodeCfg*)malloc(StreamNum*sizeof(SysPkgEncodeCfg));
        if (NULL == SysEncodeCfgPtr)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "malloc fail\n");
            break;
        }

        uint32_t RespNum;
        Result = SysGetEncodeCfg(SessionId, AuthValue, SysEncodeCfgPtr, StreamNum, &RespNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeCfg fail, Result = 0x%lx\n", Result);
            break;
        }

        uint32_t Id;
        for (Id = 0; Id < StreamNum; Id++)
        {
            if (0 == strcmp(trt__GetVideoEncoderConfiguration->ConfigurationToken, VideoEncoderConfigurationPtr[Id].s_Token))
            {
                ONVIF_INFO("GetStream Id %d, Token %s\n", Id,trt__GetVideoEncoderConfiguration->ConfigurationToken);
                break;
            }
        }

        if (Id >= StreamNum)
        {
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", NULL, "The argument value is invalid");
            break;
        }

        VideoEncoderConfigurationPtr[Id].s_Resolution.Height          = SysEncodeCfgPtr[Id].s_PicHeight;
        VideoEncoderConfigurationPtr[Id].s_Resolution.Width           = SysEncodeCfgPtr[Id].s_PicWidth;
        VideoEncoderConfigurationPtr[Id].s_RateControl.FrameRateLimit = SysEncodeCfgPtr[Id].s_FPS;
        VideoEncoderConfigurationPtr[Id].s_RateControl.BitrateLimit   = SysEncodeCfgPtr[Id].s_BitRateUp;
        VideoEncoderConfigurationPtr[Id].s_H264.GovLength             = SysEncodeCfgPtr[Id].s_Gop;
        if (SYS_BRC_CBR == SysEncodeCfgPtr[Id].s_BitrateCtrl)
        {
            VideoEncoderConfigurationPtr[Id].s_RateControl.BitrateLimit = SysEncodeCfgPtr[Id].s_BitRateAverage;
        }
        else
        {
            VideoEncoderConfigurationPtr[Id].s_RateControl.BitrateLimit = SysEncodeCfgPtr[Id].s_BitRateUp;
        }

        switch (SysEncodeCfgPtr[Id].s_Compression)
        {
        case SYS_COMP_H264:
            VideoEncoderConfigurationPtr[Id].s_H264.GovLength  = SysEncodeCfgPtr[Id].s_Gop;
            break;
        case SYS_COMP_MPEG4:
            VideoEncoderConfigurationPtr[Id].s_MPEG4.GovLength = SysEncodeCfgPtr[Id].s_Gop;
            break;
        }

        trt__GetVideoEncoderConfigurationResponse->Configuration = (struct tt__VideoEncoderConfiguration *)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoEncoderConfiguration));

        trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution  = (struct tt__VideoResolution *)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoResolution));
        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl = (struct tt__VideoRateControl*)soap_malloc_zero(soap_ptr, sizeof(struct tt__VideoRateControl));
        trt__GetVideoEncoderConfigurationResponse->Configuration->H264        = (struct tt__H264Configuration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__H264Configuration));
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast   = (struct tt__MulticastConfiguration*)soap_malloc_zero(soap_ptr, sizeof(struct tt__MulticastConfiguration));
        trt__GetVideoEncoderConfigurationResponse->Configuration->MPEG4       = NULL;

        trt__GetVideoEncoderConfigurationResponse->Configuration->Name        = (char*)soap_strdup(soap_ptr, VideoEncoderConfigurationPtr[Id].s_Name);
        trt__GetVideoEncoderConfigurationResponse->Configuration->UseCount    = VideoEncoderConfigurationPtr[Id].s_UseCount;
        trt__GetVideoEncoderConfigurationResponse->Configuration->token       = (char*)soap_strdup(soap_ptr, VideoEncoderConfigurationPtr[Id].s_Token);
        trt__GetVideoEncoderConfigurationResponse->Configuration->Encoding    = VideoEncoderConfigurationPtr[Id].s_Encoding;
        trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Width  = VideoEncoderConfigurationPtr[Id].s_Resolution.Width;
        trt__GetVideoEncoderConfigurationResponse->Configuration->Resolution->Height = VideoEncoderConfigurationPtr[Id].s_Resolution.Height;
        trt__GetVideoEncoderConfigurationResponse->Configuration->Quality                       = VideoEncoderConfigurationPtr[Id].s_Quality;
        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->FrameRateLimit   = VideoEncoderConfigurationPtr[Id].s_RateControl.FrameRateLimit;
        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->EncodingInterval = VideoEncoderConfigurationPtr[Id].s_RateControl.EncodingInterval;
        trt__GetVideoEncoderConfigurationResponse->Configuration->RateControl->BitrateLimit     = VideoEncoderConfigurationPtr[Id].s_RateControl.BitrateLimit;

        trt__GetVideoEncoderConfigurationResponse->Configuration->H264->GovLength               = VideoEncoderConfigurationPtr[Id].s_H264.GovLength ;
        trt__GetVideoEncoderConfigurationResponse->Configuration->H264->H264Profile             = VideoEncoderConfigurationPtr[Id].s_H264.H264Profile;
        trt__GetVideoEncoderConfigurationResponse->Configuration->SessionTimeout                = VideoEncoderConfigurationPtr[Id].s_SessionTimeout;

        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address = (struct tt__IPAddress*)soap_malloc_zero(soap_ptr, sizeof(struct tt__IPAddress));
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->Type = VideoEncoderConfigurationPtr[Id].s_Multicast.s_Address.Type;
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address = (char_t**)soap_malloc_zero(soap_ptr, sizeof(char_t*));
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Address->IPv4Address[0] = (char_t*)soap_strdup(soap_ptr, VideoEncoderConfigurationPtr[Id].s_Multicast.s_Address.IPv4Address);
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->Port = VideoEncoderConfigurationPtr[Id].s_Multicast.s_Port;
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->TTL  = VideoEncoderConfigurationPtr[Id].s_Multicast.s_TTL;
        trt__GetVideoEncoderConfigurationResponse->Configuration->Multicast->AutoStart = VideoEncoderConfigurationPtr[Id].s_Multicast.s_AutoStart;

        if (SysEncodeCfgPtr)
        {
            free(SysEncodeCfgPtr);
            SysEncodeCfgPtr = NULL;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    if (SysEncodeCfgPtr)
    {
        free(SysEncodeCfgPtr);
        SysEncodeCfgPtr = NULL;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfiguration(struct soap *soap_ptr, struct _trt__GetAudioSourceConfiguration *trt__GetAudioSourceConfiguration, struct _trt__GetAudioSourceConfigurationResponse *trt__GetAudioSourceConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfiguration(struct soap *soap_ptr, struct _trt__GetAudioEncoderConfiguration *trt__GetAudioEncoderConfiguration, struct _trt__GetAudioEncoderConfigurationResponse *trt__GetAudioEncoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoAnalyticsConfiguration(struct soap *soap_ptr, struct _trt__GetVideoAnalyticsConfiguration *trt__GetVideoAnalyticsConfiguration, struct _trt__GetVideoAnalyticsConfigurationResponse *trt__GetVideoAnalyticsConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfiguration(struct soap *soap_ptr, struct _trt__GetMetadataConfiguration *trt__GetMetadataConfiguration, struct _trt__GetMetadataConfigurationResponse *trt__GetMetadataConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfiguration(struct soap *soap_ptr, struct _trt__GetAudioOutputConfiguration *trt__GetAudioOutputConfiguration, struct _trt__GetAudioOutputConfigurationResponse *trt__GetAudioOutputConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfiguration(struct soap *soap_ptr, struct _trt__GetAudioDecoderConfiguration *trt__GetAudioDecoderConfiguration, struct _trt__GetAudioDecoderConfigurationResponse *trt__GetAudioDecoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoEncoderConfigurations(struct soap *soap_ptr, struct _trt__GetCompatibleVideoEncoderConfigurations *trt__GetCompatibleVideoEncoderConfigurations, struct _trt__GetCompatibleVideoEncoderConfigurationsResponse *trt__GetCompatibleVideoEncoderConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoSourceConfigurations(struct soap *soap_ptr, struct _trt__GetCompatibleVideoSourceConfigurations *trt__GetCompatibleVideoSourceConfigurations, struct _trt__GetCompatibleVideoSourceConfigurationsResponse *trt__GetCompatibleVideoSourceConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioEncoderConfigurations(struct soap *soap_ptr, struct _trt__GetCompatibleAudioEncoderConfigurations *trt__GetCompatibleAudioEncoderConfigurations, struct _trt__GetCompatibleAudioEncoderConfigurationsResponse *trt__GetCompatibleAudioEncoderConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioSourceConfigurations(struct soap *soap_ptr, struct _trt__GetCompatibleAudioSourceConfigurations *trt__GetCompatibleAudioSourceConfigurations, struct _trt__GetCompatibleAudioSourceConfigurationsResponse *trt__GetCompatibleAudioSourceConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleVideoAnalyticsConfigurations(struct soap *soap_ptr, struct _trt__GetCompatibleVideoAnalyticsConfigurations *trt__GetCompatibleVideoAnalyticsConfigurations, struct _trt__GetCompatibleVideoAnalyticsConfigurationsResponse *trt__GetCompatibleVideoAnalyticsConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleMetadataConfigurations(struct soap *soap_ptr, struct _trt__GetCompatibleMetadataConfigurations *trt__GetCompatibleMetadataConfigurations, struct _trt__GetCompatibleMetadataConfigurationsResponse *trt__GetCompatibleMetadataConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioOutputConfigurations(struct soap *soap_ptr, struct _trt__GetCompatibleAudioOutputConfigurations *trt__GetCompatibleAudioOutputConfigurations, struct _trt__GetCompatibleAudioOutputConfigurationsResponse *trt__GetCompatibleAudioOutputConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetCompatibleAudioDecoderConfigurations(struct soap *soap_ptr, struct _trt__GetCompatibleAudioDecoderConfigurations *trt__GetCompatibleAudioDecoderConfigurations, struct _trt__GetCompatibleAudioDecoderConfigurationsResponse *trt__GetCompatibleAudioDecoderConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoSourceConfiguration(struct soap *soap_ptr, struct _trt__SetVideoSourceConfiguration *trt__SetVideoSourceConfiguration, struct _trt__SetVideoSourceConfigurationResponse *trt__SetVideoSourceConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoEncoderConfiguration(struct soap *soap_ptr, struct _trt__SetVideoEncoderConfiguration *trt__SetVideoEncoderConfiguration, struct _trt__SetVideoEncoderConfigurationResponse *trt__SetVideoEncoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t   SessionId = 0;
    uint32_t   AuthValue = 0;
    GMI_RESULT Result = GMI_SUCCESS;
    uint32_t   Id;
    int32_t    Width = 0;
    int32_t    Height = 0;
    uint32_t   StreamNum;
    Profile   *ProfilePtr = NULL;
    SysPkgEncodeCfg *SysEncodeCfgPtr = NULL;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auhten fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        if (trt__SetVideoEncoderConfiguration->Configuration != NULL)
        {
            if (trt__SetVideoEncoderConfiguration->Configuration->Resolution != NULL)
            {
                Width = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width;
                Height = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height;
            }
        }

        ProfilePtr = l_Profile;
        for (Id = 0; Id <= MAX_STREAM_NUM; Id++)
        {
            if (strcmp(trt__SetVideoEncoderConfiguration->Configuration->token, ProfilePtr[Id].s_VideoEncoderConfiguration.s_Token) == 0)
            {
                break;
            }
        }

        if (Id >= MAX_STREAM_NUM)
        {
            ONVIF_ERROR("token %s invalid\n", trt__SetVideoEncoderConfiguration->Configuration->token);
            break;
        }

        if (((Width%2) != 0 || (Height%2) != 0 ) || ((Width*Height)%16 != 0))
        {
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:ConfigModify", "The argument value is invalid");
            break;
        }

        if (trt__SetVideoEncoderConfiguration->Configuration != NULL)
        {
            strcpy(ProfilePtr[Id].s_VideoEncoderConfiguration.s_Name, trt__SetVideoEncoderConfiguration->Configuration->Name);
            strcpy(ProfilePtr[Id].s_VideoEncoderConfiguration.s_Token, trt__SetVideoEncoderConfiguration->Configuration->token);
            ProfilePtr[Id].s_VideoEncoderConfiguration.s_UseCount = trt__SetVideoEncoderConfiguration->Configuration->UseCount;
            ProfilePtr[Id].s_VideoEncoderConfiguration.s_Quality  = trt__SetVideoEncoderConfiguration->Configuration->Quality;
            ProfilePtr[Id].s_VideoEncoderConfiguration.s_Encoding = trt__SetVideoEncoderConfiguration->Configuration->Encoding;
            ONVIF_INFO("UseCount %d\n", trt__SetVideoEncoderConfiguration->Configuration->UseCount);
            ONVIF_INFO("Quality  %f\n", trt__SetVideoEncoderConfiguration->Configuration->Quality);
            ONVIF_INFO("Encoding %d\n", trt__SetVideoEncoderConfiguration->Configuration->Encoding);
            if (trt__SetVideoEncoderConfiguration->Configuration->Resolution != NULL)
            {
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Width = trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width;
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Height= trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height;
                ONVIF_INFO("Width %d\n", trt__SetVideoEncoderConfiguration->Configuration->Resolution->Width);
                ONVIF_INFO("Height %d\n", trt__SetVideoEncoderConfiguration->Configuration->Resolution->Height);
            }
            if (trt__SetVideoEncoderConfiguration->Configuration->RateControl != NULL)
            {
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.FrameRateLimit   = trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit;
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.EncodingInterval = trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval;
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.BitrateLimit     = trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit;
                ONVIF_INFO("FrameRateLimit %d\n", trt__SetVideoEncoderConfiguration->Configuration->RateControl->FrameRateLimit);
                ONVIF_INFO("EncodingInterval %d\n", trt__SetVideoEncoderConfiguration->Configuration->RateControl->EncodingInterval);
                ONVIF_INFO("BitrateLimit %d\n", trt__SetVideoEncoderConfiguration->Configuration->RateControl->BitrateLimit);
            }
            else
            {
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.FrameRateLimit   = 0;
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.EncodingInterval = 0;
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.BitrateLimit     = 0;
            }

            if (trt__SetVideoEncoderConfiguration->Configuration->MPEG4 != NULL)
            {
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_MPEG4.Mpeg4Profile = trt__SetVideoEncoderConfiguration->Configuration->MPEG4->Mpeg4Profile;
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_MPEG4.GovLength    = trt__SetVideoEncoderConfiguration->Configuration->MPEG4->GovLength;
            }

            if (trt__SetVideoEncoderConfiguration->Configuration->H264 != NULL)
            {
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_H264.GovLength   = trt__SetVideoEncoderConfiguration->Configuration->H264->GovLength;
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_H264.H264Profile = trt__SetVideoEncoderConfiguration->Configuration->H264->H264Profile;
            }

            ProfilePtr[Id].s_VideoEncoderConfiguration.s_SessionTimeout = trt__SetVideoEncoderConfiguration->Configuration->SessionTimeout;
            if (trt__SetVideoEncoderConfiguration->Configuration->Multicast != NULL)
            {
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_Address.Type = trt__SetVideoEncoderConfiguration->Configuration->Multicast->Address->Type;
                strcpy(ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_Address.IPv4Address, *trt__SetVideoEncoderConfiguration->Configuration->Multicast->Address->IPv4Address);
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_Port = trt__SetVideoEncoderConfiguration->Configuration->Multicast->Port;
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_TTL  = trt__SetVideoEncoderConfiguration->Configuration->Multicast->TTL;
                ProfilePtr[Id].s_VideoEncoderConfiguration.s_Multicast.s_AutoStart = trt__SetVideoEncoderConfiguration->Configuration->Multicast->AutoStart;
            }

            //set encodecfg to sys_server
            Result = SysGetEncodeStreamNum(SessionId, AuthValue, &StreamNum);
            if (FAILED(Result))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeStreamNum fail, Result = 0x%lx\n", Result);
                ONVIF_ERROR("SysGetEncodeStreamNum fail, Result = 0x%lx\n", Result);
                break;
            }

            if (StreamNum <= 0)
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StreamNum %d incorrect\n", StreamNum);
                ONVIF_ERROR("StreamNum %d incorrect\n", StreamNum);
                break;
            }

            SysEncodeCfgPtr = (SysPkgEncodeCfg*)malloc(StreamNum*sizeof(SysPkgEncodeCfg));
            if (NULL == SysEncodeCfgPtr)
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "malloc fail\n");
                ONVIF_ERROR("malloc fail\n");
                break;
            }

            uint32_t RespNum;
            Result = SysGetEncodeCfg(SessionId, AuthValue, SysEncodeCfgPtr, StreamNum, &RespNum);
            if (FAILED(Result))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeCfg fail, Result = 0x%lx\n", Result);
                ONVIF_ERROR("SysGetEncodeCfg fail, Result = 0x%lx\n", Result);
                break;
            }

            if (trt__SetVideoEncoderConfiguration->Configuration->MPEG4 != NULL)
            {
                SysEncodeCfgPtr[Id].s_Compression = SYS_COMP_MPEG4;
                SysEncodeCfgPtr[Id].s_Gop         = ProfilePtr[Id].s_VideoEncoderConfiguration.s_MPEG4.GovLength;
            }
            else if (trt__SetVideoEncoderConfiguration->Configuration->H264 != NULL)
            {
                SysEncodeCfgPtr[Id].s_Compression = SYS_COMP_H264;
                SysEncodeCfgPtr[Id].s_Gop         = ProfilePtr[Id].s_VideoEncoderConfiguration.s_H264.GovLength;
            }
            else
            {
                SysEncodeCfgPtr[Id].s_Compression = SYS_COMP_MJPEG;
            }
            SysEncodeCfgPtr[Id].s_Flag        = Id;
            SysEncodeCfgPtr[Id].s_PicHeight   = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Height;
            SysEncodeCfgPtr[Id].s_PicWidth    = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Resolution.Width;
            SysEncodeCfgPtr[Id].s_Quality     = ProfilePtr[Id].s_VideoEncoderConfiguration.s_Quality;
            if (SYS_BRC_CBR == SysEncodeCfgPtr[Id].s_BitrateCtrl)
            {
                SysEncodeCfgPtr[Id].s_BitRateAverage = ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.BitrateLimit;
            }
            else
            {
                SysEncodeCfgPtr[Id].s_BitRateUp = ProfilePtr[Id].s_VideoEncoderConfiguration.s_RateControl.BitrateLimit;
            }

//temp mask, the reason of set encode config leads to rtsp pause and resume, but Dahua donot open stream
#if 0
            Result = SysSetEncodeCfg(SessionId, AuthValue, Id, &SysEncodeCfgPtr[Id]);
            if (FAILED(Result))
            {
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysSetEncodeCfg fail, Result = 0x%lx\n", Result);
                ONVIF_ERROR("SysSetEncodeCfg fail, Result = 0x%lx\n", Result);
                break;
            }
#endif            
        }

        if (SysEncodeCfgPtr)
        {
            free(SysEncodeCfgPtr);
            SysEncodeCfgPtr = NULL;
        }
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    if (SysEncodeCfgPtr)
    {
        free(SysEncodeCfgPtr);
        SysEncodeCfgPtr = NULL;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioSourceConfiguration(struct soap *soap_ptr, struct _trt__SetAudioSourceConfiguration *trt__SetAudioSourceConfiguration, struct _trt__SetAudioSourceConfigurationResponse *trt__SetAudioSourceConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioEncoderConfiguration(struct soap *soap_ptr, struct _trt__SetAudioEncoderConfiguration *trt__SetAudioEncoderConfiguration, struct _trt__SetAudioEncoderConfigurationResponse *trt__SetAudioEncoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__SetVideoAnalyticsConfiguration(struct soap *soap_ptr, struct _trt__SetVideoAnalyticsConfiguration *trt__SetVideoAnalyticsConfiguration, struct _trt__SetVideoAnalyticsConfigurationResponse *trt__SetVideoAnalyticsConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__SetMetadataConfiguration(struct soap *soap_ptr, struct _trt__SetMetadataConfiguration *trt__SetMetadataConfiguration, struct _trt__SetMetadataConfigurationResponse *trt__SetMetadataConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioOutputConfiguration(struct soap *soap_ptr, struct _trt__SetAudioOutputConfiguration *trt__SetAudioOutputConfiguration, struct _trt__SetAudioOutputConfigurationResponse *trt__SetAudioOutputConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__SetAudioDecoderConfiguration(struct soap *soap_ptr, struct _trt__SetAudioDecoderConfiguration *trt__SetAudioDecoderConfiguration, struct _trt__SetAudioDecoderConfigurationResponse *trt__SetAudioDecoderConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoSourceConfigurationOptions(struct soap *soap_ptr, struct _trt__GetVideoSourceConfigurationOptions *trt__GetVideoSourceConfigurationOptions, struct _trt__GetVideoSourceConfigurationOptionsResponse *trt__GetVideoSourceConfigurationOptionsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetVideoEncoderConfigurationOptions(struct soap *soap_ptr, struct _trt__GetVideoEncoderConfigurationOptions *trt__GetVideoEncoderConfigurationOptions, struct _trt__GetVideoEncoderConfigurationOptionsResponse *trt__GetVideoEncoderConfigurationOptionsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t   SessionId = 0;
    uint32_t   AuthValue = 0;
    GMI_RESULT                         Result                       = GMI_SUCCESS;
    int32_t                            SizeConfigurations           = 0;
    VideoEncoderConfigurationOptions  *VideoEncoderConfigOptionsPtr = NULL;
    uint32_t           StreamNum                           = 0;
    SysPkgEncodeCfg   *SysEncodeCfgPtr                     = NULL;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "user auhten fail\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        //should get meida capabilities from sys control server
        //get I interval range, quality range, resolution,
        //get stream num
        SizeConfigurations           = 1;
        VideoEncoderConfigOptionsPtr = &l_VideoEncoderConfigOptions;
        Result = SysGetEncodeStreamNum(SessionId, AuthValue, &StreamNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeStreamNum fail, Result = 0x%lx\n", Result);
            break;
        }

        if (StreamNum <= 0)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "StreamNum %d incorrect\n", StreamNum);
            break;
        }

        SysEncodeCfgPtr = (SysPkgEncodeCfg*)malloc(StreamNum*sizeof(SysPkgEncodeCfg));
        if (NULL == SysEncodeCfgPtr)
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "malloc fail\n");
            break;
        }

        uint32_t RespNum;
        Result = SysGetEncodeCfg(SessionId, AuthValue, SysEncodeCfgPtr, StreamNum, &RespNum);
        if (FAILED(Result))
        {
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysGetEncodeCfg fail, Result = 0x%lx\n", Result);
            break;
        }

        for (uint32_t Id = 0; Id < RespNum; Id++)
        {
            ONVIF_INFO("VideoId 	   = %d\n", SysEncodeCfgPtr[Id].s_VideoId);
            ONVIF_INFO("StreamId	   = %d\n", SysEncodeCfgPtr[Id].s_Flag);
            ONVIF_INFO("PicHeight	   = %d\n", SysEncodeCfgPtr[Id].s_PicHeight);
            ONVIF_INFO("PicWidth	   = %d\n", SysEncodeCfgPtr[Id].s_PicWidth);
            ONVIF_INFO("Compression    = %d\n", SysEncodeCfgPtr[Id].s_Compression);
            ONVIF_INFO("BitrateCtrl    = %d\n", SysEncodeCfgPtr[Id].s_BitrateCtrl);
            ONVIF_INFO("BitRateAverage = %d\n", SysEncodeCfgPtr[Id].s_BitRateAverage);
            ONVIF_INFO("BitRateDown    = %d\n", SysEncodeCfgPtr[Id].s_BitRateDown);
            ONVIF_INFO("BitRateUp	   = %d\n", SysEncodeCfgPtr[Id].s_BitRateUp);
            ONVIF_INFO("FPS 		   = %d\n", SysEncodeCfgPtr[Id].s_FPS);
            ONVIF_INFO("Gop 		   = %d\n", SysEncodeCfgPtr[Id].s_Gop);
            ONVIF_INFO("Quality 	   = %d\n", SysEncodeCfgPtr[Id].s_Quality);
            ONVIF_INFO("Rotate		   = %d\n", SysEncodeCfgPtr[Id].s_Rotate);
        }

        trt__GetVideoEncoderConfigurationOptionsResponse->Options = (struct tt__VideoEncoderConfigurationOptions *)soap_malloc_zero(soap_ptr, SizeConfigurations * sizeof(struct tt__VideoEncoderConfigurationOptions));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].QualityRange                = (struct tt__IntRange *)soap_malloc_zero(soap_ptr, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264                        = (struct tt__H264Options *)soap_malloc_zero(soap_ptr, sizeof(struct tt__H264Options));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->__sizeResolutionsAvailable  = StreamNum;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->__sizeH264ProfilesSupported = StreamNum;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->EncodingIntervalRange = (struct tt__IntRange *)soap_malloc_zero(soap_ptr, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->FrameRateRange        = (struct tt__IntRange *)soap_malloc_zero(soap_ptr, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->GovLengthRange        = (struct tt__IntRange *)soap_malloc_zero(soap_ptr, sizeof(struct tt__IntRange));
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->H264ProfilesSupported = (enum tt__H264Profile *)soap_malloc_zero(soap_ptr, trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->__sizeH264ProfilesSupported * sizeof(enum tt__H264Profile));

        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->ResolutionsAvailable = (struct tt__VideoResolution *)soap_malloc_zero(soap_ptr, trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->__sizeResolutionsAvailable * sizeof(struct tt__VideoResolution));
        //this is incorrect, should get media capabilities
        for (int32_t Id = 0; Id <  trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->__sizeH264ProfilesSupported; Id++)
        {
            trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->ResolutionsAvailable[Id].Height = SysEncodeCfgPtr[Id].s_PicHeight;
            trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->ResolutionsAvailable[Id].Width  = SysEncodeCfgPtr[Id].s_PicWidth;
        }

        for (int32_t Id = 0; Id < trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->__sizeH264ProfilesSupported; Id++)
        {
            trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->H264ProfilesSupported[Id] = tt__H264Profile__Main;
        }

        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].QualityRange->Max                = VideoEncoderConfigOptionsPtr->s_QualityRange.Max;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].QualityRange->Min                = VideoEncoderConfigOptionsPtr->s_QualityRange.Min;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->EncodingIntervalRange->Max = VideoEncoderConfigOptionsPtr->s_H264Options.s_EncodingIntervalRange.Max;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->EncodingIntervalRange->Min = VideoEncoderConfigOptionsPtr->s_H264Options.s_EncodingIntervalRange.Min;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->FrameRateRange->Max        = VideoEncoderConfigOptionsPtr->s_H264Options.s_FrameRateRange.Max;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->FrameRateRange->Min        = VideoEncoderConfigOptionsPtr->s_H264Options.s_FrameRateRange.Min;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->GovLengthRange->Max        = VideoEncoderConfigOptionsPtr->s_H264Options.s_GovLengthRange.Max;
        trt__GetVideoEncoderConfigurationOptionsResponse->Options[0].H264->GovLengthRange->Min        = VideoEncoderConfigOptionsPtr->s_H264Options.s_GovLengthRange.Min;

        if (SysEncodeCfgPtr)
        {
            free(SysEncodeCfgPtr);
            SysEncodeCfgPtr = NULL;
        }
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    if (SysEncodeCfgPtr)
    {
        free(SysEncodeCfgPtr);
        SysEncodeCfgPtr = NULL;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioSourceConfigurationOptions(struct soap *soap_ptr, struct _trt__GetAudioSourceConfigurationOptions *trt__GetAudioSourceConfigurationOptions, struct _trt__GetAudioSourceConfigurationOptionsResponse *trt__GetAudioSourceConfigurationOptionsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioEncoderConfigurationOptions(struct soap *soap_ptr, struct _trt__GetAudioEncoderConfigurationOptions *trt__GetAudioEncoderConfigurationOptions, struct _trt__GetAudioEncoderConfigurationOptionsResponse *trt__GetAudioEncoderConfigurationOptionsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetMetadataConfigurationOptions(struct soap *soap_ptr, struct _trt__GetMetadataConfigurationOptions *trt__GetMetadataConfigurationOptions, struct _trt__GetMetadataConfigurationOptionsResponse *trt__GetMetadataConfigurationOptionsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioOutputConfigurationOptions(struct soap *soap_ptr, struct _trt__GetAudioOutputConfigurationOptions *trt__GetAudioOutputConfigurationOptions, struct _trt__GetAudioOutputConfigurationOptionsResponse *trt__GetAudioOutputConfigurationOptionsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetAudioDecoderConfigurationOptions(struct soap *soap_ptr, struct _trt__GetAudioDecoderConfigurationOptions *trt__GetAudioDecoderConfigurationOptions, struct _trt__GetAudioDecoderConfigurationOptionsResponse *trt__GetAudioDecoderConfigurationOptionsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetGuaranteedNumberOfVideoEncoderInstances(struct soap *soap_ptr, struct _trt__GetGuaranteedNumberOfVideoEncoderInstances *trt__GetGuaranteedNumberOfVideoEncoderInstances, struct _trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse *trt__GetGuaranteedNumberOfVideoEncoderInstancesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return SOAP_OK;
}


