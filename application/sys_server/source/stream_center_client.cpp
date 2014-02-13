
#include "stream_center_client.h"
#include "log.h"
#include "gmi_system_headers.h"
#include "config_file_manager.h"
#include "sys_env_types.h"
#include "gmi_media_ctrl.h"

static int16_t  l_ExposureGainMaxTable[] = {30, 36, 42, 48, 54, 60};
static uint16_t l_ReslutionTable[][2] =
{
    {RES_1080P_WIDTH, RES_1080P_HEIGHT},
    {RES_720P_WIDTH, RES_720P_HEIGHT},
    {RES_D1_PAL_WIDTH, RES_D1_PAL_HEIGHT},
    {RES_D1_NTSC_WIDTH, RES_D1_NTSC_HEIGHT},
    {RES_CIF_NTSC_WIDTH, RES_CIF_NTSC_HEIGHT},
    {RES_CIF_PAL_WIDTH, RES_CIF_PAL_HEIGHT},
    {RES_QCIF_PAL_WIDTH, RES_QCIF_PAL_HEIGHT},
    {RES_QVGA_WIDTH, RES_QVGA_HEIGHT}
};

//OSD Font Size Map Table
static int32_t l_OSD_FontSize[][2] =
{
    {RES_1080P_HEIGHT, OSD_FONT_SIZE_BIGNESS},
    {RES_720P_HEIGHT, OSD_FONT_SIZE_BIG},
    {RES_D1_PAL_HEIGHT, OSD_FONT_SIZE_MEDIUM},
    {RES_D1_NTSC_HEIGHT, OSD_FONT_SIZE_MEDIUM},
    {RES_CIF_PAL_HEIGHT, OSD_FONT_SIZE_SMALLNESS},
    {RES_CIF_NTSC_HEIGHT, OSD_FONT_SIZE_SMALLNESS},
    {RES_QCIF_PAL_HEIGHT, OSD_FONT_SIZE_SMALLNESS},
    {RES_QVGA_HEIGHT, OSD_FONT_SIZE_SMALLNESS}
};

StreamCenterClient::StreamCenterClient()
{
}


StreamCenterClient::~StreamCenterClient()
{
}


GMI_RESULT StreamCenterClient::Initialize(uint16_t LocalClientPort, uint16_t LocalServerPort, uint16_t RTSP_ServerPort, size_t SessionBufferSize)
{
    SYS_INFO("%s:%s in.........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    //GMI_RESULT Result = GMI_SUCCESS;

    //information
    SYS_INFO("LocalClientPort  = %d\n", LocalClientPort);
    SYS_INFO("LocalServerPort  = %d\n", LocalServerPort);
    SYS_INFO("SessionBufferSize= %d\n", SessionBufferSize);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "LocalClientPort  = %d\n", LocalClientPort);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "LocalServerPort  = %d\n", LocalServerPort);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "SessionBufferSize= %d\n", SessionBufferSize);

    //Result = m_MediaTransport.Initialize(RTSP_ServerPort);
    //if (FAILED(Result))
    //{
    //    SYS_ERROR("m_MediaTransport.Initialize failed, Result = 0x%lx\n", Result);
    //    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaTransport.Initialize failed, Result = 0x%lx\n", Result);
    //    return Result;
    //}

    //Result = m_MediaCenter.Initialize(inet_addr("127.0.0.1"), htons(LocalClientPort), SessionBufferSize, inet_addr("127.0.0.1"), htons(LocalServerPort));
    //if (FAILED(Result))
    //{
    //m_MediaTransport.Deinitialize();
    //     SYS_ERROR("m_MediaCenter.Initialize failed\n");
    //     DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter.Initialize failed\n");
    //     return Result;
    // }

    m_ClientPort            = LocalClientPort;
    m_ServerPort            = LocalServerPort;
    m_UDP_SessionBufferSize = SessionBufferSize;
    SYS_INFO("%s:%s normal out.........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out.........\n", __FILE__, __func__);

    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::Deinitialize()
{
    //GMI_RESULT Result = GMI_SUCCESS;

    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    //Result = m_MediaCenter.Deinitialize();
    //if (FAILED(Result))
    //{
    //    SYS_ERROR("m_MediaCenter.Deinitalize failed, Result = 0x%lx\n", Result);
    //    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter.Deinitalize failed, Result = 0x%lx\n", Result);
    //    return Result;
    //}

    //Result = m_MediaTransport.Deinitialize();
    //if (FAILED(Result))
    //{
    //    SYS_ERROR("m_MediaTransport.Deinitialize failed, Result = 0x%lx\n", Result);
    //    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaTransport.Deinitialize failed, Result = 0x%lx\n", Result);
    //    return Result;
    //}

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::GeneralParamSet(General_Param ParamType, void_t *GeneralParamPtr)
{
	SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
	if (NULL == GeneralParamPtr)
	{
		SYS_ERROR("GeneralParamPtr is null\n");
		return GMI_INVALID_PARAMETER;
	}

	GMI_RESULT Result;
	int32_t Type = (int32_t)ParamType;
	
	switch (Type)
	{
	case GEN_PARAM_AUTO:
		SysPkgComponents SysComponents;
		GeneralParam_Capability Capability;

		memcpy(&SysComponents, GeneralParamPtr, sizeof(SysPkgComponents));
		memset(&Capability, 0, sizeof(GeneralParam_Capability));
		Capability.s_CpuType = (SysCpuIdType)SysComponents.s_CpuId;
		Capability.s_SensorType = (SysVinIdType)SysComponents.s_SensorId;
		Capability.s_ZoomLensType = (SysLensIdType)SysComponents.s_ZoomLensId;
		Capability.s_EncodeBoardType = (SysEncoderBoardIdType)SysComponents.s_BoardId;	
		Result = GMI_GeneralParamSetConfig(GEN_PARAM_AUTO, (void_t*)&Capability);
		if (FAILED(Result))
		{
			SYS_ERROR("GMI_GeneralParamSetConfig ParamType %d fail, Result = 0x%lx\n", ParamType, Result);
			return Result;
		}
		break;
	case GEN_PARAM_IRCUT:
		GeneralParam_Ircut Ircut;
		memcpy(&Ircut, GeneralParamPtr, sizeof(GeneralParam_Ircut));
		printf("Ircut->s_AdcMode %d\n", Ircut.s_AdcMode);
		Result = GMI_GeneralParamSetConfig(GEN_PARAM_IRCUT, (void_t*)&Ircut);
		if (FAILED(Result))
		{
			SYS_ERROR("GMI_GeneralParamSetConfig ParamType %d fail, Result = 0x%lx\n", ParamType, Result);
			return Result;
		}
		break;
	default:
		return 	GMI_INVALID_PARAMETER;
	}
	SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
	return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::StartCodec(boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *CodecHandle)
{
    SYS_INFO("%s in.......\n", __func__);
    GMI_RESULT Result = GMI_SUCCESS;

    Result = m_MediaCenter.CreateCodec(EncodeMode, SourceId, MediaId, MediaType, CodecType, CodecParameter, CodecParameterLength, CodecHandle);
    if (FAILED(Result))
    {
        SYS_ERROR("create codec fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_MediaCenter.StartCodec(*CodecHandle);
    if (FAILED(Result))
    {
        SYS_ERROR("start codec2 fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SYS_INFO("%s out.......\n", __func__);
    return Result;
}


GMI_RESULT StreamCenterClient::Start(uint32_t SourceId, uint32_t StreamId, uint32_t MediaType, uint32_t CodecType, boolean_t EncMode, void_t * CodecParameter, size_t CodecParameterLength, FD_HANDLE * Handle)
{
    GMI_RESULT     Result = GMI_SUCCESS;

    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    if (MediaType == MEDIA_VIDEO)
    {
        VideoEncodeParam *VidEncParam = NULL;

        VidEncParam = (VideoEncodeParam*)CodecParameter;

        MediaHandle* MediaHd = BaseMemoryManager::Instance().New<MediaHandle>();
        MediaHd->s_SourceId = SourceId;
        MediaHd->s_StreamId = StreamId;
        MediaHd->s_MediaType= MediaType;
        MediaHd->s_CodecType= CodecType;

        SYS_INFO("SourceId         = %d\n", SourceId);
        SYS_INFO("StreamId         = %d\n", StreamId);
        SYS_INFO("MediaType        = %d\n", MediaType);
        SYS_INFO("CodeType         = %d\n", CodecType);
        SYS_INFO("PicHeight        = %d\n", VidEncParam->s_EncodeHeight);
        SYS_INFO("PicWidth         = %d\n", VidEncParam->s_EncodeWidth);
        SYS_INFO("FrameRate        = %d\n", VidEncParam->s_FrameRate);
        SYS_INFO("Gop              = %d\n", VidEncParam->s_FrameInterval);
        SYS_INFO("BitRateType      = %d\n", VidEncParam->s_BitRateType);
        SYS_INFO("BitRateAverage   = %d\n", VidEncParam->s_BitRateAverage);
        SYS_INFO("BitRateUp        = %d\n", VidEncParam->s_BitRateUp);
        SYS_INFO("BitRateDown      = %d\n", VidEncParam->s_BitRateDown);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "SourceId         = %d\n", SourceId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "StreamId         = %d\n", StreamId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "MediaType        = %d\n", MediaType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "CodeType         = %d\n", CodecType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "PicHeight        = %d\n", VidEncParam->s_EncodeHeight);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "PicWidth         = %d\n", VidEncParam->s_EncodeWidth);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "FrameRate        = %d\n", VidEncParam->s_FrameRate);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Gop              = %d\n", VidEncParam->s_FrameInterval);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateType      = %d\n", VidEncParam->s_BitRateType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateAverage   = %d\n", VidEncParam->s_BitRateAverage);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateUp        = %d\n", VidEncParam->s_BitRateUp);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateDown      = %d\n", VidEncParam->s_BitRateDown);

        Result = StartCodec( true, SourceId, StreamId, MediaType, CodecType, VidEncParam, sizeof(VideoEncodeParam), &(MediaHd->s_Encode));
        if (FAILED(Result))
        {
            SYS_ERROR("m_MediaCenter.Start failed, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter.Start failed, Result = 0x%lx\n", Result);
            return Result;
        }

#if 0
        Result = m_MediaTransport.Start( true, SourceId, StreamId, MediaType, CodecType, VidEncParam, sizeof(VideoEncodeParam), &(MediaHd->s_Transport));
        if (FAILED(Result))
        {
            SYS_ERROR("m_MediaTransport.Start failed, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaTransport.Start failed, Result = 0x%lx\n", Result);
            return Result;
        }
#endif

        *Handle = (FD_HANDLE)MediaHd;
    }
    else if (MediaType == MEDIA_AUDIO)
    {
        if (EncMode)
        {
            AudioEncParam *AudEncParam = (AudioEncParam*)CodecParameter;

            MediaHandle* MediaHd = BaseMemoryManager::Instance().New<MediaHandle>();
            MediaHd->s_SourceId = 0;
            MediaHd->s_StreamId = StreamId;
            MediaHd->s_MediaType= MediaType;
            MediaHd->s_CodecType= CodecType;
            MediaHd->s_EncMode  = EncMode;

            SYS_INFO("Audio Encode\n");
            SYS_INFO("Codec        = %d\n", AudEncParam->s_Codec);
            SYS_INFO("SampleFreq   = %d\n", AudEncParam->s_SampleFreq);
            SYS_INFO("BitWidth     = %d\n", AudEncParam->s_BitWidth);
            SYS_INFO("ChannelNum   = %d\n", AudEncParam->s_ChannelNum);
            SYS_INFO("FrameRate    = %d\n", AudEncParam->s_FrameRate);
            SYS_INFO("BitRate      = %d\n", AudEncParam->s_BitRate);
            SYS_INFO("CapVolume    = %d\n", AudEncParam->s_Volume);
            SYS_INFO("AecFlag      = %d\n", AudEncParam->s_AecFlag);
            SYS_INFO("AecDelayTime = %d\n", AudEncParam->s_AecDelayTime);

            Result = StartCodec(true, 0, StreamId, MediaType, CodecType, AudEncParam, sizeof(AudioEncParam), &(MediaHd->s_Encode));
            if (FAILED(Result))
            {
                SYS_ERROR("m_MediaCenter.Start audio encode failed, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter.Start encode failed, Result = 0x%lx\n", Result);
                return Result;
            }

            *Handle = (FD_HANDLE)MediaHd;
        }
        else
        {
            AudioDecParam *AudDecParam = (AudioDecParam*)CodecParameter;

            MediaHandle* MediaHd = BaseMemoryManager::Instance().New<MediaHandle>();
            MediaHd->s_SourceId = 0;
            MediaHd->s_StreamId = StreamId;
            MediaHd->s_MediaType= MediaType;
            MediaHd->s_CodecType= CodecType;
            MediaHd->s_EncMode  = EncMode;

            SYS_INFO("Audio Decode\n");
            SYS_INFO("Codec        = %d\n", AudDecParam->s_Codec);
            SYS_INFO("SampleFreq   = %d\n", AudDecParam->s_SampleFreq);
            SYS_INFO("BitWidth     = %d\n", AudDecParam->s_BitWidth);
            SYS_INFO("ChannelNum   = %d\n", AudDecParam->s_ChannelNum);
            SYS_INFO("FrameRate    = %d\n", AudDecParam->s_FrameRate);
            SYS_INFO("BitRate      = %d\n", AudDecParam->s_BitRate);
            SYS_INFO("CapVolume    = %d\n", AudDecParam->s_Volume);
            SYS_INFO("AecFlag      = %d\n", AudDecParam->s_AecFlag);
            SYS_INFO("AecDelayTime = %d\n", AudDecParam->s_AecDelayTime);

            Result = StartCodec(false, 0, StreamId, MediaType, CodecType, AudDecParam, sizeof(AudioDecParam), &(MediaHd->s_Encode));
            if (FAILED(Result))
            {
                SYS_ERROR("MediaCenter.Start audio decode failed, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenter.Start audio decode failed, Result = 0x%lx\n", Result);
                return Result;
            }

            *Handle = (FD_HANDLE)MediaHd;
        }
    }
    else
    {
        SYS_ERROR("MediaType %d invalid\n", MediaType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaType %d invalid\n", MediaType);
        return GMI_INVALID_PARAMETER;
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::StopCodec(FD_HANDLE& CodecHandle)
{
    SYS_INFO("%s in........\n", __func__);
    if (NULL == CodecHandle)
    {
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_SUCCESS;

    Result = m_MediaCenter.StopCodec(CodecHandle);
    if (FAILED(Result))
    {
        SYS_ERROR("stop codec2 fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_MediaCenter.DestroyCodec(CodecHandle);
    if (FAILED(Result))
    {
        SYS_ERROR("destroy code fail, Result = 0x%lx\n", Result);
        return Result;
    }
    SYS_INFO("%s out.........\n", __func__);

    return Result;
}


GMI_RESULT StreamCenterClient::Stop(FD_HANDLE Handle)
{
    GMI_RESULT   Result = GMI_SUCCESS;
    MediaHandle *MediaHd;

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    if (Handle)
    {
        MediaHd = (MediaHandle*)Handle;
#if 0
        if (MEDIA_VIDEO == MediaHd->s_MediaType)
        {
            Result = m_MediaTransport.Stop(MediaHd->s_Transport);
            if (FAILED(Result))
            {
                SYS_ERROR("m_MediaTransport.Stop fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaTransport.Stop, Result = 0x%lx\n", Result);
                return Result;
            }
        }
#endif

        Result = StopCodec(MediaHd->s_Encode);
        if (FAILED(Result))
        {
            SYS_ERROR("m_MediaCenter.Stop fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter.Stop fail, Result = 0x%lx\n", Result);
            return Result;
        }

        BaseMemoryManager::Instance().Delete<MediaHandle>(MediaHd);
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::Create(uint32_t SourceId, uint32_t StreamId, uint32_t MediaType, uint32_t CodecType, boolean_t EncMode, void_t * CodecParameter, size_t CodecParameterLength, FD_HANDLE * Handle)
{
    GMI_RESULT	   Result = GMI_SUCCESS;

    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    if (MediaType == MEDIA_VIDEO)
    {
        if (StreamId >= MAX_VIDEO_STREAM_NUM)
        {
            SYS_ERROR("streamId %d incorrect\n", StreamId);
            return GMI_INVALID_PARAMETER;
        }

        VideoEncodeParam *VidEncParam = NULL;

        VidEncParam = (VideoEncodeParam*)CodecParameter;

        MediaHandle* MediaHd = BaseMemoryManager::Instance().New<MediaHandle>();
        MediaHd->s_SourceId = SourceId;
        MediaHd->s_StreamId = StreamId;
        MediaHd->s_MediaType= MediaType;
        MediaHd->s_CodecType= CodecType;

        SYS_INFO("SourceId		   = %d\n", SourceId);
        SYS_INFO("StreamId		   = %d\n", StreamId);
        SYS_INFO("MediaType        = %d\n", MediaType);
        SYS_INFO("CodeType		   = %d\n", CodecType);
        SYS_INFO("PicHeight        = %d\n", VidEncParam->s_EncodeHeight);
        SYS_INFO("PicWidth		   = %d\n", VidEncParam->s_EncodeWidth);
        SYS_INFO("FrameRate        = %d\n", VidEncParam->s_FrameRate);
        SYS_INFO("Gop			   = %d\n", VidEncParam->s_FrameInterval);
        SYS_INFO("BitRateType	   = %d\n", VidEncParam->s_BitRateType);
        SYS_INFO("BitRateAverage   = %d\n", VidEncParam->s_BitRateAverage);
        SYS_INFO("BitRateUp 	   = %d\n", VidEncParam->s_BitRateUp);
        SYS_INFO("BitRateDown	   = %d\n", VidEncParam->s_BitRateDown);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "SourceId		  = %d\n", SourceId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "StreamId		  = %d\n", StreamId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "MediaType		  = %d\n", MediaType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "CodeType		  = %d\n", CodecType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "PicHeight		  = %d\n", VidEncParam->s_EncodeHeight);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "PicWidth		  = %d\n", VidEncParam->s_EncodeWidth);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "FrameRate		  = %d\n", VidEncParam->s_FrameRate);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Gop			  = %d\n", VidEncParam->s_FrameInterval);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateType	  = %d\n", VidEncParam->s_BitRateType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateAverage   = %d\n", VidEncParam->s_BitRateAverage);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateUp		  = %d\n", VidEncParam->s_BitRateUp);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateDown	  = %d\n", VidEncParam->s_BitRateDown);

        Result = CheckVideoEncodeConfiguration(VidEncParam);
        if (FAILED(Result))
        {
            SYS_ERROR("CheckVideoEncodeConfiguration failed, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CheckVideoEncodeConfiguration failed, Result = 0x%lx\n", Result);
            return Result;
        }

        Result = m_MediaCenter.CreateCodec(true, SourceId, StreamId, MediaType, CodecType, VidEncParam, sizeof(VideoEncodeParam), &(MediaHd->s_Encode));
        if (FAILED(Result))
        {
            SYS_ERROR("m_MediaCenter.CreateCodec failed, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter.CreateCodec failed, Result = 0x%lx\n", Result);
            return Result;
        }

        *Handle = (FD_HANDLE)MediaHd;
        memcpy(&m_VideoEncParam[StreamId], VidEncParam, sizeof(VideoEncodeParam));
    }
    else if (MediaType == MEDIA_AUDIO)
    {
        if (EncMode)
        {
            AudioEncParam *AudEncParam = (AudioEncParam*)CodecParameter;

            MediaHandle* MediaHd = BaseMemoryManager::Instance().New<MediaHandle>();
            MediaHd->s_SourceId = 0;
            MediaHd->s_StreamId = StreamId;
            MediaHd->s_MediaType= MediaType;
            MediaHd->s_CodecType= CodecType;
            MediaHd->s_EncMode  = EncMode;

            SYS_INFO("Codec 	   = %d\n", AudEncParam->s_Codec);
            SYS_INFO("SampleFreq   = %d\n", AudEncParam->s_SampleFreq);
            SYS_INFO("BitWidth	   = %d\n", AudEncParam->s_BitWidth);
            SYS_INFO("ChannelNum   = %d\n", AudEncParam->s_ChannelNum);
            SYS_INFO("FrameRate    = %d\n", AudEncParam->s_FrameRate);
            SYS_INFO("BitRate	   = %d\n", AudEncParam->s_BitRate);
            SYS_INFO("CapVolume    = %d\n", AudEncParam->s_Volume);
            SYS_INFO("AecFlag	   = %d\n", AudEncParam->s_AecFlag);
            SYS_INFO("AecDelayTime = %d\n", AudEncParam->s_AecDelayTime);

            Result = CheckAudioEncodeConfiguration(AudEncParam);
            if (FAILED(Result))
            {
                SYS_ERROR("CheckAudioEncodeConfiguration failed, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CheckAudioEncodeConfiguration failed, Result = 0x%lx\n", Result);
                return Result;
            }

            Result = m_MediaCenter.CreateCodec(true, 0, StreamId, MediaType, CodecType, AudEncParam, sizeof(AudioEncParam), &(MediaHd->s_Encode));
            if (FAILED(Result))
            {
                SYS_ERROR("m_MediaCenter.CreateCodec audio encode failed, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter.CreateCodec encode failed, Result = 0x%lx\n", Result);
                return Result;
            }

            *Handle = (FD_HANDLE)MediaHd;
        }
        else
        {
            AudioDecParam *AudDecParam = (AudioDecParam*)CodecParameter;

            MediaHandle* MediaHd = BaseMemoryManager::Instance().New<MediaHandle>();
            MediaHd->s_SourceId = 0;
            MediaHd->s_StreamId = StreamId;
            MediaHd->s_MediaType= MediaType;
            MediaHd->s_CodecType= CodecType;
            MediaHd->s_EncMode  = EncMode;

            SYS_INFO("Codec 	   = %d\n", AudDecParam->s_Codec);
            SYS_INFO("SampleFreq   = %d\n", AudDecParam->s_SampleFreq);
            SYS_INFO("BitWidth	   = %d\n", AudDecParam->s_BitWidth);
            SYS_INFO("ChannelNum   = %d\n", AudDecParam->s_ChannelNum);
            SYS_INFO("FrameRate    = %d\n", AudDecParam->s_FrameRate);
            SYS_INFO("BitRate	   = %d\n", AudDecParam->s_BitRate);
            SYS_INFO("CapVolume    = %d\n", AudDecParam->s_Volume);
            SYS_INFO("AecFlag	   = %d\n", AudDecParam->s_AecFlag);
            SYS_INFO("AecDelayTime = %d\n", AudDecParam->s_AecDelayTime);

            Result = CheckAudioDecodeConfiguration(AudDecParam);
            if (FAILED(Result))
            {
                SYS_ERROR("CheckAudioDecodeConfiguration failed, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CheckAudioDecodeConfiguration failed, Result = 0x%lx\n", Result);
                return Result;
            }

            Result = m_MediaCenter.CreateCodec(false, 0, StreamId, MediaType, CodecType, AudDecParam, sizeof(AudioDecParam), &(MediaHd->s_Encode));
            if (FAILED(Result))
            {
                SYS_ERROR("MediaCenter.CreateCodec audio decode failed, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaCenter.CreateCodec audio decode failed, Result = 0x%lx\n", Result);
                return Result;
            }

            *Handle = (FD_HANDLE)MediaHd;
        }
    }
    else
    {
        SYS_ERROR("MediaType %d invalid\n", MediaType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "MediaType %d invalid\n", MediaType);
        return GMI_INVALID_PARAMETER;
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::Destroy(FD_HANDLE Handle)
{
    GMI_RESULT   Result = GMI_SUCCESS;
    MediaHandle *MediaHd;

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    if (Handle)
    {
        MediaHd = (MediaHandle*)Handle;
#if 0
        if (MEDIA_VIDEO == MediaHd->s_MediaType)
        {
            Result = m_MediaTransport.Stop(MediaHd->s_Transport);
            if (FAILED(Result))
            {
                SYS_ERROR("m_MediaTransport.Stop fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaTransport.Stop, Result = 0x%lx\n", Result);
                return Result;
            }
        }
#endif

        Result = m_MediaCenter.DestroyCodec(MediaHd->s_Encode);
        if (FAILED(Result))
        {
            SYS_ERROR("m_MediaCenter.DestroyCodec fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter.DestroyCodec fail, Result = 0x%lx\n", Result);
            return Result;
        }

        BaseMemoryManager::Instance().Delete<MediaHandle>(MediaHd);
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::Start2(FD_HANDLE Handle)
{
    MediaHandle *MediaHd = (MediaHandle*)Handle;
    GMI_RESULT Result = m_MediaCenter.StartCodec(MediaHd->s_Encode);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter.StartCodec2 fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter.StartCodec2 fail, Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::Stop2(FD_HANDLE Handle)
{
    MediaHandle *MediaHd = (MediaHandle*)Handle;
    GMI_RESULT Result = m_MediaCenter.StopCodec(MediaHd->s_Encode);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter.StopCodec2 fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter.StopCodec2 fail, Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::OpenVideoInOutDevice(uint32_t VidSourceId, uint32_t VidChanId, FD_HANDLE *Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.OpenVinVoutDevice(VidSourceId, VidChanId, Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter OpenVinVoutDevice failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter OpenVinVoutDevice failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CloseVideoInOutDevice(FD_HANDLE Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.CloseVinVoutDevice(Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter CloseVideoInOutDevice failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter CloseVideoInOutDevice failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::OpenImageDevice(uint32_t VidSourceId, uint32_t VidChanId, FD_HANDLE *Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.OpenImageDevice(VidSourceId, VidChanId, Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter OpenImageDevice failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter OpenImageDevice failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CloseImageDevice(FD_HANDLE Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.CloseImageDevice(Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter CloseImageDevice failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter CloseImageDevice failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::GetCodecConfiguration(FD_HANDLE Handle, void_t *CodecParameter, size_t CodecParameterLength)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    MediaHandle *MediaHd;

    if (Handle)
    {
        MediaHd = (MediaHandle*)Handle;
        if (MEDIA_VIDEO == MediaHd->s_MediaType)
        {
            VideoEncodeParam EncParam;
            memset(&EncParam, 0, sizeof(VideoEncodeParam));
            size_t ParamLength = sizeof(VideoEncodeParam);

            GMI_RESULT Result = m_MediaCenter.GetCodecConfig(MediaHd->s_Encode, &EncParam, &ParamLength );
            if (FAILED(Result))
            {
                SYS_ERROR("GetCodecConfig fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetCodecConfig fail, Result = 0x%lx\n", Result);
                return Result;
            }

            size_t MinParamLength =  CodecParameterLength > sizeof(VideoEncodeParam) ? sizeof(VideoEncodeParam) : CodecParameterLength;

            memcpy(CodecParameter, &EncParam, MinParamLength);

            SYS_INFO( "StreamId       = %d\n", EncParam.s_StreamId );
            SYS_INFO( "EncodeType     = %d\n", EncParam.s_EncodeType );
            SYS_INFO( "FrameRate      = %d\n", EncParam.s_FrameRate );
            SYS_INFO( "EncodeWidth    = %d\n", EncParam.s_EncodeWidth );
            SYS_INFO( "EncodeHeight   = %d\n", EncParam.s_EncodeHeight );
            SYS_INFO( "BitRateType    = %d\n", EncParam.s_BitRateType );
            SYS_INFO( "BitRateAverage = %d\n", EncParam.s_BitRateAverage );
            SYS_INFO( "BitRateUp      = %d\n", EncParam.s_BitRateUp );
            SYS_INFO( "BitRateDown    = %d\n", EncParam.s_BitRateDown );
            SYS_INFO( "FrameInterval  = %d\n", EncParam.s_FrameInterval );
            SYS_INFO( "EncodeQulity   = %d\n", EncParam.s_EncodeQulity );
            SYS_INFO( "Rotate         = %d\n", EncParam.s_Rotate );
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "StreamId       = %d\n", EncParam.s_StreamId);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "EncodeType     = %d\n", EncParam.s_EncodeType);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "FrameRate      = %d\n", EncParam.s_FrameRate);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "EncodeWidth    = %d\n", EncParam.s_EncodeWidth);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "EncodeHeight   = %d\n", EncParam.s_EncodeHeight);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateType    = %d\n", EncParam.s_BitRateType);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateAverage = %d\n", EncParam.s_BitRateAverage);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateUp      = %d\n", EncParam.s_BitRateUp);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateDown    = %d\n", EncParam.s_BitRateDown);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "FrameInterval  = %d\n", EncParam.s_FrameInterval);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "EncodeQulity   = %d\n", EncParam.s_EncodeQulity);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Rotate         = %d\n", EncParam.s_Rotate);

        }
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CheckVideoReslution(uint16_t Height, uint16_t Width)
{
    if (Height%4 != 0
            || Width%4 != 0)
    {
        return GMI_INVALID_PARAMETER;
    }

    //sys components
    SysPkgComponents SysComponents;
    SafePtr<ConfigFileManager>CfgFileManager(BaseMemoryManager::Instance().New<ConfigFileManager>());
    if (NULL == CfgFileManager.GetPtr())
    {
        SYS_ERROR("ConfigFileManager new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ConfigFileManager new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    GMI_RESULT Result = CfgFileManager->GetHwAutoDetectInfo(&SysComponents);
    if (FAILED(Result))
    {
        SYS_ERROR("GetHwAutoDetectInfo fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetHwAutoDetectInfo fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //accrroding hw capabilities, check video encode width and height
    if (e_CPU_A5S_66 == SysComponents.s_CpuId
            || e_CPU_A5S_88 == SysComponents.s_CpuId)
    {
        if (Height > RES_1080P_HEIGHT
                || Width > RES_1080P_WIDTH)
        {
            SYS_ERROR("Height %d error, Width %d error\n", Height, Width);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Height %d error, Width %d error\n", Height, Width);
            return GMI_NOT_SUPPORT;
        }
    }
    else if (e_CPU_A5S_55 == SysComponents.s_CpuId)
    {
        if (Height > RES_720P_HEIGHT
                || Width > RES_720P_WIDTH)
        {
            SYS_ERROR("Height %d error, Width %d error\n", Height, Width);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Height %d error, Width %d error\n", Height, Width);
            return GMI_NOT_SUPPORT;
        }
    }
    else
    {
        SYS_ERROR("SysComponents.s_CpuId %d not support\n", SysComponents.s_CpuId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SysComponents.s_CpuId %d not support\n", SysComponents.s_CpuId);
        return GMI_NOT_SUPPORT;
    }

    uint32_t Row;
    for (Row = 0; Row < sizeof(l_ReslutionTable)/(sizeof(l_ReslutionTable[0][0])*2); Row++)
    {
        if (Width == l_ReslutionTable[Row][0]
                && Height == l_ReslutionTable[Row][1])
        {
            SYS_INFO("%dX%d correct\n", Width, Height);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%dX%d correct\n", Width, Height);
            break;
        }
    }

    if (Row >= sizeof(l_ReslutionTable)/(sizeof(l_ReslutionTable[0][0])*2))
    {
        SYS_ERROR("%dX%d incorrect\n", Width, Height);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "%dX%d incorrect\n", Width, Height);
        return GMI_INVALID_PARAMETER;
    }

    return GMI_SUCCESS;

}


GMI_RESULT StreamCenterClient::CheckVideoEncodeConfiguration(VideoEncodeParam *EncParamPtr)
{
    if (EncParamPtr->s_EncodeType != MIN_VIDEO_ENC_TYPE)
    {
        return GMI_INVALID_PARAMETER;
    }

    if (EncParamPtr->s_BitRateType < MIN_VIDEO_ENC_BITRATETYPE
            || EncParamPtr->s_BitRateType > MAX_VIDEO_ENC_BITRATETYPE)
    {
        SYS_ERROR("EncParamPtr->s_BitRateType = %d incorrect\n", EncParamPtr->s_BitRateType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "EncParamPtr->s_BitRateType = %d incorrect\n", EncParamPtr->s_BitRateType);
        return GMI_INVALID_PARAMETER;
    }

    if (BIT_VBR == EncParamPtr->s_BitRateType)
    {
        if (EncParamPtr->s_BitRateUp == 0)
        {
            SYS_ERROR("EncParamPtr->s_BitRateUp = %d incorrect\n", EncParamPtr->s_BitRateUp);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "EncParamPtr->s_BitRateUp = %d incorrect\n", EncParamPtr->s_BitRateUp);
            return GMI_INVALID_PARAMETER;
        }

        if (EncParamPtr->s_BitRateDown > EncParamPtr->s_BitRateUp)
        {
            SYS_ERROR("BitRateDown %d, BitRateUp %d incorrect\n", EncParamPtr->s_BitRateDown, EncParamPtr->s_BitRateUp);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "BitRateDown %d, BitRateUp %d incorrect\n", EncParamPtr->s_BitRateDown, EncParamPtr->s_BitRateUp);
            return GMI_INVALID_PARAMETER;
        }
    }
    else if (BIT_CBR == EncParamPtr->s_BitRateType)
    {
    	if (EncParamPtr->s_BitRateAverage < EncParamPtr->s_BitRateDown
    	|| EncParamPtr->s_BitRateAverage > EncParamPtr->s_BitRateUp)
    	{
    		SYS_ERROR("BitRateAverage %d incorrect, because BitRateDown %d, BitRateUp %d\n", EncParamPtr->s_BitRateAverage, EncParamPtr->s_BitRateDown, EncParamPtr->s_BitRateUp);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "BitRateAverage %d incorrect, because BitRateDown %d, BitRateUp %d\n", EncParamPtr->s_BitRateAverage, EncParamPtr->s_BitRateDown, EncParamPtr->s_BitRateUp);
            return GMI_INVALID_PARAMETER;
    	}
    }
    else
    {
    	SYS_ERROR("BitRateType %d incorrect\n", EncParamPtr->s_BitRateType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "BitRateType %d incorrect\n", EncParamPtr->s_BitRateType);
    	return GMI_INVALID_PARAMETER;
    }

    if (EncParamPtr->s_Rotate < MIN_VIDEO_ENC_ROTATE
            || EncParamPtr->s_Rotate > MAX_VIDEO_ENC_ROTATE)
    {
        SYS_ERROR("EncParamPtr->s_Rotate = %d incorrect\n", EncParamPtr->s_Rotate);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "EncParamPtr->s_Rotate = %d incorrect\n", EncParamPtr->s_Rotate);
        return GMI_INVALID_PARAMETER;
    }

    if (EncParamPtr->s_EncodeQulity < MIN_VIDEO_ENC_QULITY
            || EncParamPtr->s_EncodeQulity > MAX_VIDEO_ENC_QULITY)
    {
        SYS_ERROR("EncParamPtr->s_EncodeQulity = %d incorrect\n", EncParamPtr->s_EncodeQulity);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "EncParamPtr->s_EncodeQulity = %d incorrect\n", EncParamPtr->s_EncodeQulity);
        return GMI_INVALID_PARAMETER;
    }

    if (EncParamPtr->s_FrameInterval< MIN_VIDEO_ENC_GOP
            || EncParamPtr->s_FrameInterval > MAX_VIDEO_ENC_GOP)
    {
        SYS_ERROR("EncParamPtr->s_FrameInterval = %d incorrect\n", EncParamPtr->s_FrameInterval);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "EncParamPtr->s_FrameInterval = %d incorrect\n", EncParamPtr->s_FrameInterval);
        return GMI_INVALID_PARAMETER;
    }

    if (EncParamPtr->s_StreamId < MIN_VIDEO_ENC_STREAMID
            || EncParamPtr->s_StreamId > MAX_VIDEO_ENC_STREAMID)
    {

        SYS_ERROR("EncParamPtr->s_StreamId = %d incorrect\n", EncParamPtr->s_StreamId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "EncParamPtr->s_StreamId = %d incorrect\n", EncParamPtr->s_StreamId);
        return GMI_INVALID_PARAMETER;
    }

    if (EncParamPtr->s_FrameRate < MIN_VIDEO_ENC_FRAMERATE
            || EncParamPtr->s_FrameRate > MAX_VIDEO_ENC_FRAMERATE)
    {

        SYS_ERROR("EncParamPtr->s_FrameRate = %d incorrect\n", EncParamPtr->s_FrameRate);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "EncParamPtr->s_FrameRate = %d incorrect\n", EncParamPtr->s_FrameRate);
        return GMI_INVALID_PARAMETER;
    }

    //check width and height proper or not
    GMI_RESULT Result = CheckVideoReslution(EncParamPtr->s_EncodeHeight, EncParamPtr->s_EncodeWidth);
    if (FAILED(Result))
    {
        SYS_ERROR("Reslution %dx%d error\n", EncParamPtr->s_EncodeWidth, EncParamPtr->s_EncodeHeight);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Reslution %dx%d error\n", EncParamPtr->s_EncodeWidth, EncParamPtr->s_EncodeHeight);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CheckAudioEncodeConfiguration(AudioEncParam *AudioEncParamPtr)
{

    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CheckAudioDecodeConfiguration(AudioDecParam *AudioDecParamPtr)
{
    return GMI_SUCCESS;
}


// static unsigned int DmGetTimeStamp()
// {
//     unsigned int stamp;
//     struct timeval now;

//     gettimeofday(&now, NULL);

//     stamp = now.tv_sec * 1000 + now.tv_usec/1000;

//     return stamp;
// }

GMI_RESULT StreamCenterClient::SetCodecConfiguration(FD_HANDLE Handle, void_t *CodecParameter, size_t CodecParameterLength)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    MediaHandle     *MediaHd;

    if (Handle)
    {
        MediaHd = (MediaHandle*)Handle;

        if (MEDIA_VIDEO == MediaHd->s_MediaType)
        {
            size_t MinParamLength =  CodecParameterLength > sizeof(VideoEncodeParam) ? sizeof(VideoEncodeParam) : CodecParameterLength;
            VideoEncodeParam EncParam;

            memcpy(&EncParam, CodecParameter, MinParamLength);

            SYS_INFO("StreamId       = %d\n", EncParam.s_StreamId);
            SYS_INFO("EncodeType     = %d\n", EncParam.s_EncodeType);
            SYS_INFO("FrameRate      = %d\n", EncParam.s_FrameRate);
            SYS_INFO("EncodeWidth    = %d\n", EncParam.s_EncodeWidth);
            SYS_INFO("EncodeHeight   = %d\n", EncParam.s_EncodeHeight);
            SYS_INFO("BitRateType    = %d\n", EncParam.s_BitRateType);
            SYS_INFO("BitRateAverage = %d\n", EncParam.s_BitRateAverage);
            SYS_INFO("BitRateUp      = %d\n", EncParam.s_BitRateUp);
            SYS_INFO("BitRateDown    = %d\n", EncParam.s_BitRateDown);
            SYS_INFO("FrameInterval  = %d\n", EncParam.s_FrameInterval);
            SYS_INFO("EncodeQulity   = %d\n", EncParam.s_EncodeQulity);
            SYS_INFO("Rotate         = %d\n", EncParam.s_Rotate);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "StreamId       = %d\n", EncParam.s_StreamId);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "EncodeType     = %d\n", EncParam.s_EncodeType);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "FrameRate      = %d\n", EncParam.s_FrameRate);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "EncodeWidth    = %d\n", EncParam.s_EncodeWidth);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "EncodeHeight   = %d\n", EncParam.s_EncodeHeight);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateType    = %d\n", EncParam.s_BitRateType);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateAverage = %d\n", EncParam.s_BitRateAverage);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateUp      = %d\n", EncParam.s_BitRateUp);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "BitRateDown    = %d\n", EncParam.s_BitRateDown);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "FrameInterval  = %d\n", EncParam.s_FrameInterval);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "EncodeQulity   = %d\n", EncParam.s_EncodeQulity);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Rotate         = %d\n", EncParam.s_Rotate);

            GMI_RESULT Result = CheckVideoEncodeConfiguration(&EncParam);
            if (FAILED(Result))
            {
                SYS_ERROR("VideoEncodeConfiguration is invalid, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "VideoEncodeConfiguration is invalid, Result = 0x%lx\n", Result);
                return Result;
            }

            Result = m_MediaCenter.SetCodecConfig(MediaHd->s_Encode, &EncParam, MinParamLength );
            if (FAILED(Result))
            {
                SYS_ERROR("SetCodecConfiguration fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetCodecConfiguration fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (MEDIA_AUDIO == MediaHd->s_MediaType)
        {
            if (MediaHd->s_EncMode)
            {
                AudioEncParam EncParam;
                size_t MinParamLength =  CodecParameterLength > sizeof(AudioEncParam) ? sizeof(AudioEncParam) : CodecParameterLength;

                memcpy(&EncParam, CodecParameter, MinParamLength);
                SYS_INFO("AudioId    = %d\n", EncParam.s_AudioId);
                SYS_INFO("CapVolume  = %d\n", EncParam.s_Volume);
                SYS_INFO("Codec      = %d\n", EncParam.s_Codec);

                GMI_RESULT Result = CheckAudioEncodeConfiguration(&EncParam);
                if (FAILED(Result))
                {
                    SYS_ERROR("audio encode config is invalid, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "audio encode config is invalid, Result = 0x%lx\n", Result);
                    return Result;
                }

                Result = m_MediaCenter.SetCodecConfig(MediaHd->s_Encode, &EncParam, MinParamLength );
                if (FAILED(Result))
                {
                    SYS_ERROR("set audio encode config fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "set audio encode fail, Result = 0x%lx\n", Result);
                    return Result;
                }
            }
            else
            {
                AudioDecParam DecParam;
                size_t MinParamLength =  CodecParameterLength > sizeof(AudioDecParam) ? sizeof(AudioDecParam) : CodecParameterLength;

                memcpy(&DecParam, CodecParameter, MinParamLength);
                SYS_INFO("AudioId     = %d\n", DecParam.s_AudioId);
                SYS_INFO("PlayVolume  = %d\n", DecParam.s_Volume);
                SYS_INFO("Codec       = %d\n", DecParam.s_Codec);

                GMI_RESULT Result = CheckAudioDecodeConfiguration(&DecParam);
                if (FAILED(Result))
                {
                    SYS_ERROR("audio decode config is invalid, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "audio decode config is invalid, Result = 0x%lx\n", Result);
                    return Result;
                }

                Result = m_MediaCenter.SetCodecConfig(MediaHd->s_Encode, &DecParam, MinParamLength );
                if (FAILED(Result))
                {
                    SYS_ERROR("set audio decode config fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "set audio decode fail, Result = 0x%lx\n", Result);
                    return Result;
                }
            }
        }
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CheckOsdConfiguration(VideoOSDParam *OsdParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    if (NULL == OsdParamPtr)
    {
        SYS_ERROR("OsdParamPtr is null\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_StreamId < MIN_VIDEO_OSD_STREAMID
            || OsdParamPtr->s_StreamId > MAX_VIDEO_OSD_STREAMID)
    {
        SYS_ERROR("OsdParamPtr->s_StreamId %d error\n", OsdParamPtr->s_StreamId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_StreamId %d error\n", OsdParamPtr->s_StreamId);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_Language < MIN_VIDEO_OSD_LANGUAGE
            || OsdParamPtr->s_Language > MAX_VIDEO_OSD_LANGUAGE)
    {
        SYS_ERROR("OsdParamPtr->s_Language %d error\n", OsdParamPtr->s_Language);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_Language %d error\n", OsdParamPtr->s_Language);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_Flag < MIN_VIDEO_OSD_TIMEFLAG
            || OsdParamPtr->s_TimeDisplay.s_Flag > MAX_VIDEO_OSD_TIMEFLAG)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_Flag %d error\n", OsdParamPtr->s_TimeDisplay.s_Flag);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_Flag %d error\n", OsdParamPtr->s_TimeDisplay.s_Flag);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_DateStyle < MIN_VIDEO_OSD_DATASTYLE
            || OsdParamPtr->s_TimeDisplay.s_DateStyle > MAX_VIDEO_OSD_DATASTYLE)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_DateStyle %d error\n", OsdParamPtr->s_TimeDisplay.s_DateStyle);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_DateStyle %d error\n", OsdParamPtr->s_TimeDisplay.s_DateStyle);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_TimeStyle < MIN_VIDEO_OSD_TIMESTYLE
            || OsdParamPtr->s_TimeDisplay.s_TimeStyle > MAX_VIDEO_OSD_TIMESTYPE)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_TimeStyle %d error\n", OsdParamPtr->s_TimeDisplay.s_TimeStyle);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_TimeStyle %d error\n", OsdParamPtr->s_TimeDisplay.s_TimeStyle);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_FontColor < MIN_VIDEO_OSD_FONTCOLOR
            || OsdParamPtr->s_TimeDisplay.s_FontColor > MAX_VIDEO_OSD_FONTCOLOR)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_FontColor %d error\n", OsdParamPtr->s_TimeDisplay.s_FontColor);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_FontColor %d error\n", OsdParamPtr->s_TimeDisplay.s_FontColor);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_FontStyle < MIN_VIDEO_OSD_FONTSTYLE
            || OsdParamPtr->s_TimeDisplay.s_FontStyle > MAX_VIDEO_OSD_FONTSTYLE)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_FontStyle %d error\n", OsdParamPtr->s_TimeDisplay.s_FontStyle);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_FontStyle %d error\n", OsdParamPtr->s_TimeDisplay.s_FontStyle);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_FontBlod < MIN_VIDEO_OSD_FONTBLOD
            || OsdParamPtr->s_TimeDisplay.s_FontBlod > MAX_VIDEO_OSD_FONTBLOD)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_FontBlod %d error\n", OsdParamPtr->s_TimeDisplay.s_FontBlod);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_FontBlod %d error\n", OsdParamPtr->s_TimeDisplay.s_FontBlod);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_Rotate < MIN_VIDEO_OSD_FONTROTATE
            || OsdParamPtr->s_TimeDisplay.s_Rotate > MAX_VIDEO_OSD_FONTROTATE)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_Rotate %d error\n", OsdParamPtr->s_TimeDisplay.s_Rotate);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_Rotate %d error\n", OsdParamPtr->s_TimeDisplay.s_Rotate);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_Italic < MIN_VIDEO_OSD_FONTITALIC
            || OsdParamPtr->s_TimeDisplay.s_Italic > MAX_VIDEO_OSD_FONTITALIC)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_Italic %d error\n", OsdParamPtr->s_TimeDisplay.s_Italic);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_Italic %d error\n", OsdParamPtr->s_TimeDisplay.s_Italic);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_Outline < MIN_VIDEO_OSD_FONTOUTLINE
            || OsdParamPtr->s_TimeDisplay.s_Outline > MAX_VIDEO_OSD_FONTOUTLINE)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_Outline %d error\n", OsdParamPtr->s_TimeDisplay.s_Outline);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_Outline %d error\n", OsdParamPtr->s_TimeDisplay.s_Outline);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_DisplayX < MIN_VIDEO_OSD_DISPLAYX
            || OsdParamPtr->s_TimeDisplay.s_DisplayX > MAX_VIDEO_OSD_DISPLAYX)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_DisplayY %d error\n", OsdParamPtr->s_TimeDisplay.s_DisplayY);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_DisplayY %d error\n", OsdParamPtr->s_TimeDisplay.s_DisplayY);
        return GMI_INVALID_PARAMETER;
    }

    if (OsdParamPtr->s_TimeDisplay.s_DisplayY < MIN_VIDEO_OSD_DISPLAYY
            || OsdParamPtr->s_TimeDisplay.s_DisplayY > MAX_VIDEO_OSD_DISPLAYY)
    {
        SYS_ERROR("OsdParamPtr->s_TimeDisplay.s_DisplayY %d error\n", OsdParamPtr->s_TimeDisplay.s_DisplayY);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TimeDisplay.s_DisplayY %d error\n", OsdParamPtr->s_TimeDisplay.s_DisplayY);
        return GMI_INVALID_PARAMETER;
    }


    for (int32_t Id = 0; Id < 1; Id++)
    {
        if (OsdParamPtr->s_TextDisplay[Id].s_Flag < MIN_VIDEO_OSD_TEXTFLAG
                || OsdParamPtr->s_TextDisplay[Id].s_Flag > MAX_VIDEO_OSD_TEXTFLAG)
        {
            SYS_ERROR("OsdParamPtr->s_TextDisplay[%d].s_Flag %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_Flag);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TextDisplay[%d].s_Flag %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_Flag);
            return GMI_INVALID_PARAMETER;
        }

        if (OsdParamPtr->s_TextDisplay[Id].s_FontColor < MIN_VIDEO_OSD_FONTCOLOR
                || OsdParamPtr->s_TextDisplay[Id].s_FontColor > MAX_VIDEO_OSD_FONTCOLOR)
        {
            SYS_ERROR("OsdParamPtr->s_TextDisplay[%d].s_FontColor %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_FontColor);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TextDisplay[%d].s_FontColor %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_FontColor);
            return GMI_INVALID_PARAMETER;
        }

        if (OsdParamPtr->s_TextDisplay[Id].s_FontStyle < MIN_VIDEO_OSD_FONTSTYLE
                || OsdParamPtr->s_TextDisplay[Id].s_FontStyle > MAX_VIDEO_OSD_FONTSTYLE)
        {
            SYS_ERROR("OsdParamPtr->s_TextDisplay[%d].s_FontStyle %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_FontStyle);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TextDisplay[%d].s_FontStyle %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_FontStyle);
            return GMI_INVALID_PARAMETER;
        }

        if (OsdParamPtr->s_TextDisplay[Id].s_FontBlod < MIN_VIDEO_OSD_FONTBLOD
                || OsdParamPtr->s_TextDisplay[Id].s_FontBlod > MAX_VIDEO_OSD_FONTBLOD)
        {
            SYS_ERROR("OsdParamPtr->s_TextDisplay[%d].s_FontBlod %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_FontBlod);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TextDisplay[%d].s_FontBlod %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_FontBlod);
            return GMI_INVALID_PARAMETER;
        }

        if (OsdParamPtr->s_TextDisplay[Id].s_Rotate < MIN_VIDEO_OSD_FONTROTATE
                || OsdParamPtr->s_TextDisplay[Id].s_Rotate > MAX_VIDEO_OSD_FONTROTATE)
        {
            SYS_ERROR("OsdParamPtr->s_TextDisplay[%d].s_Rotate %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_Rotate);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TextDisplay[%d].s_Rotate %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_Rotate);
            return GMI_INVALID_PARAMETER;
        }

        if (OsdParamPtr->s_TextDisplay[Id].s_Italic < MIN_VIDEO_OSD_FONTITALIC
                || OsdParamPtr->s_TextDisplay[Id].s_Italic > MAX_VIDEO_OSD_FONTITALIC)
        {
            SYS_ERROR("OsdParamPtr->s_TextDisplay[%d].s_Italic %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_Italic);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TextDisplay[%d].s_Italic %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_Italic);
            return GMI_INVALID_PARAMETER;
        }

        if (OsdParamPtr->s_TextDisplay[Id].s_Outline < MIN_VIDEO_OSD_FONTOUTLINE
                || OsdParamPtr->s_TextDisplay[Id].s_Outline > MAX_VIDEO_OSD_FONTOUTLINE)
        {
            SYS_ERROR("OsdParamPtr->s_TextDisplay[%d].s_Outline %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_Outline);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TextDisplay[%d].s_Outline %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_Outline);
            return GMI_INVALID_PARAMETER;
        }

        if (OsdParamPtr->s_TextDisplay[Id].s_DisplayX < MIN_VIDEO_OSD_DISPLAYX
                || OsdParamPtr->s_TextDisplay[Id].s_DisplayY > MAX_VIDEO_OSD_DISPLAYX)
        {
            SYS_ERROR("OsdParamPtr->s_TextDisplay[%d].s_DisplayX %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_DisplayX);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TextDisplay[%d].s_DisplayX %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_DisplayX);
            return GMI_INVALID_PARAMETER;
        }

        if (OsdParamPtr->s_TextDisplay[Id].s_DisplayY < MIN_VIDEO_OSD_DISPLAYY
                || OsdParamPtr->s_TextDisplay[Id].s_DisplayY > MAX_VIDEO_OSD_DISPLAYY)
        {
            SYS_ERROR("OsdParamPtr->s_TextDisplay[%d].s_DisplayY %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_DisplayY);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TextDisplay[%d].s_DisplayY %d error\n", Id, OsdParamPtr->s_TextDisplay[Id].s_DisplayY);
            return GMI_INVALID_PARAMETER;
        }

        if (0 == strlen((const char_t*)OsdParamPtr->s_TextDisplay[Id].s_TextContent))
        {
            SYS_ERROR("OsdParamPtr->s_TextDisplay[%d].s_TextContent is null\n", Id);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "OsdParamPtr->s_TextDisplay[%d].s_TextContent is null\n", Id);
            return GMI_INVALID_PARAMETER;
        }
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::GetAdaptedOsdFontSize(uint32_t Id, uint8_t *FontSize)
{
    uint32_t Row;
    for (Row = 0; Row < sizeof(l_OSD_FontSize)/(sizeof(l_OSD_FontSize[0][0])*2); Row++)
    {
        if (m_VideoEncParam[Id].s_EncodeHeight == l_OSD_FontSize[Row][0])
        {
            *FontSize = l_OSD_FontSize[Row][1];
            break;
        }
    }

    if (Row >= sizeof(l_OSD_FontSize)/(sizeof(l_OSD_FontSize[0][0])*2))
    {
        *FontSize = OSD_FONT_SIZE_MEDIUM;
    }

    return GMI_SUCCESS;
}

GMI_RESULT StreamCenterClient::SetOsdConfiguration(FD_HANDLE Handle,  VideoOSDParam *OsdParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    MediaHandle     *MediaHd;
    VideoOSDParam    OsdParam;

    if (Handle)
    {
        size_t ParamLength = sizeof(VideoOSDParam);
        memcpy(&OsdParam, OsdParamPtr, ParamLength);
        MediaHd = (MediaHandle*)Handle;
        //font size adapted
        if (0 == OsdParam.s_TimeDisplay.s_FontStyle)
        {
            GetAdaptedOsdFontSize(MediaHd->s_StreamId, &OsdParam.s_TimeDisplay.s_FontStyle);
        }

        SYS_INFO("StreamId    = %d\n", OsdParam.s_StreamId);
        SYS_INFO("DisplayType = %d\n", OsdParam.s_DisplayType);
        SYS_INFO("Flag        = %d\n", OsdParam.s_Flag);
        SYS_INFO("Language    = %d\n", OsdParam.s_Language);

        SYS_INFO("TimeDisplay.Flag = %d\n", OsdParam.s_TimeDisplay.s_Flag);
        SYS_INFO("TimeDisplay.DateStyle = %d\n", OsdParam.s_TimeDisplay.s_DateStyle);
        SYS_INFO("TimeDisplay.TimeStyle = %d\n", OsdParam.s_TimeDisplay.s_TimeStyle);
        SYS_INFO("TimeDisplay.FontColor = %d\n", OsdParam.s_TimeDisplay.s_FontColor);
        SYS_INFO("TimeDisplay.FontStyle = %d\n", OsdParam.s_TimeDisplay.s_FontStyle);
        SYS_INFO("TimeDisplay.FontBlod  = %d\n", OsdParam.s_TimeDisplay.s_FontBlod);
        SYS_INFO("TimeDisplay.Rotate    = %d\n", OsdParam.s_TimeDisplay.s_Rotate);
        SYS_INFO("TimeDisplay.Italic    = %d\n", OsdParam.s_TimeDisplay.s_Italic);
        SYS_INFO("TimeDisplay.Outline   = %d\n", OsdParam.s_TimeDisplay.s_Outline);
        SYS_INFO("TimeDisplay.DisplayX  = %d\n", OsdParam.s_TimeDisplay.s_DisplayX);
        SYS_INFO("TimeDisplay.DisplayY  = %d\n", OsdParam.s_TimeDisplay.s_DisplayY);
        SYS_INFO("TimeDisplay.DisplayH  = %d\n", OsdParam.s_TimeDisplay.s_DisplayH);
        SYS_INFO("TimeDisplay.DisplayW  = %d\n", OsdParam.s_TimeDisplay.s_DisplayW);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "StreamId    = %d\n", OsdParam.s_StreamId);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "DisplayType = %d\n", OsdParam.s_DisplayType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Flag        = %d\n", OsdParam.s_Flag);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Language    = %d\n", OsdParam.s_Language);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.Flag = %d\n", OsdParam.s_TimeDisplay.s_Flag);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.DateStyle = %d\n", OsdParam.s_TimeDisplay.s_DateStyle);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.TimeStyle = %d\n", OsdParam.s_TimeDisplay.s_TimeStyle);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.FontColor = %d\n", OsdParam.s_TimeDisplay.s_FontColor);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.FontStyle = %d\n", OsdParam.s_TimeDisplay.s_FontStyle);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.FontBlod  = %d\n", OsdParam.s_TimeDisplay.s_FontBlod);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.Rotate    = %d\n", OsdParam.s_TimeDisplay.s_Rotate);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.Italic    = %d\n", OsdParam.s_TimeDisplay.s_Italic);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.Outline   = %d\n", OsdParam.s_TimeDisplay.s_Outline);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.DisplayX  = %d\n", OsdParam.s_TimeDisplay.s_DisplayX);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.DisplayH  = %d\n", OsdParam.s_TimeDisplay.s_DisplayH);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TimeDisplay.DisplayW  = %d\n", OsdParam.s_TimeDisplay.s_DisplayW);

        for ( int32_t i = 0; i < 1; ++i )
        {
            //font size adapted
            if (0 == OsdParam.s_TextDisplay[i].s_FontStyle)
            {
                GetAdaptedOsdFontSize(MediaHd->s_StreamId, &OsdParam.s_TextDisplay[i].s_FontStyle);
            }

            SYS_INFO("TextDisplay[%d].Flag           = %d\n", i, OsdParam.s_TextDisplay[i].s_Flag);
            SYS_INFO("TextDisplay[%d].FontColor      = %d\n", i, OsdParam.s_TextDisplay[i].s_FontColor);
            SYS_INFO("TextDisplay[%d].FontStyle      = %d\n", i, OsdParam.s_TextDisplay[i].s_FontStyle);
            SYS_INFO("TextDisplay[%d].FontBlod       = %d\n", i, OsdParam.s_TextDisplay[i].s_FontBlod);
            SYS_INFO("TextDisplay[%d].Rotate         = %d\n", i, OsdParam.s_TextDisplay[i].s_Rotate);
            SYS_INFO("TextDisplay[%d].Italic         = %d\n", i, OsdParam.s_TextDisplay[i].s_Italic);
            SYS_INFO("TextDisplay[%d].Outline        = %d\n", i, OsdParam.s_TextDisplay[i].s_Outline);
            SYS_INFO("TextDisplay[%d].TextContentLen = %d\n", i, OsdParam.s_TextDisplay[i].s_TextContentLen);

            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].Flag           = %d\n", i, OsdParam.s_TextDisplay[i].s_Flag);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].FontColor      = %d\n", i, OsdParam.s_TextDisplay[i].s_FontColor);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].FontStyle      = %d\n", i, OsdParam.s_TextDisplay[i].s_FontStyle);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].FontBlod       = %d\n", i, OsdParam.s_TextDisplay[i].s_FontBlod);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].Rotate         = %d\n", i, OsdParam.s_TextDisplay[i].s_Rotate);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].Italic         = %d\n", i, OsdParam.s_TextDisplay[i].s_Italic);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].Outline        = %d\n", i, OsdParam.s_TextDisplay[i].s_Outline);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].TextContentLen = %d\n", i, OsdParam.s_TextDisplay[i].s_TextContentLen);


            if (0 < OsdParam.s_TextDisplay[i].s_Flag && 0 < OsdParam.s_TextDisplay[i].s_TextContentLen)
            {
                SYS_INFO("TextDisplay[%d].TextContent = %s\n", i, (char_t*)(OsdParam.s_TextDisplay[i].s_TextContent));
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].TextContent = %s\n", i, (char_t*)(OsdParam.s_TextDisplay[i].s_TextContent));
            }
            SYS_INFO("TextDisplay[%d].DisplayX = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayX);
            SYS_INFO("TextDisplay[%d].DisplayY = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayY);
            SYS_INFO("TextDisplay[%d].DisplayH = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayH);
            SYS_INFO("TextDisplay[%d].DisplayW = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayW);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].DisplayX = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayX);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].DisplayY = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayY);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].DisplayH = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayH);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "TextDisplay[%d].DisplayW = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayW);
        }

        GMI_RESULT Result = CheckOsdConfiguration(&OsdParam);
        if (FAILED(Result))
        {
            SYS_ERROR("CheckOsdConfiguration fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CheckOsdConfiguration fail, Result = 0x%lx\n", Result);
            return Result;
        }

        Result = m_MediaCenter.SetOsdConfig(MediaHd->s_Encode, &OsdParam, ParamLength );
        if (FAILED(Result))
        {
            SYS_ERROR("SetOsdConfiguration fail, Result= 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetOsdConfiguration fail, Result= 0x%lx\n", Result);
            return Result;
        }
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::ForceGenerateIdrFrame(FD_HANDLE Handle)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    MediaHandle     *MediaHd;

    if (Handle)
    {
        MediaHd = (MediaHandle*)Handle;

        GMI_RESULT Result = m_MediaCenter.ForceGenerateIdrFrame(MediaHd->s_Encode);
        if (FAILED(Result))
        {
            SYS_ERROR("ForceGenerateIdrFrame fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ForceGenerateIdrFrame fail, Result = 0x%lx\n", Result);
            return Result;
        }
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CheckImageConfiguration(ImageBaseParam *ImageParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    if (NULL == ImageParamPtr)
    {
        SYS_ERROR("ImageParamPtr\n");
        return GMI_INVALID_PARAMETER;
    }

    if (ImageParamPtr->s_Brightness > MAX_IMAGE_BASE_BRIGHT
            || ImageParamPtr->s_Brightness < MIN_IMAGE_BASE_BRIGHT)
    {
        SYS_ERROR("ImageParamPtr->s_Brightness %d error\n", ImageParamPtr->s_Brightness);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageParamPtr->s_Brightness %d error\n", ImageParamPtr->s_Brightness);
        return GMI_INVALID_PARAMETER;
    }

    if (ImageParamPtr->s_Contrast < MIN_IMAGE_BASE_CONTRAST
            || ImageParamPtr->s_Contrast > MAX_IMAGE_BASE_CONTRAST)
    {
        SYS_ERROR("ImageParamPtr->s_Contrast %d error\n", ImageParamPtr->s_Contrast);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageParamPtr->s_Contrast %d error\n", ImageParamPtr->s_Contrast);
        return GMI_INVALID_PARAMETER;
    }

    if (ImageParamPtr->s_Saturation < MIN_IMAGE_BASE_SATURATION
            || ImageParamPtr->s_Saturation > MAX_IMAGE_BASE_SATURATION)
    {
        SYS_ERROR("ImageParamPtr->s_Saturation %d error\n", ImageParamPtr->s_Saturation);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageParamPtr->s_Saturation %d error\n", ImageParamPtr->s_Saturation);
        return GMI_INVALID_PARAMETER;
    }

    if (ImageParamPtr->s_Hue < MIN_IMAGE_BASE_HUE
            || ImageParamPtr->s_Hue > MAX_IMAGE_BASE_HUE)
    {
        SYS_ERROR("ImageParamPtr->s_Hue %d error\n", ImageParamPtr->s_Hue);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageParamPtr->s_Hue %d error\n", ImageParamPtr->s_Hue);
        return GMI_INVALID_PARAMETER;
    }

    if (ImageParamPtr->s_Sharpness < MIN_IMAGE_BASE_SHARP
            || ImageParamPtr->s_Sharpness > MAX_IMAGE_BASE_SHARP)
    {
        SYS_ERROR("ImageParamPtr->s_Sharpness %d error\n", ImageParamPtr->s_Sharpness);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageParamPtr->s_Sharpness %d error\n", ImageParamPtr->s_Sharpness);
        return GMI_INVALID_PARAMETER;
    }

    if (ImageParamPtr->s_ExposureMode < MIN_IMAGE_BASE_EXPMODE
            || ImageParamPtr->s_ExposureMode > MAX_IMAGE_BASE_EXPMODE)
    {
        SYS_ERROR("ImageParamPtr->s_ExposureMode %d error\n", ImageParamPtr->s_ExposureMode);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageParamPtr->s_ExposureMode %d error\n", ImageParamPtr->s_ExposureMode);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_BASE_EXPVALUEMAX > ImageParamPtr->s_ExposureValueMax
            || MAX_IMAGE_BASE_EXPVALUEMAX < ImageParamPtr->s_ExposureValueMax)
    {
        SYS_ERROR("ImageParamPtr->s_ExposureValueMax %d error\n", ImageParamPtr->s_ExposureValueMax);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageParamPtr->s_ExposureValueMax %d error\n", ImageParamPtr->s_ExposureValueMax);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_BASE_EXPVALUEMIN > ImageParamPtr->s_ExposureValueMin
            || MAX_IMAGE_BASE_EXPVALUEMIN < ImageParamPtr->s_ExposureValueMin)
    {
        SYS_ERROR("ImageParamPtr->s_ExposureValueMin %d error\n", ImageParamPtr->s_ExposureValueMin);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageParamPtr->s_ExposureValueMin %d error\n", ImageParamPtr->s_ExposureValueMin);
        return GMI_INVALID_PARAMETER;
    }

    uint32_t Id;
    uint32_t ArraySize = sizeof(l_ExposureGainMaxTable)/sizeof(l_ExposureGainMaxTable[0]);
    int16_t  *ExposureGainMaxTablePtr = l_ExposureGainMaxTable;
    for (Id = 0; Id < ArraySize; Id++)
    {
        if (ImageParamPtr->s_GainMax == *ExposureGainMaxTablePtr)
        {
            break;
        }
        ExposureGainMaxTablePtr++;
    }
    if (Id >= ArraySize)
    {
        //SYS_ERROR("ImageParamPtr->s_Brightness %d error\n", ImageParamPtr->s_GainMax);
        return GMI_INVALID_PARAMETER;
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::SetImageConfiguration(FD_HANDLE Handle, ImageBaseParam *ImageParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    ImageBaseParam   ImageParam;

    size_t ParamLength = sizeof(ImageBaseParam);
    memset(&ImageParam, 0, ParamLength);
    memcpy(&ImageParam, ImageParamPtr, ParamLength);

    SYS_INFO("ExposureMode     = %d\n", ImageParam.s_ExposureMode);
    SYS_INFO("ExposureValueMin = %d\n", ImageParam.s_ExposureValueMin);
    SYS_INFO("ExposureValueMax = %d\n", ImageParam.s_ExposureValueMax);
    SYS_INFO("GainMax          = %d\n", ImageParam.s_GainMax);
    SYS_INFO("Brightness       = %d\n", ImageParam.s_Brightness);
    SYS_INFO("Contrast         = %d\n", ImageParam.s_Contrast);
    SYS_INFO("Saturation       = %d\n", ImageParam.s_Saturation);
    SYS_INFO("Hue              = %d\n", ImageParam.s_Hue);
    SYS_INFO("Sharpness        = %d\n", ImageParam.s_Sharpness);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ExposureMode     = %d\n", ImageParam.s_ExposureMode);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ExposureValueMin = %d\n", ImageParam.s_ExposureValueMin);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ExposureValueMax = %d\n", ImageParam.s_ExposureValueMax);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "GainMax          = %d\n", ImageParam.s_GainMax);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Brightness       = %d\n", ImageParam.s_Brightness);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Contrast         = %d\n", ImageParam.s_Contrast);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Saturation       = %d\n", ImageParam.s_Saturation);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Hue              = %d\n", ImageParam.s_Hue);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Sharpness        = %d\n", ImageParam.s_Sharpness);

    GMI_RESULT Result = CheckImageConfiguration(&ImageParam);
    if (FAILED(Result))
    {
        SYS_ERROR("CheckImageConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CheckImageConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_MediaCenter.SetBaseImageConfig(Handle, &ImageParam, ParamLength);
    if (FAILED(Result))
    {
        SYS_ERROR("SetImageConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetImageConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CheckImageAdvancedConfiguration(ImageAdvanceParam *ImageAdvancedParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    if (NULL == ImageAdvancedParamPtr)
    {
        SYS_ERROR("ImageAdvancedParamPtr\n");
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_ADVANCE_METERMODE > ImageAdvancedParamPtr->s_MeteringMode
            || MAX_IMAGE_ADVANCE_METERMODE < ImageAdvancedParamPtr->s_MeteringMode)
    {
        SYS_ERROR("ImageAdvancedParamPtr->s_MeteringMode %d error\n", ImageAdvancedParamPtr->s_MeteringMode);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageAdvancedParamPtr->s_MeteringMode %d error\n", ImageAdvancedParamPtr->s_MeteringMode);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_ADVANCE_BLCOMPFLAG != ImageAdvancedParamPtr->s_BackLightCompFlag
            && MAX_IMAGE_ADVANCE_BLCOMPFLAG != ImageAdvancedParamPtr->s_BackLightCompFlag)
    {
        SYS_ERROR("ImageAdvancedParamPtr->s_BackLightCompFlag %d error\n", ImageAdvancedParamPtr->s_BackLightCompFlag);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageAdvancedParamPtr->s_BackLightCompFlag %d error\n", ImageAdvancedParamPtr->s_BackLightCompFlag);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_ADVANCE_DCIRISFLAG != ImageAdvancedParamPtr->s_DcIrisFlag
            && MAX_IMAGE_ADVANCE_DCIRISFLAG != ImageAdvancedParamPtr->s_DcIrisFlag)
    {
        SYS_ERROR("ImageAdvancedParamPtr->s_DcIrisFlag %d error\n", ImageAdvancedParamPtr->s_DcIrisFlag);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageAdvancedParamPtr->s_DcIrisFlag %d error\n", ImageAdvancedParamPtr->s_DcIrisFlag);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_ADVANCE_LOCALEXP > ImageAdvancedParamPtr->s_LocalExposure
            || MAX_IMAGE_ADVANCE_LOCALEXP < ImageAdvancedParamPtr->s_LocalExposure)
    {
        SYS_ERROR("ImageAdvancedParamPtr->s_LocalExposure %d error\n", ImageAdvancedParamPtr->s_LocalExposure);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageAdvancedParamPtr->s_LocalExposure %d error\n", ImageAdvancedParamPtr->s_LocalExposure);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_ADVANCE_MCTFSTRENGTH > ImageAdvancedParamPtr->s_MctfStrength
            || MAX_IMAGE_ADVANCE_MCTFSTRENGTH < ImageAdvancedParamPtr->s_MctfStrength)
    {
        SYS_ERROR("ImageAdvancedParamPtr->s_MctfStrength %d error\n", ImageAdvancedParamPtr->s_MctfStrength);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageAdvancedParamPtr->s_MctfStrength %d error\n", ImageAdvancedParamPtr->s_MctfStrength);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_ADVANCE_DCIRISDUTY > ImageAdvancedParamPtr->s_DcIrisDuty
            || MAX_IMAGE_ADVANCE_DCIRISDUTY < ImageAdvancedParamPtr->s_DcIrisDuty)
    {
        SYS_ERROR("ImageAdvancedParamPtr->s_DcIrisDuty %d error\n", ImageAdvancedParamPtr->s_DcIrisDuty);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageAdvancedParamPtr->s_DcIrisDuty %d error\n", ImageAdvancedParamPtr->s_DcIrisDuty);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_ADVANCE_AETARGETRATIO > ImageAdvancedParamPtr->s_AeTargetRatio
            || MAX_IMAGE_ADVANCE_AETARGETRATIO < ImageAdvancedParamPtr->s_AeTargetRatio)
    {
        SYS_ERROR("ImageAdvancedParamPtr->s_AeTargetRatio %d error\n", ImageAdvancedParamPtr->s_AeTargetRatio);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageAdvancedParamPtr->s_AeTargetRatio %d error\n", ImageAdvancedParamPtr->s_AeTargetRatio);
        return GMI_INVALID_PARAMETER;
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::SetImageAdvanceConfiguration(FD_HANDLE Handle, ImageAdvanceParam *ImageAdvanceParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    ImageAdvanceParam ImageAdvanceParam;
    size_t ParamLength = sizeof(ImageAdvanceParam);

    memset(&ImageAdvanceParam, 0, ParamLength);
    memcpy(&ImageAdvanceParam, ImageAdvanceParamPtr, ParamLength);

    SYS_INFO("ImageAdvanceParam.s_MeteringMode      = %d\n", ImageAdvanceParam.s_MeteringMode);
    SYS_INFO("ImageAdvanceParam.s_BackLightCompFlag = %d\n", ImageAdvanceParam.s_BackLightCompFlag);
    SYS_INFO("ImageAdvanceParam.s_DcIrisFlag        = %d\n", ImageAdvanceParam.s_DcIrisFlag);
    SYS_INFO("ImageAdvanceParam.s_LocalExposure     = %d\n", ImageAdvanceParam.s_LocalExposure);
    SYS_INFO("ImageAdvanceParam.s_MctfStrength      = %d\n", ImageAdvanceParam.s_MctfStrength);
    SYS_INFO("ImageAdvanceParam.s_DcIrisDuty        = %d\n", ImageAdvanceParam.s_DcIrisDuty);
    SYS_INFO("ImageAdvanceParam.s_AeTargetRatio     = %d\n", ImageAdvanceParam.s_AeTargetRatio);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageAdvanceParam.s_MeteringMode      = %d\n", ImageAdvanceParam.s_MeteringMode);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageAdvanceParam.s_BackLightCompFlag = %d\n", ImageAdvanceParam.s_BackLightCompFlag);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageAdvanceParam.s_DcIrisFlag        = %d\n", ImageAdvanceParam.s_DcIrisFlag);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageAdvanceParam.s_LocalExposure     = %d\n", ImageAdvanceParam.s_LocalExposure);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageAdvanceParam.s_MctfStrength      = %d\n", ImageAdvanceParam.s_MctfStrength);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageAdvanceParam.s_DcIrisDuty        = %d\n", ImageAdvanceParam.s_DcIrisDuty);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageAdvanceParam.s_AeTargetRatio     = %d\n", ImageAdvanceParam.s_AeTargetRatio);

    GMI_RESULT Result = CheckImageAdvancedConfiguration(&ImageAdvanceParam);
    if (FAILED(Result))
    {
        SYS_ERROR("CheckAdvancedImageConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CheckAdvancedImageConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_MediaCenter.SetAdvancedImageConfig(Handle, &ImageAdvanceParam, ParamLength );
    if (FAILED(Result))
    {
        SYS_ERROR("SetAdvancedImageConfig fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetAdvancedImageConfig fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CheckVinVoutConfiguration(VideoInParam *VidInParamPtr, VideoOutParam *VidOutParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);

    if (MIN_VIDEO_IN_FLAG != VidInParamPtr->s_VinFlag
            && MAX_VIDEO_IN_FLAG != VidInParamPtr->s_VinFlag)
    {
        SYS_ERROR("video in enable = %d incorrect\n", VidInParamPtr->s_VinFlag);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "video in enable = %d incorrect\n", VidInParamPtr->s_VinFlag);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_VIDEO_IN_FRAMERATE > VidInParamPtr->s_VinFrameRate
            || MAX_VIDEO_IN_FRAMERATE < VidInParamPtr->s_VinFrameRate)
    {
        SYS_ERROR("video in frame rate = %d incorrect\n", VidInParamPtr->s_VinFrameRate);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "video in frame rate = %d incorrect\n", VidInParamPtr->s_VinFrameRate);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_VIDEO_IN_MIRRORMODE > VidInParamPtr->s_VinMirrorPattern
            || MAX_VIDEO_IN_MIRRORMODE < VidInParamPtr->s_VinMirrorPattern)
    {
        SYS_ERROR("video in mirror pattern = %d incorrect\n", VidInParamPtr->s_VinMirrorPattern);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "video in mirror pattern = %d incorrect\n", VidInParamPtr->s_VinMirrorPattern);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_VIDEO_IN_BAYERMODE > VidInParamPtr->s_VinBayerPattern
            || MAX_VIDEO_IN_BAYERMODE < VidInParamPtr->s_VinBayerPattern)
    {
        SYS_ERROR("video in bayer pattern = %d incorrect\n", VidInParamPtr->s_VinBayerPattern);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "video in bayer pattern = %d incorrect\n", VidInParamPtr->s_VinBayerPattern);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_VIDEO_OUT_FLIP > VidOutParamPtr->s_VoutFlip
            || MAX_VIDEO_OUT_FLIP < VidOutParamPtr->s_VoutFlip)
    {
        SYS_ERROR("video out flip = %d incorrect\n", VidOutParamPtr->s_VoutFlip);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "video out flip = %d incorrect\n", VidOutParamPtr->s_VoutFlip);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_VIDEO_OUT_TYPE > VidOutParamPtr->s_VoutType
            || MAX_VIDEO_OUT_TYPE < VidOutParamPtr->s_VoutType)
    {
        SYS_ERROR("video out type = %d incorrect\n", VidOutParamPtr->s_VoutType);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "video out type = %d incorrect\n", VidOutParamPtr->s_VoutType);
        return GMI_INVALID_PARAMETER;
    }

    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::SetVinVoutConfiguration(FD_HANDLE Handle, VideoInParam *VidInParamPtr, VideoOutParam *VidOutParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    VideoInParam  Vin;
    size_t        VinParameterLength = sizeof(VideoInParam);
    VideoOutParam Vout;
    size_t        VoutParameterLength = sizeof(VideoOutParam);

    SYS_INFO("##User Set Video In Param:\n");
    SYS_INFO("Vin.Flag      = %d\n", VidInParamPtr->s_VinFlag);
    SYS_INFO("Vin.Mode      = %d\n", VidInParamPtr->s_VinMode);
    SYS_INFO("Vin.FrameRate = %d\n", VidInParamPtr->s_VinFrameRate);
    SYS_INFO("Vin.Mirror    = %d\n", VidInParamPtr->s_VinMirrorPattern);
    SYS_INFO("Vin.Bayer     = %d\n", VidInParamPtr->s_VinBayerPattern);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##User Set Video In Param:\n");
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Vin.Flag      = %d\n", VidInParamPtr->s_VinFlag);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Vin.Mode      = %d\n", VidInParamPtr->s_VinMode);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Vin.FrameRate = %d\n", VidInParamPtr->s_VinFrameRate);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Vin.Mirror    = %d\n", VidInParamPtr->s_VinMirrorPattern);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Vin.Bayer     = %d\n", VidInParamPtr->s_VinBayerPattern);

    GMI_RESULT Result = CheckVinVoutConfiguration(VidInParamPtr, VidOutParamPtr);
    if (FAILED(Result))
    {
        SYS_ERROR("CheckVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CheckVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    memset(&Vin, 0, VinParameterLength);
    memcpy(&Vin, VidInParamPtr, VinParameterLength);
    memset(&Vout, 0, VoutParameterLength);
    memcpy(&Vout, VidOutParamPtr, VoutParameterLength);

    Result = m_MediaCenter.SetVinVoutConfig(Handle, &Vin, VinParameterLength, &Vout, VoutParameterLength);
    if (FAILED(Result))
    {
        SYS_ERROR("SetVinVoutConfiguration fail, Result = 0x%lx\n", Result );
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetVinVoutConfiguration fail, Result = 0x%lx\n", Result );
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::GetVinVoutConfiguration(FD_HANDLE Handle, VideoInParam *VidInParamPtr, VideoOutParam *VidOutParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    size_t        VinParameterLength = sizeof(VideoInParam);
    size_t        VoutParameterLength = sizeof(VideoOutParam);

    GMI_RESULT Result = m_MediaCenter.GetVinVoutConfig(Handle, VidInParamPtr, &VinParameterLength, VidOutParamPtr, &VoutParameterLength );
    if (FAILED(Result))
    {
        SYS_ERROR("GetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetVinVoutConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SYS_INFO("##User Get Video In Param:\n");
    SYS_INFO("Vin.Flag      = %d\n", VidInParamPtr->s_VinFlag);
    SYS_INFO("Vin.Mode      = %d\n", VidInParamPtr->s_VinMode);
    SYS_INFO("Vin.FrameRate = %d\n", VidInParamPtr->s_VinFrameRate);
    SYS_INFO("Vin.Mirror    = %d\n", VidInParamPtr->s_VinMirrorPattern);
    SYS_INFO("Vin.Bayer     = %d\n", VidInParamPtr->s_VinBayerPattern);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "##User Set Video In Param:\n");
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Vin.Flag      = %d\n", VidInParamPtr->s_VinFlag);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Vin.Mode      = %d\n", VidInParamPtr->s_VinMode);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Vin.FrameRate = %d\n", VidInParamPtr->s_VinFrameRate);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Vin.Mirror    = %d\n", VidInParamPtr->s_VinMirrorPattern);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Vin.Bayer     = %d\n", VidInParamPtr->s_VinBayerPattern);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CheckWhiteBalanceConfiguration(ImageWbParam *ImageWbParamPtr)
{
    if (MIN_IMAGE_WB_MODE > ImageWbParamPtr->s_WbMode
            || MAX_IMAGE_WB_MODE < ImageWbParamPtr->s_WbMode)
    {
        SYS_ERROR("ImageWbParamPtr->s_WbMode = %d incorrect\n", ImageWbParamPtr->s_WbMode);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageWbParamPtr->s_WbMode = %d incorrect\n", ImageWbParamPtr->s_WbMode);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_WB_RGAIN > ImageWbParamPtr->s_WbBgain
            || MAX_IMAGE_WB_RGAIN < ImageWbParamPtr->s_WbBgain)
    {
        SYS_ERROR("ImageWbParamPtr->s_WbBgain = %d incorrect\n", ImageWbParamPtr->s_WbBgain);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageWbParamPtr->s_WbBgain = %d incorrect\n", ImageWbParamPtr->s_WbBgain);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_WB_BGAIN > ImageWbParamPtr->s_WbRgain
            || MAX_IMAGE_WB_BGAIN < ImageWbParamPtr->s_WbRgain)
    {
        SYS_ERROR("ImageWbParamPtr->s_WbRgain = %d incorrect\n", ImageWbParamPtr->s_WbRgain);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ImageWbParamPtr->s_WbRgain = %d incorrect\n", ImageWbParamPtr->s_WbRgain);
        return GMI_INVALID_PARAMETER;
    }

    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::SetWhiteBalanceConfiguration(FD_HANDLE Handle, ImageWbParam *ImageWbParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    ImageWbParam  ImgWbParam;
    size_t        ImageWbParamLength = sizeof(ImageWbParam);

    memset(&ImgWbParam, 0, sizeof(ImageWbParam));
    memcpy(&ImgWbParam, ImageWbParamPtr, ImageWbParamLength);

    SYS_INFO("ImageWbParam.s_WbMode = %d\n",  ImgWbParam.s_WbMode);
    SYS_INFO("ImageWbParam.s_WbRgain = %d\n", ImgWbParam.s_WbRgain);
    SYS_INFO("ImageWbParam.s_WbBgain = %d\n", ImgWbParam.s_WbBgain);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageWbParam.s_WbMode = %d\n",  ImgWbParam.s_WbMode);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageWbParam.s_WbRgain = %d\n", ImgWbParam.s_WbRgain);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageWbParam.s_WbBgain = %d\n", ImgWbParam.s_WbBgain);

    GMI_RESULT Result = CheckWhiteBalanceConfiguration(&ImgWbParam);
    if (FAILED(Result))
    {
        SYS_ERROR("CheckWhiteBalanceConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CheckWhiteBalanceConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_MediaCenter.SetWhiteBalanceConfig(Handle, &ImgWbParam, ImageWbParamLength);
    if (FAILED(Result))
    {
        SYS_ERROR("SetWhiteBalanceConfig fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetWhiteBalanceConfig fail, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::GetWhiteBalanceConfiguration(FD_HANDLE Handle, ImageWbParam *ImageWbParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    size_t  ImageWbParamLength = sizeof(ImageWbParam);

    GMI_RESULT Result = m_MediaCenter.GetWhiteBalanceConfig(Handle, ImageWbParamPtr, &ImageWbParamLength);
    if (FAILED(Result))
    {
        SYS_ERROR("GetWhiteBalanceConfig fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetWhiteBalanceConfig fail, Result = 0x%lx\n", Result);
        return Result;
    }
    SYS_INFO("ImageWbParam.s_WbMode = %d\n",  ImageWbParamPtr->s_WbMode);
    SYS_INFO("ImageWbParam.s_WbRgain = %d\n", ImageWbParamPtr->s_WbRgain);
    SYS_INFO("ImageWbParam.s_WbBgain = %d\n", ImageWbParamPtr->s_WbBgain);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageWbParam.s_WbMode = %d\n",  ImageWbParamPtr->s_WbMode);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageWbParam.s_WbRgain = %d\n", ImageWbParamPtr->s_WbRgain);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "ImageWbParam.s_WbBgain = %d\n", ImageWbParamPtr->s_WbBgain);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::GetDaynightConfiguration(FD_HANDLE Handle, ImageDnParam *ImageDaynightParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    size_t  ImageDaynightParamLength = sizeof(ImageDnParam);

    GMI_RESULT Result = m_MediaCenter.GetDaynightConfig(Handle, ImageDaynightParamPtr, &ImageDaynightParamLength);
    if (FAILED(Result))
    {
        SYS_ERROR("get daynight config fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "get daynight config fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SYS_INFO("daynight mode          = %d\n", ImageDaynightParamPtr->s_DnMode);
    SYS_INFO("daynight duration time = %d\n", ImageDaynightParamPtr->s_DnDurtime);
    SYS_INFO("night to day threshold = %d\n", ImageDaynightParamPtr->s_NightToDayThr);
    SYS_INFO("day to night threshold = %d\n", ImageDaynightParamPtr->s_DayToNightThr);
    for (int32_t i = 0; i < SCHEDULE_WEEK_DAYS; i++)
    {
        SYS_INFO("day %d, Enable    = %d\n", i, ImageDaynightParamPtr->s_DnSchedule.s_DnSchedFlag[i]);
        SYS_INFO("day %d, StartTime = %d", i, ImageDaynightParamPtr->s_DnSchedule.s_StartTime[i]);
        SYS_INFO("day %d, EndTime   = %d", i, ImageDaynightParamPtr->s_DnSchedule.s_EndTime[i]);
    }

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "daynight mode          = %d\n", ImageDaynightParamPtr->s_DnMode);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "daynight duration time = %d\n", ImageDaynightParamPtr->s_DnDurtime);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "night to day threshold = %d\n", ImageDaynightParamPtr->s_NightToDayThr);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "day to night threshold = %d\n", ImageDaynightParamPtr->s_DayToNightThr);
    for (int32_t i = 0; i < SCHEDULE_WEEK_DAYS; i++)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "day %d, Enable    = %d\n", i, ImageDaynightParamPtr->s_DnSchedule.s_DnSchedFlag[i]);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "day %d, StartTime = %d", i, ImageDaynightParamPtr->s_DnSchedule.s_StartTime[i]);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "day %d, EndTime   = %d", i, ImageDaynightParamPtr->s_DnSchedule.s_EndTime[i]);
    }

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CheckDaynightConfiguration(ImageDnParam *ImgDnParamPtr)
{
    if (MIN_IMAGE_DN_MODE > ImgDnParamPtr->s_DnMode
            || MAX_IMAGE_DN_MODE < ImgDnParamPtr->s_DnMode)
    {
        SYS_ERROR("Daynight mode %d error\n", ImgDnParamPtr->s_DnMode);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Daynight mode %d error\n", ImgDnParamPtr->s_DnMode);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_DN_DURTIME > ImgDnParamPtr->s_DnDurtime
            || MAX_IMAGE_DN_DURTIME < ImgDnParamPtr->s_DnDurtime)
    {
        SYS_ERROR("Daynight duration time %d error\n", ImgDnParamPtr->s_DnDurtime);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Daynight duration time %d error\n", ImgDnParamPtr->s_DnDurtime);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_DN_NTOD > ImgDnParamPtr->s_NightToDayThr
            || MAX_IMAGE_DN_NTOD < ImgDnParamPtr->s_NightToDayThr)
    {
        SYS_ERROR("Daynight night to day threshold %d error\n", ImgDnParamPtr->s_NightToDayThr);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Daynight night to day threshold %d error\n", ImgDnParamPtr->s_NightToDayThr);
        return GMI_INVALID_PARAMETER;
    }

    if (MIN_IMAGE_DN_DTON > ImgDnParamPtr->s_DayToNightThr
            || MAX_IMAGE_DN_DTON < ImgDnParamPtr->s_DayToNightThr)
    {
        SYS_ERROR("Daynight day to night threshold %d error\n", ImgDnParamPtr->s_DayToNightThr);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Daynight day to night threshold %d error\n", ImgDnParamPtr->s_DayToNightThr);
        return GMI_INVALID_PARAMETER;
    }

    for (int32_t i = 0; i < SCHEDULE_WEEK_DAYS; i++)
    {
        if (MIN_IMAGE_DN_SCHEDFLAG != ImgDnParamPtr->s_DnSchedule.s_DnSchedFlag[i]
                && MAX_IMAGE_DN_SCHEDFLAG != ImgDnParamPtr->s_DnSchedule.s_DnSchedFlag[i])
        {
            SYS_ERROR("DAY%d schedule enable %d error\n", i, ImgDnParamPtr->s_DnSchedule.s_DnSchedFlag[i]);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DAY%d schedule enable %d error\n", i, ImgDnParamPtr->s_DnSchedule.s_DnSchedFlag[i]);
            return GMI_INVALID_PARAMETER;
        }

        if (MIN_IMAGE_DN_SCHEDTIME > ImgDnParamPtr->s_DnSchedule.s_StartTime[i]
                || MAX_IMAGE_DN_SCHEDTIME < ImgDnParamPtr->s_DnSchedule.s_StartTime[i])
        {
            SYS_ERROR("DAY%d schedule start time %d error\n", i, ImgDnParamPtr->s_DnSchedule.s_StartTime[i]);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DAY%d schedule start time %d error\n", i, ImgDnParamPtr->s_DnSchedule.s_StartTime[i]);
            return GMI_INVALID_PARAMETER;
        }

        if (MIN_IMAGE_DN_SCHEDTIME > ImgDnParamPtr->s_DnSchedule.s_EndTime[i]
                || MAX_IMAGE_DN_SCHEDTIME < ImgDnParamPtr->s_DnSchedule.s_EndTime[i])
        {
            SYS_ERROR("DAY%d schedule end time %d error\n", i, ImgDnParamPtr->s_DnSchedule.s_EndTime[i]);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DAY%d schedule end time %d error\n", i, ImgDnParamPtr->s_DnSchedule.s_EndTime[i]);
            return GMI_INVALID_PARAMETER;
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::SetDaynightConfiguration(FD_HANDLE Handle, ImageDnParam *ImageDaynightParamPtr)
{
    SYS_INFO("%s:%s in..........\n", __FILE__, __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in..........\n", __FILE__, __func__);
    size_t  ImageDaynightParamLength = sizeof(ImageDnParam);
    ImageDnParam ImgDaynightParam;

    memset(&ImgDaynightParam, 0, sizeof(ImageDnParam));
    memcpy(&ImgDaynightParam, ImageDaynightParamPtr, sizeof(ImageDnParam));

    SYS_INFO("daynight mode          = %d\n", ImgDaynightParam.s_DnMode);
    SYS_INFO("daynight duration time = %d\n", ImgDaynightParam.s_DnDurtime);
    SYS_INFO("night to day threshold = %d\n", ImgDaynightParam.s_NightToDayThr);
    SYS_INFO("day to night threshold = %d\n", ImgDaynightParam.s_DayToNightThr);
    for (int32_t i = 0; i < SCHEDULE_WEEK_DAYS; i++)
    {
        SYS_INFO("day %d, Enable    = %d\n", i, ImgDaynightParam.s_DnSchedule.s_DnSchedFlag[i]);
        SYS_INFO("day %d, StartTime = %d",   i, ImgDaynightParam.s_DnSchedule.s_StartTime[i]);
        SYS_INFO("day %d, EndTime   = %d",   i, ImgDaynightParam.s_DnSchedule.s_EndTime[i]);
    }

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "daynight mode          = %d\n", ImgDaynightParam.s_DnMode);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "daynight duration time = %d\n", ImgDaynightParam.s_DnDurtime);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "night to day threshold = %d\n", ImgDaynightParam.s_NightToDayThr);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "day to night threshold = %d\n", ImgDaynightParam.s_DayToNightThr);
    for (int32_t i = 0; i < SCHEDULE_WEEK_DAYS; i++)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "day %d, Enable    = %d\n", i, ImgDaynightParam.s_DnSchedule.s_DnSchedFlag[i]);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "day %d, StartTime = %d",   i, ImgDaynightParam.s_DnSchedule.s_StartTime[i]);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "day %d, EndTime   = %d",   i, ImgDaynightParam.s_DnSchedule.s_EndTime[i]);
    }

    GMI_RESULT Result = CheckDaynightConfiguration(&ImgDaynightParam);
    if (FAILED(Result))
    {
        SYS_ERROR("CheckDaynightConfiguration fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CheckDaynightConfiguration fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_MediaCenter.SetDaynightConfig(Handle, &ImgDaynightParam, ImageDaynightParamLength);
    if (FAILED(Result))
    {
        SYS_ERROR("set daynight config fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "set daynight config fail, Result = 0x%lx\n", Result);
        return Result;
    }

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    SYS_INFO("%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::OpenAutoFocusDevice(uint32_t VidSourceId, FD_HANDLE *Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.OpenAutoFocusDevice(VidSourceId, Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter OpenAutoFocusDevice failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter OpenAutoFocusDevice failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CloseAutoFocusDevice(FD_HANDLE Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.CloseAutoFocusDevice(Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter CloseAutoFocusDevice failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter CloseAutoFocusDevice failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::StartAutoFocusDevice(FD_HANDLE Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.StartAutoFocus(Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter StartAutoFocus failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter StartAutoFocus failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::StopAutoFocusDevice(FD_HANDLE Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.StopAutoFocus(Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter StopAutoFocus failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter StopAutoFocus failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::PauseAutoFocus(FD_HANDLE Handle, boolean_t Pause)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.PauseAutoFocus(Handle, Pause);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter PauseAutoFocus failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter PauseAutoFocus failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::AutoFocusGlobalScan(FD_HANDLE Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.AutoFocusGlobalScan(Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter AutoFocusGlobalScan failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter AutoFocusGlobalScan failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::SetAutoFocusMode(FD_HANDLE Handle, int32_t Mode)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.SetAutoFocusMode(Handle, Mode);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter AutoFocusGlobalScan failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter AutoFocusGlobalScan failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::ControlAutoFocus(FD_HANDLE Handle, int32_t Mode)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.ControlAutoFocus(Handle, Mode);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter ControlAutoFocus failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter ControlAutoFocus failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::OpenZoomDevice(uint32_t VidSourceId, FD_HANDLE *Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.OpenZoomDevice(0, Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter OpenZoomDevice failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter OpenZoomDevice failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::CloseZoomDevice(FD_HANDLE Handle)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.CloseZoomDevice(Handle);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter CloseZoomDevice failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter CloseZoomDevice failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::SetZoomPosition(FD_HANDLE Handle, int32_t Position)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.SetZoomPosition(Handle, Position);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter SetZoomPosition failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter SetZoomPosition failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::GetZoomPosition(FD_HANDLE Handle, int32_t *PositionPtr)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.GetZoomPosition(Handle, PositionPtr);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter GetZoomPostion failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter GetZoomPostion failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::GetZoomRange(FD_HANDLE Handle, int32_t *MinPositionPtr, int32_t *MaxPositionPtr)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.GetZoomRange(Handle, MinPositionPtr, MaxPositionPtr);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter GetZoomRange failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter GetZoomRange failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::ControlZoom(FD_HANDLE Handle, int8_t Mode)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.ControlZoom(Handle, Mode);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter ControlZoom failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter ControlZoom failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::SetZoomStep(FD_HANDLE Handle, int32_t Step)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s in.........\n", __FILE__, __func__);
    GMI_RESULT Result = m_MediaCenter.SetZoomStep(Handle, Step);
    if (FAILED(Result))
    {
        SYS_ERROR("m_MediaCenter SetZoomStep failed, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter SetZoomStep failed, Result = 0x%lx\n", Result);
        return Result;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s:%s normal out........\n", __FILE__, __func__);
    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::StartZoom(FD_HANDLE FocusHandle, FD_HANDLE ZoomHandle, int8_t Mode)
{
    //GMI_RESULT Result = m_MediaCenter.StartZoom(FocusHandle, 1, ZoomHandle, Mode);
    //if (FAILED(Result))
    //{
    //    SYS_ERROR("m_MediaCenter StartZoom fail, Result = 0x%lx\n", Result);
    //    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter StartZoom fail, Result = 0x%lx\n", Result);
    //    return Result;
    //}

    return GMI_SUCCESS;
}


GMI_RESULT StreamCenterClient::StopZoom(FD_HANDLE FocusHandle, FD_HANDLE ZoomHandle, int32_t *PositionPtr)
{
    //int32_t ZoomPosition;

    //GMI_RESULT Result = m_MediaCenter.StopZoom(FocusHandle, 0, ZoomHandle, ZOOM_MODE_STOP, &ZoomPosition);
    //if (FAILED(Result))
    //{
    //    SYS_ERROR("m_MediaCenter StopZoom fail, Result = 0x%lx\n", Result);
    //    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_MediaCenter StopZoom fail, Result = 0x%lx\n", Result);
    //    return Result;
    //}

    //*PositionPtr = ZoomPosition;

    return GMI_SUCCESS;
}

