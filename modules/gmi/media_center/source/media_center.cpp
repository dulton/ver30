#include "media_center.h"

#include "media_codec_parameter.h"
#include "media_decode_pipeline.h"
#include "media_encode_pipeline.h"
#include "share_memory_log_client.h"
#include "timer_task_queue.h"

MediaCenter::MediaCenter(void)
    : m_EncodePipelines()
    , m_DecodePipelines()
{
}

MediaCenter::~MediaCenter(void)
{
}

GMI_RESULT MediaCenter::OpenVinVoutDevice( uint16_t SensorId, uint16_t ChannelId, FD_HANDLE *VinVoutHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer vin vout to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::OpenVinVoutDevice begin, passed SensorId=%d, ChannelId=%d \n", SensorId, ChannelId );

    *VinVoutHandle = GMI_VinVoutCreate( SensorId, ChannelId );
    if ( NULL == *VinVoutHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::OpenVinVoutDevice, vin vout device is not opened, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::OpenVinVoutDevice end, function return %x \n", SensorId, ChannelId, GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::CloseVinVoutDevice( FD_HANDLE& VinVoutHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer vin vout to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CloseVinVoutDevice begin, passed VinVoutHandle=%p \n", VinVoutHandle );

    if ( NULL == VinVoutHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::CloseVinVoutDevice, vin vout device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_VinVoutDestroy( VinVoutHandle );
    VinVoutHandle = NULL;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CloseVinVoutDevice end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::GetVinVoutConfig( FD_HANDLE VinVoutHandle, void_t *VinParameter, size_t *VinParameterLength, void_t *VoutParameter, size_t *VoutParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer vin vout to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetVinVoutConfig begin, passed VinParameterLength=%d, VoutParameterLength=%d \n", *VinParameterLength, *VoutParameterLength );

    if ( NULL == VinVoutHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetVinVoutConfig, vin vout device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(VideoInParam) > *VinParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetVinVoutConfig, passed vin buffer size is too little(%d), actually need %d bytes, function return %x \n", *VinParameterLength, sizeof(VideoInParam), (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    if ( sizeof(VideoOutParam) > *VoutParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetVinVoutConfig, passed vout buffer size is too little(%d), actually need %d bytes, function return %x \n", *VoutParameterLength, sizeof(VideoOutParam), (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    GMI_RESULT Result = GMI_VinVoutGetConfig( VinVoutHandle, (VideoInParam*)VinParameter, (VideoOutParam*)VoutParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetVinVoutConfig, GMI_VinVoutGetConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetVinVoutConfig: VinFlag=%d \n",         ((VideoInParam*)VinParameter)->s_VinFlag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetVinVoutConfig: VinMode=%d \n",         ((VideoInParam*)VinParameter)->s_VinMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetVinVoutConfig: VinFrameRate=%d \n",    ((VideoInParam*)VinParameter)->s_VinFrameRate );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetVinVoutConfig: VinMirrorPattern=%d \n",((VideoInParam*)VinParameter)->s_VinMirrorPattern );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetVinVoutConfig: VinBayerPattern=%d \n", ((VideoInParam*)VinParameter)->s_VinBayerPattern );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetVinVoutConfig: VoutMode=%d \n",        ((VideoOutParam*)VoutParameter)->s_VoutMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetVinVoutConfig: VoutType=%d \n",        ((VideoOutParam*)VoutParameter)->s_VoutType );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetVinVoutConfig: VoutFlip=%d \n",        ((VideoOutParam*)VoutParameter)->s_VoutFlip );
#endif

    *VinParameterLength = sizeof(VideoInParam);
    *VoutParameterLength = sizeof(VideoOutParam);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetVinVoutConfig end and return %x, returned VinParameterLength=%d, VoutParameterLength=%d \n", (uint32_t) Result, *VinParameterLength, *VoutParameterLength );
    return Result;
}

GMI_RESULT MediaCenter::SetVinVoutConfig( FD_HANDLE VinVoutHandle, const void_t *VinParameter, size_t VinParameterLength, const void_t *VoutParameter, size_t VoutParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer vin vout to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetVinVoutConfig begin, passed VinParameterLength=%d, VoutParameterLength=%d \n", VinParameterLength, VoutParameterLength );

    if ( NULL == VinVoutHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetVinVoutConfig, vin vout device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(VideoInParam) > VinParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetVinVoutConfig, passed vin buffer size is too little(%d), actually need %d bytes, function return %x \n", VinParameterLength, sizeof(VideoInParam), (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

    if ( sizeof(VideoOutParam) > VoutParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetVinVoutConfig, passed vout buffer size is too little(%d), actually need %d bytes, function return %x \n", VoutParameterLength, sizeof(VideoOutParam), (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetVinVoutConfig: VinFlag=%d \n",         ((VideoInParam*)VinParameter)->s_VinFlag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetVinVoutConfig: VinMode=%d \n",         ((VideoInParam*)VinParameter)->s_VinMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetVinVoutConfig: VinFrameRate=%d \n",    ((VideoInParam*)VinParameter)->s_VinFrameRate );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetVinVoutConfig: VinMirrorPattern=%d \n",((VideoInParam*)VinParameter)->s_VinMirrorPattern );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetVinVoutConfig: VinBayerPattern=%d \n", ((VideoInParam*)VinParameter)->s_VinBayerPattern );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetVinVoutConfig: VoutMode=%d \n",        ((VideoOutParam*)VoutParameter)->s_VoutMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetVinVoutConfig: VoutType=%d \n",        ((VideoOutParam*)VoutParameter)->s_VoutType );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetVinVoutConfig: VoutFlip=%d \n",        ((VideoOutParam*)VoutParameter)->s_VoutFlip );
#endif

    GMI_RESULT Result = GMI_VinVoutSetConfig( VinVoutHandle, (VideoInParam*)VinParameter, (VideoOutParam*)VoutParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetVinVoutConfig, GMI_VinVoutSetConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetVinVoutConfig end, function return %x \n", Result );
    return Result;
}

GMI_RESULT MediaCenter::CreateCodec( boolean_t EncodeMode, uint32_t SourceId, uint32_t MediaId, uint32_t MediaType, uint32_t CodecType, void_t *CodecParameter, size_t CodecParameterLength, FD_HANDLE *CodecHandle )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CreateCodec begin, EncodeMode=%d, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d, CodecParameterLength=%d \n", EncodeMode, SourceId, MediaId, MediaType, CodecType, CodecParameterLength );

    GMI_RESULT Result = GMI_FAIL;
    if ( EncodeMode )
    {
        std::vector< SafePtr<MediaEncodePipeline> >::iterator EncodePipelineIt = m_EncodePipelines.begin(), EncodePipelineEnd = m_EncodePipelines.end();
        for ( ; EncodePipelineIt != EncodePipelineEnd; ++EncodePipelineIt )
        {
            if ( SourceId  == (*EncodePipelineIt)->GetSourceId() &&
                    MediaId   == (*EncodePipelineIt)->GetMediaId() &&
                    MediaType == (*EncodePipelineIt)->GetMediaType() &&
                    CodecType == (*EncodePipelineIt)->GetCodecType() )
            {
                const size_t MaxCodecParameterLength = 1024;
                size_t InternalCodecParameterLength = MaxCodecParameterLength;
                uint8_t InternalCodecParameter[MaxCodecParameterLength];
                Result = (*EncodePipelineIt)->GetEncodeConfig( InternalCodecParameter, &InternalCodecParameterLength );
                if ( FAILED( Result ) )
                {
                    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::CreateCodec, GetEncodeConfig failed, function return %x \n", (uint32_t) Result );
                    return Result;
                }

                if ( InternalCodecParameterLength == CodecParameterLength &&
                        0 == memcmp( InternalCodecParameter, CodecParameter, InternalCodecParameterLength ) )
                {
                    *CodecHandle = EncodePipelineIt->GetPtr();
                    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "MediaCenter::CreateCodec, encoding parameter is same, function return %x \n", (uint32_t) GMI_SUCCESS );
                    return GMI_SUCCESS;
                }
                else
                {
                    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CreateCodec, encoding parameter is different, restart encoding \n" );
                    if ( MEDIA_VIDEO == MediaType && 0 == SourceId && 0 == MediaId )
                    {
                        std::vector< SafePtr<MediaEncodePipeline> >::reverse_iterator ReverseIt = m_EncodePipelines.rbegin(), ReverseEnd = m_EncodePipelines.rend();

                        for ( ; ReverseIt != ReverseEnd; ++ReverseIt )
                        {
                            (*ReverseIt)->Stop();
                            (*ReverseIt)->Deinitialize();
                        }

                        m_EncodePipelines.clear();
                    }
                    else
                    {
                        (*EncodePipelineIt)->Stop();
                        (*EncodePipelineIt)->Deinitialize();
                        m_EncodePipelines.erase( EncodePipelineIt );
                    }
                    break;
                }
            }
        }

        SafePtr<MediaEncodePipeline> EncodePipeline( BaseMemoryManager::Instance().New<MediaEncodePipeline>() );
        if ( NULL == EncodePipeline.GetPtr() )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::CreateCodec, encoding pipeline object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
            return GMI_OUT_OF_MEMORY;
        }

        Result = EncodePipeline->Initialize( SourceId, MediaId, MediaType, CodecType, CodecParameter, CodecParameterLength );
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::CreateCodec, encoding pipeline object initialization failed, function return %x \n", (uint32_t) Result );
            return Result;
        }

        *CodecHandle = EncodePipeline.GetPtr();

        m_EncodePipelines.push_back( EncodePipeline );
    }
    else
    {
        std::vector< SafePtr<MediaDecodePipeline> >::iterator DecodePipelineIt = m_DecodePipelines.begin(), DecodePipelineEnd = m_DecodePipelines.end();
        for ( ; DecodePipelineIt != DecodePipelineEnd; ++DecodePipelineIt )
        {
            if ( SourceId  == (*DecodePipelineIt)->GetSourceId() &&
                    MediaId   == (*DecodePipelineIt)->GetMediaId() &&
                    MediaType == (*DecodePipelineIt)->GetMediaType() &&
                    CodecType == (*DecodePipelineIt)->GetCodecType() )
            {
                const size_t MaxCodecParameterLength = 1024;
                size_t InternalCodecParameterLength = MaxCodecParameterLength;
                uint8_t InternalCodecParameter[MaxCodecParameterLength];
                Result = (*DecodePipelineIt)->GetDecodeConfig( InternalCodecParameter, &InternalCodecParameterLength );
                if ( FAILED( Result ) )
                {
                    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::CreateCodec, GetDecodeConfig failed, function return %x \n", (uint32_t) Result );
                    return Result;
                }

                if ( InternalCodecParameterLength == CodecParameterLength &&
                        0 == memcmp( InternalCodecParameter, CodecParameter, InternalCodecParameterLength ) )
                {
                    *CodecHandle = DecodePipelineIt->GetPtr();
                    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "MediaCenter::CreateCodec, decoding parameter is same, function return %x \n", (uint32_t) GMI_SUCCESS );
                    return GMI_SUCCESS;
                }
                else
                {
                    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CreateCodec, decoding parameter is different, restart encoding \n" );
                    (*DecodePipelineIt)->Stop();
                    (*DecodePipelineIt)->Deinitialize();
                    m_DecodePipelines.erase( DecodePipelineIt );
                }
            }
        }

        SafePtr<MediaDecodePipeline> DecodePipeline( BaseMemoryManager::Instance().New<MediaDecodePipeline>() );
        if ( NULL == DecodePipeline.GetPtr() )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::CreateCodec, decoding pipeline object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
            return GMI_OUT_OF_MEMORY;
        }

        Result = DecodePipeline->Initialize( SourceId, MediaId, MediaType, CodecType, CodecParameter, CodecParameterLength );
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::CreateCodec, decoding pipeline object initialization failed, function return %x \n", (uint32_t) Result );
            return Result;
        }

        *CodecHandle = DecodePipeline.GetPtr();

        m_DecodePipelines.push_back( DecodePipeline );
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CreateCodec end, function return %x \n", (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::DestroyCodec( FD_HANDLE& CodecHandle )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::DestroyCodec begin, CodecHandle=%p \n", CodecHandle );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::DestroyCodec, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FAIL;
    MediaEncodePipeline *EncodePipeline = reinterpret_cast<MediaEncodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaEncodePipeline> >::iterator EncodePipelineIt = m_EncodePipelines.begin(), EncodePipelineEnd = m_EncodePipelines.end();
    for ( ; EncodePipelineIt != EncodePipelineEnd; ++EncodePipelineIt )
    {
        if ( EncodePipeline == EncodePipelineIt->GetPtr() )
        {
            Result = EncodePipeline->Stop();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "MediaCenter::DestroyCodec, trying to EncodePipeline::Stop failed before destroying encoder, CodecHandle=%p, Stop return %x, code flow go on... \n", CodecHandle, (uint32_t) Result );
            }

            Result = EncodePipeline->Deinitialize();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::DestroyCodec, EncodePipeline::Deinitialize failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }

            m_EncodePipelines.erase( EncodePipelineIt );
            CodecHandle = NULL;
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::DestroyCodec end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    MediaDecodePipeline *DecodePipeline = reinterpret_cast<MediaDecodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaDecodePipeline> >::iterator DecodePipelineIt = m_DecodePipelines.begin(), DecodePipelineEnd = m_DecodePipelines.end();
    for ( ; DecodePipelineIt != DecodePipelineEnd; ++DecodePipelineIt )
    {
        if ( DecodePipeline == DecodePipelineIt->GetPtr() )
        {
            Result = DecodePipeline->Stop();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Warning, "MediaCenter::DestroyCodec, trying to EncodePipeline::Stop failed before destroying decoder, CodecHandle=%p, Stop return %x, code flow go on... \n", CodecHandle, (uint32_t) Result );
            }

            Result = DecodePipeline->Deinitialize();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::DestroyCodec, DecodePipeline::Deinitialize failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }

            m_DecodePipelines.erase( DecodePipelineIt );
            CodecHandle = NULL;
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::DestroyCodec end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::DestroyCodec, does not find device, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) GMI_INVALID_PARAMETER );
    return GMI_INVALID_PARAMETER;
}

GMI_RESULT MediaCenter::StartCodec( FD_HANDLE CodecHandle )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::StartCodec begin, CodecHandle=%p \n", CodecHandle );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StartCodec, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FAIL;
    MediaEncodePipeline *EncodePipeline = reinterpret_cast<MediaEncodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaEncodePipeline> >::iterator EncodePipelineIt = m_EncodePipelines.begin(), EncodePipelineEnd = m_EncodePipelines.end();
    for ( ; EncodePipelineIt != EncodePipelineEnd; ++EncodePipelineIt )
    {
        if ( EncodePipeline == EncodePipelineIt->GetPtr() )
        {
            Result = EncodePipeline->Play();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StartCodec, EncodePipeline::Play failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }

            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::StartCodec end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    MediaDecodePipeline *DecodePipeline = reinterpret_cast<MediaDecodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaDecodePipeline> >::iterator DecodePipelineIt = m_DecodePipelines.begin(), DecodePipelineEnd = m_DecodePipelines.end();
    for ( ; DecodePipelineIt != DecodePipelineEnd; ++DecodePipelineIt )
    {
        if ( DecodePipeline == DecodePipelineIt->GetPtr() )
        {
            Result = DecodePipeline->Play();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StartCodec, DecodePipeline::Play failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }

            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::StartCodec end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StartCodec, does not find device, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) GMI_INVALID_PARAMETER );
    return GMI_INVALID_PARAMETER;
}

GMI_RESULT MediaCenter::StopCodec( FD_HANDLE CodecHandle )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::StopCodec begin, CodecHandle=%p \n", CodecHandle );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StopCodec, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FAIL;
    MediaEncodePipeline *EncodePipeline = reinterpret_cast<MediaEncodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaEncodePipeline> >::iterator EncodePipelineIt = m_EncodePipelines.begin(), EncodePipelineEnd = m_EncodePipelines.end();
    for ( ; EncodePipelineIt != EncodePipelineEnd; ++EncodePipelineIt )
    {
        if ( EncodePipeline == EncodePipelineIt->GetPtr() )
        {
            Result = EncodePipeline->Stop();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StopCodec, EncodePipeline::Stop failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }

            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::StopCodec end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    MediaDecodePipeline *DecodePipeline = reinterpret_cast<MediaDecodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaDecodePipeline> >::iterator DecodePipelineIt = m_DecodePipelines.begin(), DecodePipelineEnd = m_DecodePipelines.end();
    for ( ; DecodePipelineIt != DecodePipelineEnd; ++DecodePipelineIt )
    {
        if ( DecodePipeline == DecodePipelineIt->GetPtr() )
        {
            Result = DecodePipeline->Stop();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StopCodec, DecodePipeline::Stop failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }

            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::StopCodec end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StopCodec, does not find device, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) GMI_INVALID_PARAMETER );
    return GMI_INVALID_PARAMETER;
}

GMI_RESULT MediaCenter::GetCodecConfig( FD_HANDLE CodecHandle, void_t *CodecParameter, size_t *CodecParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetCodecConfig begin, CodecHandle=%p, CodecParameterLength=%d \n", CodecHandle, *CodecParameterLength );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetCodecConfig, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FAIL;
    MediaEncodePipeline *EncodePipeline = reinterpret_cast<MediaEncodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaEncodePipeline> >::iterator EncodePipelineIt = m_EncodePipelines.begin(), EncodePipelineEnd = m_EncodePipelines.end();
    for ( ; EncodePipelineIt != EncodePipelineEnd; ++EncodePipelineIt )
    {
        if ( EncodePipeline == EncodePipelineIt->GetPtr() )
        {
            Result = EncodePipeline->GetEncodeConfig( CodecParameter, CodecParameterLength );
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetCodecConfig, EncodePipeline::GetEncodeConfig failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetCodecConfig end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    MediaDecodePipeline *DecodePipeline = reinterpret_cast<MediaDecodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaDecodePipeline> >::iterator DecodePipelineIt = m_DecodePipelines.begin(), DecodePipelineEnd = m_DecodePipelines.end();
    for ( ; DecodePipelineIt != DecodePipelineEnd; ++DecodePipelineIt )
    {
        if ( DecodePipeline == DecodePipelineIt->GetPtr() )
        {
            Result = DecodePipeline->GetDecodeConfig( CodecParameter, CodecParameterLength );
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetCodecConfig, DecodePipeline::GetDecodeConfig failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetCodecConfig end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetCodecConfig, does not find device, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) GMI_INVALID_PARAMETER );
    return GMI_INVALID_PARAMETER;
}

GMI_RESULT MediaCenter::SetCodecConfig( FD_HANDLE CodecHandle, const void_t *CodecParameter, size_t CodecParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetCodecConfig begin, CodecHandle=%p, CodecParameterLength=%d \n", CodecHandle, CodecParameterLength );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetCodecConfig, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FAIL;
    MediaEncodePipeline *EncodePipeline = reinterpret_cast<MediaEncodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaEncodePipeline> >::iterator EncodePipelineIt = m_EncodePipelines.begin(), EncodePipelineEnd = m_EncodePipelines.end();
    for ( ; EncodePipelineIt != EncodePipelineEnd; ++EncodePipelineIt )
    {
        if ( EncodePipeline == EncodePipelineIt->GetPtr() )
        {
            Result = EncodePipeline->SetEncodeConfig( CodecParameter, CodecParameterLength );
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetCodecConfig, EncodePipeline::SetCodecConfig failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetCodecConfig end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    MediaDecodePipeline *DecodePipeline = reinterpret_cast<MediaDecodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaDecodePipeline> >::iterator DecodePipelineIt = m_DecodePipelines.begin(), DecodePipelineEnd = m_DecodePipelines.end();
    for ( ; DecodePipelineIt != DecodePipelineEnd; ++DecodePipelineIt )
    {
        if ( DecodePipeline == DecodePipelineIt->GetPtr() )
        {
            Result = DecodePipeline->SetDecodeConfig( CodecParameter, CodecParameterLength );
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetCodecConfig, DecodePipeline::SetDecodeConfig failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetCodecConfig end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetCodecConfig, does not find device, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) GMI_INVALID_PARAMETER );
    return GMI_INVALID_PARAMETER;
}

GMI_RESULT MediaCenter::GetOsdConfig( FD_HANDLE CodecHandle, void_t *OsdParameter, size_t *OsdParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetOsdConfig begin, CodecHandle=%p, OsdParameterLength=%d \n", CodecHandle, *OsdParameterLength );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetOsdConfig, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FAIL;
    MediaEncodePipeline *EncodePipeline = reinterpret_cast<MediaEncodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaEncodePipeline> >::iterator PipelineIt = m_EncodePipelines.begin(), PipelineEnd = m_EncodePipelines.end();
    for ( ; PipelineIt != PipelineEnd; ++PipelineIt )
    {
        if ( EncodePipeline == PipelineIt->GetPtr() )
        {
            Result = EncodePipeline->GetOsdConfig( OsdParameter, OsdParameterLength );
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetOsdConfig, EncodePipeline::GetOsdConfig failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetOsdConfig end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetOsdConfig, does not find device, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) GMI_INVALID_PARAMETER );
    return GMI_INVALID_PARAMETER;
}

GMI_RESULT MediaCenter::SetOsdConfig( FD_HANDLE CodecHandle, const void_t *OsdParameter, size_t OsdParameterLength )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetOsdConfig begin, CodecHandle=%p, OsdParameterLength=%d \n", CodecHandle, OsdParameterLength );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetOsdConfig, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FAIL;
    MediaEncodePipeline *EncodePipeline = reinterpret_cast<MediaEncodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaEncodePipeline> >::iterator PipelineIt = m_EncodePipelines.begin(), PipelineEnd = m_EncodePipelines.end();
    for ( ; PipelineIt != PipelineEnd; ++PipelineIt )
    {
        if ( EncodePipeline == PipelineIt->GetPtr() )
        {
            Result = EncodePipeline->SetOsdConfig( OsdParameter, OsdParameterLength );
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetOsdConfig, EncodePipeline::SetOsdConfig failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetOsdConfig end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetOsdConfig, does not find device, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) GMI_INVALID_PARAMETER );
    return GMI_INVALID_PARAMETER;
}

GMI_RESULT MediaCenter::ForceGenerateIdrFrame( FD_HANDLE CodecHandle )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::ForceGenerateIdrFrame begin, CodecHandle=%p \n", CodecHandle );
    if ( NULL == CodecHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ForceGenerateIdrFrame, codec device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FAIL;
    MediaEncodePipeline *EncodePipeline = reinterpret_cast<MediaEncodePipeline *> (CodecHandle);
    std::vector< SafePtr<MediaEncodePipeline> >::iterator PipelineIt = m_EncodePipelines.begin(), PipelineEnd = m_EncodePipelines.end();
    for ( ; PipelineIt != PipelineEnd; ++PipelineIt )
    {
        if ( EncodePipeline == PipelineIt->GetPtr() )
        {
            Result = EncodePipeline->ForceGenerateIdrFrame();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ForceGenerateIdrFrame, EncodePipeline::ForceGenerateIdrFrame failed, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
                return Result;
            }
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::ForceGenerateIdrFrame end, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) Result );
            return Result;
        }
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ForceGenerateIdrFrame, does not find device, CodecHandle=%p, function return %x \n", CodecHandle, (uint32_t) GMI_INVALID_PARAMETER );
    return GMI_INVALID_PARAMETER;
}

GMI_RESULT MediaCenter::ReleaseCodecResource()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenter::ReleaseCodecResource begin \n" );

    GMI_RESULT Result = GMI_FAIL;
    std::vector< SafePtr<MediaEncodePipeline> >::iterator EncodePipelineIt = m_EncodePipelines.begin(), EncodePipelineEnd = m_EncodePipelines.end();
    for ( ; EncodePipelineIt != EncodePipelineEnd; ++EncodePipelineIt )
    {

        Result = (*EncodePipelineIt)->Stop();
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Warning, "MediaCenter::ReleaseCodecResource, EncodePipeline::Stop failed, CodecHandle=%p, function return %x \n", EncodePipelineIt->GetPtr(), (uint32_t) Result );
        }
        else
        {
            Result = (*EncodePipelineIt)->Deinitialize();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Warning, "MediaCenter::ReleaseCodecResource, EncodePipeline::Deinitialize failed, CodecHandle=%p, function return %x \n", EncodePipelineIt->GetPtr(), (uint32_t) Result );
            }
        }
    }
    m_EncodePipelines.clear();

    std::vector< SafePtr<MediaDecodePipeline> >::iterator DecodePipelineIt = m_DecodePipelines.begin(), DecodePipelineEnd = m_DecodePipelines.end();
    for ( ; DecodePipelineIt != DecodePipelineEnd; ++DecodePipelineIt )
    {
        Result = (*DecodePipelineIt)->Stop();
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Warning, "MediaCenter::ReleaseCodecResource, DecodePipeline::Stop failed, CodecHandle=%p, function return %x \n", DecodePipelineIt->GetPtr(), (uint32_t) Result );
        }
        else
        {
            Result = (*DecodePipelineIt)->Deinitialize();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Warning, "MediaCenter::ReleaseCodecResource, DecodePipeline::Deinitialize failed, CodecHandle=%p, function return %x \n", DecodePipelineIt->GetPtr(), (uint32_t) Result );
            }
        }
    }
    m_DecodePipelines.clear();

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "MediaCenter::ReleaseCodecResource end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::OpenImageDevice( uint16_t SensorId, uint16_t ChannelId, FD_HANDLE *ImageHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::OpenImageDevice begin, passed SensorId=%d, ChannelId=%d \n", SensorId, ChannelId );

    *ImageHandle = GMI_ImageOperateCreate( SensorId, ChannelId );
    if ( NULL == *ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::OpenImageDevice, image device is not opened, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::OpenImageDevice end, function return %x \n", GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::CloseImageDevice( FD_HANDLE& ImageHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CloseImageDevice begin, ImageHandle=%p \n", ImageHandle );

    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::CloseImageDevice, image device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_ImageOperateDestroy( ImageHandle );
    ImageHandle = NULL;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CloseImageDevice end, function return %x \n", GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::GetBaseImageConfig( FD_HANDLE ImageHandle, void_t *ImageParameter, size_t *ImageParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig begin, passed ImageParameterLength=%d \n", *ImageParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetBaseImageConfig, image device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(ImageBaseParam) > *ImageParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetBaseImageConfig, provided buffer space(%d) is not enough, actually need %d bytes, function return %x \n", *ImageParameterLength, sizeof(ImageBaseParam), (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    GMI_RESULT Result = GMI_ImageGetBaseConfig( ImageHandle, (ImageBaseParam*)ImageParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetBaseImageConfig, GMI_ImageGetBaseConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig, ExposureMode=%d \n",     ((ImageBaseParam*)ImageParameter)->s_ExposureMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig, ExposureValueMin=%d \n", ((ImageBaseParam*)ImageParameter)->s_ExposureValueMin );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig, ExposureValueMax=%d \n", ((ImageBaseParam*)ImageParameter)->s_ExposureValueMax );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig, GainMax=%d \n",          ((ImageBaseParam*)ImageParameter)->s_GainMax );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig, Brightness=%d \n",       ((ImageBaseParam*)ImageParameter)->s_Brightness );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig, Contrast=%d \n",         ((ImageBaseParam*)ImageParameter)->s_Contrast );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig, Saturation=%d \n",       ((ImageBaseParam*)ImageParameter)->s_Saturation );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig, Hue=%d \n",              ((ImageBaseParam*)ImageParameter)->s_Hue );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig, Sharpness=%d \n",        ((ImageBaseParam*)ImageParameter)->s_Sharpness );
#endif

    *ImageParameterLength = sizeof(ImageBaseParam);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetBaseImageConfig end and return %x, returned ImageParameterLength=%d \n", (uint32_t) GMI_SUCCESS, *ImageParameterLength );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::SetBaseImageConfig( FD_HANDLE ImageHandle, const void_t *ImageParameter, size_t ImageParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig begin, passed ImageParameterLength=%d \n", ImageParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetBaseImageConfig, image device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(ImageBaseParam) > ImageParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetBaseImageConfig, provided parameter size(%d) is too small, actually need %d bytes, function return %x \n", ImageParameterLength, sizeof(ImageBaseParam), (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig, ExposureMode=%d \n",     ((ImageBaseParam*)ImageParameter)->s_ExposureMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig, ExposureValueMin=%d \n", ((ImageBaseParam*)ImageParameter)->s_ExposureValueMin );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig, ExposureValueMax=%d \n", ((ImageBaseParam*)ImageParameter)->s_ExposureValueMax );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig, GainMax=%d \n",          ((ImageBaseParam*)ImageParameter)->s_GainMax );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig, Brightness=%d \n",       ((ImageBaseParam*)ImageParameter)->s_Brightness );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig, Contrast=%d \n",         ((ImageBaseParam*)ImageParameter)->s_Contrast );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig, Saturation=%d \n",       ((ImageBaseParam*)ImageParameter)->s_Saturation );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig, Hue=%d \n",              ((ImageBaseParam*)ImageParameter)->s_Hue );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig, Sharpness=%d \n",        ((ImageBaseParam*)ImageParameter)->s_Sharpness );
#endif

    GMI_RESULT Result = GMI_ImageSetBaseConfig( ImageHandle, (ImageBaseParam*)ImageParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetBaseImageConfig, GMI_ImageSetBaseConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetBaseImageConfig end and return %x \n", (uint32_t) GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::GetAdvancedImageConfig( FD_HANDLE ImageHandle, void_t *ImageParameter, size_t *ImageParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAdvancedImageConfig begin, passed ImageParameterLength=%d \n", *ImageParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetAdvancedImageConfig, image device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(ImageAdvanceParam) > *ImageParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetAdvancedImageConfig, provided buffer space(%d) is not enough, actually need %d bytes, function return %x \n", *ImageParameterLength, sizeof(ImageBaseParam), (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    GMI_RESULT Result = GMI_ImageGetAdvanceConfig( ImageHandle, (ImageAdvanceParam*)ImageParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetAdvancedImageConfig, GMI_ImageGetAdvanceConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAdvancedImageConfig, MeteringMode=%d \n",      ((ImageAdvanceParam*)ImageParameter)->s_MeteringMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAdvancedImageConfig, BackLightCompFlag=%d \n", ((ImageAdvanceParam*)ImageParameter)->s_BackLightCompFlag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAdvancedImageConfig, DcIrisFlag=%d \n",        ((ImageAdvanceParam*)ImageParameter)->s_DcIrisFlag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAdvancedImageConfig, LocalExposure=%d \n",     ((ImageAdvanceParam*)ImageParameter)->s_LocalExposure );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAdvancedImageConfig, MctfStrength=%d \n",      ((ImageAdvanceParam*)ImageParameter)->s_MctfStrength );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAdvancedImageConfig, DcIrisDuty=%d \n",        ((ImageAdvanceParam*)ImageParameter)->s_DcIrisDuty );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAdvancedImageConfig, AeTargetRatio=%d \n",     ((ImageAdvanceParam*)ImageParameter)->s_AeTargetRatio );
#endif

    *ImageParameterLength = sizeof(ImageAdvanceParam);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAdvancedImageConfig end and return %x, returned ImageParameterLength=%d \n", (uint32_t) GMI_SUCCESS, *ImageParameterLength );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::SetAdvancedImageConfig( FD_HANDLE ImageHandle, const void_t *ImageParameter, size_t ImageParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAdvancedImageConfig begin, passed ImageParameterLength=%d \n", ImageParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetAdvancedImageConfig, image device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(ImageAdvanceParam) > ImageParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetAdvancedImageConfig, provided parameter size(%d) is too small, actually need %d bytes, function return %x \n", ImageParameterLength, sizeof(ImageBaseParam), (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAdvancedImageConfig, MeteringMode=%d \n",      ((ImageAdvanceParam*)ImageParameter)->s_MeteringMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAdvancedImageConfig, BackLightCompFlag=%d \n", ((ImageAdvanceParam*)ImageParameter)->s_BackLightCompFlag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAdvancedImageConfig, DcIrisFlag=%d \n",        ((ImageAdvanceParam*)ImageParameter)->s_DcIrisFlag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAdvancedImageConfig, LocalExposure=%d \n",     ((ImageAdvanceParam*)ImageParameter)->s_LocalExposure );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAdvancedImageConfig, MctfStrength=%d \n",      ((ImageAdvanceParam*)ImageParameter)->s_MctfStrength );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAdvancedImageConfig, DcIrisDuty=%d \n",        ((ImageAdvanceParam*)ImageParameter)->s_DcIrisDuty );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAdvancedImageConfig, AeTargetRatio=%d \n",     ((ImageAdvanceParam*)ImageParameter)->s_AeTargetRatio );
#endif

    GMI_RESULT Result = GMI_ImageSetAdvanceConfig( ImageHandle, (ImageAdvanceParam*)ImageParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetAdvancedImageConfig, GMI_ImageSetAdvanceConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAdvancedImageConfig end and return %x \n", (uint32_t) GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::GetAutoFocusConfig( FD_HANDLE ImageHandle, void_t *AutoFocusParameter, size_t *AutoFocusParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAutoFocusConfig begin, passed AutoFocusParameterLength=%d \n", *AutoFocusParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetAutoFocusConfig, image device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(ImageAfParam) > *AutoFocusParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetAdvancedImageConfig, provided buffer space(%d) is not enough, actually need %d bytes, function return %x \n", *AutoFocusParameterLength, sizeof(ImageAfParam), (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    GMI_RESULT Result = GMI_ImageGetAfConfig( ImageHandle, (ImageAfParam*)AutoFocusParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetAdvancedImageConfig, GMI_ImageGetAfConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAutoFocusConfig, AfFlag=%d \n",   ((ImageAfParam*)AutoFocusParameter)->s_AfFlag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAutoFocusConfig, AfMode=%d \n",   ((ImageAfParam*)AutoFocusParameter)->s_AfMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAutoFocusConfig, LensType=%d \n", ((ImageAfParam*)AutoFocusParameter)->s_LensType );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAutoFocusConfig, ZmDist=%d \n",   ((ImageAfParam*)AutoFocusParameter)->s_ZmDist );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAutoFocusConfig, FsDist=%d \n",   ((ImageAfParam*)AutoFocusParameter)->s_FsDist );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAutoFocusConfig, FsNear=%d \n",   ((ImageAfParam*)AutoFocusParameter)->s_FsNear );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAutoFocusConfig, FsFar=%d \n",    ((ImageAfParam*)AutoFocusParameter)->s_FsFar );
#endif

    *AutoFocusParameterLength = sizeof(ImageAfParam);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetAutoFocusConfig end and return %x, returned AutoFocusParameterLength=%d \n", (uint32_t) GMI_SUCCESS, *AutoFocusParameterLength );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::SetAutoFocusConfig( FD_HANDLE ImageHandle, const void_t *AutoFocusParameter, size_t AutoFocusParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusConfig begin, passed AutoFocusParameterLength=%d \n", AutoFocusParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetAutoFocusConfig, image device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(ImageAfParam) > AutoFocusParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetAutoFocusConfig, provided parameter size(%d) is too small, actually need %d bytes, function return %x \n", AutoFocusParameterLength, sizeof(ImageAfParam), (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusConfig, AfFlag=%d \n",   ((ImageAfParam*)AutoFocusParameter)->s_AfFlag );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusConfig, AfMode=%d \n",   ((ImageAfParam*)AutoFocusParameter)->s_AfMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusConfig, LensType=%d \n", ((ImageAfParam*)AutoFocusParameter)->s_LensType );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusConfig, ZmDist=%d \n",   ((ImageAfParam*)AutoFocusParameter)->s_ZmDist );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusConfig, FsDist=%d \n",   ((ImageAfParam*)AutoFocusParameter)->s_FsDist );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusConfig, FsNear=%d \n",   ((ImageAfParam*)AutoFocusParameter)->s_FsNear );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusConfig, FsFar=%d \n",    ((ImageAfParam*)AutoFocusParameter)->s_FsFar );
#endif

    GMI_RESULT Result = GMI_ImageSetAfConfig( ImageHandle, (ImageAfParam*)AutoFocusParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetAutoFocusConfig, GMI_ImageSetAfConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusConfig end and return %x \n", (uint32_t) GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::GetDaynightConfig( FD_HANDLE ImageHandle, void_t *DaynightParameter, size_t *DaynightParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetDaynightConfig begin, passed DaynightParameterLength=%d \n", *DaynightParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetDaynightConfig, image device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(ImageDnParam) > *DaynightParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetDaynightConfig, provided buffer space(%d) is not enough, actually need %d bytes, function return %x \n", *DaynightParameterLength, sizeof(ImageDnParam), (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    GMI_RESULT Result = GMI_ImageGetDnConfig( ImageHandle, (ImageDnParam*)DaynightParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetDaynightConfig, GMI_ImageGetDnConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetDaynightConfig, DnMode=%d \n",        ((ImageDnParam*)DaynightParameter)->s_DnMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetDaynightConfig, DnDurtime=%d \n",     ((ImageDnParam*)DaynightParameter)->s_DnDurtime );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetDaynightConfig, NightToDayThr=%d \n", ((ImageDnParam*)DaynightParameter)->s_NightToDayThr );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetDaynightConfig, DayToNightThr=%d \n", ((ImageDnParam*)DaynightParameter)->s_DayToNightThr );

    for ( int32_t i = 0; i < SCHEDULE_WEEK_DAYS; ++i )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetDaynightConfig, DnSchedule.DnSchedFlag[%d]=%d \n", ((ImageDnParam*)DaynightParameter)->s_DnSchedule.s_DnSchedFlag[i] );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetDaynightConfig, DnSchedule.StartTime[%d]=%d \n",   ((ImageDnParam*)DaynightParameter)->s_DnSchedule.s_StartTime[i] );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetDaynightConfig, DnSchedule.EndTime[%d]=%d \n",     ((ImageDnParam*)DaynightParameter)->s_DnSchedule.s_EndTime[i] );
    }
#endif

    *DaynightParameterLength = sizeof(ImageDnParam);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetDaynightConfig end and return %x, returned DaynightParameterLength=%d \n", (uint32_t) GMI_SUCCESS, *DaynightParameterLength );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::SetDaynightConfig( FD_HANDLE ImageHandle, const void_t *DaynightParameter, size_t DaynightParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetDaynightConfig begin, passed DaynightParameterLength=%d \n", DaynightParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetDaynightConfig, image device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(ImageDnParam) > DaynightParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetDaynightConfig, provided parameter size(%d) is too small, actually need %d bytes, function return %x \n", DaynightParameterLength, sizeof(ImageDnParam), (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetDaynightConfig, DnMode=%d \n",        ((ImageDnParam*)DaynightParameter)->s_DnMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetDaynightConfig, DnDurtime=%d \n",     ((ImageDnParam*)DaynightParameter)->s_DnDurtime );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetDaynightConfig, NightToDayThr=%d \n", ((ImageDnParam*)DaynightParameter)->s_NightToDayThr );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetDaynightConfig, DayToNightThr=%d \n", ((ImageDnParam*)DaynightParameter)->s_DayToNightThr );

    for ( int32_t i = 0; i < SCHEDULE_WEEK_DAYS; ++i )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetDaynightConfig, DnSchedule.DnSchedFlag[%d]=%d \n", ((ImageDnParam*)DaynightParameter)->s_DnSchedule.s_DnSchedFlag[i] );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetDaynightConfig, DnSchedule.StartTime[%d]=%d \n",   ((ImageDnParam*)DaynightParameter)->s_DnSchedule.s_StartTime[i] );
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetDaynightConfig, DnSchedule.EndTime[%d]=%d \n",     ((ImageDnParam*)DaynightParameter)->s_DnSchedule.s_EndTime[i] );
    }
#endif

    GMI_RESULT Result = GMI_ImageSetDnConfig( ImageHandle, (ImageDnParam*)DaynightParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetDaynightConfig, GMI_ImageSetDnConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetDaynightConfig end and return %x \n", (uint32_t) GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::GetWhiteBalanceConfig( FD_HANDLE ImageHandle, void_t *WhiteBalanceParameter, size_t *WhiteBalanceParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetWhiteBalanceConfig begin, passed WhiteBalanceParameterLength=%d \n", *WhiteBalanceParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetWhiteBalanceConfig, image device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(ImageWbParam) > *WhiteBalanceParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetWhiteBalanceConfig, provided buffer space(%d) is not enough, actually need %d bytes, function return %x \n", *WhiteBalanceParameterLength, sizeof(ImageWbParam), (uint32_t) GMI_NOT_ENOUGH_SPACE );
        return GMI_NOT_ENOUGH_SPACE;
    }

    GMI_RESULT Result = GMI_ImageGetWbConfig( ImageHandle, (ImageWbParam*)WhiteBalanceParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetWhiteBalanceConfig, GMI_ImageGetWbConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetWhiteBalanceConfig, WbMode=%d \n",  ((ImageWbParam*)WhiteBalanceParameter)->s_WbMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetWhiteBalanceConfig, WbRgain=%d \n", ((ImageWbParam*)WhiteBalanceParameter)->s_WbRgain );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetWhiteBalanceConfig, WbBgain=%d \n", ((ImageWbParam*)WhiteBalanceParameter)->s_WbBgain );
#endif

    *WhiteBalanceParameterLength = sizeof(ImageWbParam);
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetWhiteBalanceConfig end and return %x, returned WhiteBalanceParameterLength=%d \n", (uint32_t) GMI_SUCCESS, *WhiteBalanceParameterLength );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::SetWhiteBalanceConfig( FD_HANDLE ImageHandle, const void_t *WhiteBalanceParameter, size_t WhiteBalanceParameterLength )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer image to MediaCenter, 2013/07/22

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetWhiteBalanceConfig begin, passed WhiteBalanceParameterLength=%d \n", WhiteBalanceParameterLength );
    if ( NULL == ImageHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetWhiteBalanceConfig, image device is not opened, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    if ( sizeof(ImageWbParam) > WhiteBalanceParameterLength )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetWhiteBalanceConfig, provided parameter size(%d) is too small, actually need %d bytes, function return %x \n", WhiteBalanceParameterLength, sizeof(ImageWbParam), (uint32_t) GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

#if MEDIA_CENTER_SUPPORT_DETAIL_LOG
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetWhiteBalanceConfig, WbMode=%d \n",  ((ImageWbParam*)WhiteBalanceParameter)->s_WbMode );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetWhiteBalanceConfig, WbRgain=%d \n", ((ImageWbParam*)WhiteBalanceParameter)->s_WbRgain );
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetWhiteBalanceConfig, WbBgain=%d \n", ((ImageWbParam*)WhiteBalanceParameter)->s_WbBgain );
#endif

    GMI_RESULT Result = GMI_ImageSetWbConfig( ImageHandle, (ImageWbParam*)WhiteBalanceParameter );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetWhiteBalanceConfig, GMI_ImageSetWbConfig failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetWhiteBalanceConfig end and return %x \n", (uint32_t) GMI_SUCCESS );
    return GMI_SUCCESS;
}

GMI_RESULT MediaCenter::OpenAutoFocusDevice( uint16_t SensorId, FD_HANDLE *AutoFocusHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::OpenAutoFocusDevice begin, passed SensorId=%d \n", SensorId );
    *AutoFocusHandle = GMI_AutoFocusCreate( SensorId );
    if ( NULL == *AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::OpenAutoFocusDevice, auto focus device opening failed, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }
    GMI_RESULT Result = GMI_SUCCESS;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::OpenAutoFocusDevice end, AutoFocusHandle=%p, function return %x \n", *AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::CloseAutoFocusDevice( FD_HANDLE& AutoFocusHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CloseAutoFocusDevice begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::CloseAutoFocusDevice, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }
    GMI_AutoFocusDestroy( AutoFocusHandle );
    AutoFocusHandle = NULL;
    GMI_RESULT Result = GMI_SUCCESS;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CloseAutoFocusDevice end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::StartAutoFocus( FD_HANDLE AutoFocusHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::StartAutoFocus begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StartAutoFocus, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_AutoFocusStart( AutoFocusHandle );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StartAutoFocus end, GMI_AutoFocusStart return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::StartAutoFocus end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::StopAutoFocus( FD_HANDLE AutoFocusHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::StopAutoFocus begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StopAutoFocus, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_AutoFocusStop( AutoFocusHandle );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::StopAutoFocus end, GMI_AutoFocusStop return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::StopAutoFocus end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::PauseAutoFocus( FD_HANDLE AutoFocusHandle, int8_t ControlStatus )
{
#if MONITOR_ZOOM_OPERATION_TIME
    struct timeval Time;
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::PauseAutoFocus begin, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif

    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::PauseAutoFocus begin, passed AutoFocusHandle=%p, ControlStatus=%d \n", AutoFocusHandle, ControlStatus );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::PauseAutoFocus, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_AutoFocusPause( AutoFocusHandle, ControlStatus );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::PauseAutoFocus end, GMI_AutoFocusPause return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::PauseAutoFocus end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
#if MONITOR_ZOOM_OPERATION_TIME
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::PauseAutoFocus end, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif
    return Result;
}

GMI_RESULT MediaCenter::AutoFocusGlobalScan( FD_HANDLE AutoFocusHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::AutoFocusGlobalScan begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::AutoFocusGlobalScan, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_AutoFocusGlobalScan( AutoFocusHandle );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::AutoFocusGlobalScan end, GMI_AutoFocusGlobalScan return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::AutoFocusGlobalScan end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::SetAutoFocusMode( FD_HANDLE AutoFocusHandle, int32_t AFMode )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusMode begin, passed AutoFocusHandle=%p, AFMode=%d \n", AutoFocusHandle, AFMode );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetAutoFocusMode, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_AutoFocusSetMode( AutoFocusHandle, AFMode );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetAutoFocusMode end, GMI_AutoFocusSetMode return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusMode end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::NotifyAutoFocus( FD_HANDLE	AutoFocusHandle, int32_t EventType, uint8_t *ExtData, uint32_t Length )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::NotifyAutoFocusDevice begin, passed AutoFocusHandle=%p, EventType=%d, Length=%d \n", AutoFocusHandle, EventType, Length );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::NotifyAutoFocusDevice, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_AFEventNotify( AutoFocusHandle, EventType, ExtData, Length );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::NotifyAutoFocusDevice end, GMI_AFEventNotify return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::NotifyAutoFocusDevice end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::GetFocusPosition( FD_HANDLE AutoFocusHandle, int32_t *FocusPos )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetFocusPosition begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetFocusPosition, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FocusPositionGet( AutoFocusHandle, FocusPos );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenterProxy::GetFocusPosition, GMI_FocusPositionGet return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetFocusPosition end, AutoFocusHandle=%p, FocusPos=%d, function return %x \n", AutoFocusHandle, *FocusPos, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::SetFocusPosition( FD_HANDLE AutoFocusHandle, int32_t FocusPos )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetFocusPosition begin, passed AutoFocusHandle=%p, FocusPos=%d \n", AutoFocusHandle, FocusPos );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetFocusPosition, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FocusPositionSet( AutoFocusHandle, FocusPos );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetFocusPosition, GMI_FocusPositionSet return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetFocusPosition end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::GetFocusRange( FD_HANDLE AutoFocusHandle, int32_t *MinFocusPos, int32_t *MaxFocusPos )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetFocusRange begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetFocusRange, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FocusRangeGet( AutoFocusHandle, MinFocusPos, MaxFocusPos );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetFocusRange, GMI_FocusRangeGet return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetFocusRange end, AutoFocusHandle=%p, MinFocusPos=%d, MaxFocusPos=%d, function return %x \n", AutoFocusHandle, *MinFocusPos, *MaxFocusPos, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::ResetFocusMotor( FD_HANDLE AutoFocusHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::ResetFocusMotor begin, passed AutoFocusHandle=%p \n", AutoFocusHandle );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ResetFocusMotor, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_FocusMotorReset( AutoFocusHandle );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ResetFocusMotor, GMI_FocusMotorReset return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetFocusRange end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::ControlAutoFocus( FD_HANDLE AutoFocusHandle, int8_t AfDirMode )
{
    // implemented in 2013/08/30

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::ControlAutoFocus begin, passed AutoFocusHandle=%p, AfDirMode=%d \n", AutoFocusHandle, AfDirMode );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ControlAutoFocus, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_AfCtrol( AutoFocusHandle, AfDirMode );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ControlAutoFocus, GMI_AfCtrol return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::ControlAutoFocus end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::SetAutoFocusStep( FD_HANDLE AutoFocusHandle, int32_t AfStep )
{
    // implemented in 2013/08/30

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusStep begin, passed AutoFocusHandle=%p, AfStep=%d \n", AutoFocusHandle, AfStep );
    if ( NULL == AutoFocusHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetAutoFocusStep, auto focus device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_AfStepSet( AutoFocusHandle, AfStep );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetAutoFocusStep, GMI_AfCtrol return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetAutoFocusStep end, AutoFocusHandle=%p, function return %x \n", AutoFocusHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::OpenZoomDevice( uint16_t SensorId, FD_HANDLE *ZoomHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::OpenZoomDevice begin, passed SensorId=%d \n", SensorId );

    *ZoomHandle = GMI_ZoomCreate( SensorId );
    if ( NULL == *ZoomHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::OpenZoomDevice, zoom device opening failed, function return %x \n", (uint32_t) GMI_OPEN_DEVICE_FAIL );
        return GMI_OPEN_DEVICE_FAIL;
    }
    GMI_RESULT Result = GMI_SUCCESS;

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::OpenZoomDevice end, ZoomHandle=%p, function return %x \n", *ZoomHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::CloseZoomDevice( FD_HANDLE& ZoomHandle )
{
    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CloseZoomDevice begin, passed ZoomHandle=%p \n", ZoomHandle );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::CloseZoomDevice, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_ZoomDestroy( ZoomHandle );
    ZoomHandle = NULL;
    GMI_RESULT Result = GMI_SUCCESS;
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::CloseZoomDevice end, ZoomHandle=%p, function return %x \n", ZoomHandle, (uint32_t) Result );
    return Result;
}

GMI_RESULT MediaCenter::GetZoomPosition( FD_HANDLE ZoomHandle, int32_t *ZoomPos )
{
#if MONITOR_ZOOM_OPERATION_TIME
    struct timeval Time;
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::GetZoomPosition begin, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif

    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetZoomPosition begin, passed ZoomHandle=%p \n", ZoomHandle );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetZoomPosition, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_ZoomPositionGet( ZoomHandle, ZoomPos );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetZoomPosition, GMI_ZoomPositionGet return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetZoomPosition end, ZoomHandle=%p, ZoomPos=%d, function return %x \n", ZoomHandle, *ZoomPos, (uint32_t) Result );
#if MONITOR_ZOOM_OPERATION_TIME
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::GetZoomPosition end, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif
    return Result;
}

GMI_RESULT MediaCenter::SetZoomPosition( FD_HANDLE ZoomHandle, int32_t ZoomPos )
{
#if MONITOR_ZOOM_OPERATION_TIME
    struct timeval Time;
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::SetZoomPosition begin, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif

    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetZoomPosition begin, passed ZoomHandle=%p, ZoomPos=%d \n", ZoomHandle, ZoomPos );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetZoomPosition, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_ZoomPositionSet( ZoomHandle, ZoomPos );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetZoomPosition, GMI_ZoomPositionSet return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetZoomPosition end, ZoomHandle=%p, ZoomPos=%d, function return %x \n", ZoomHandle, ZoomPos, (uint32_t) Result );
#if MONITOR_ZOOM_OPERATION_TIME
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::SetZoomPosition end, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif
    return Result;
}

GMI_RESULT MediaCenter::GetZoomRange( FD_HANDLE ZoomHandle, int32_t *MinZoomPos, int32_t *MaxZoomPos )
{
#if MONITOR_ZOOM_OPERATION_TIME
    struct timeval Time;
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::GetZoomRange begin, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif

    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetZoomRange begin, passed ZoomHandle=%p \n", ZoomHandle );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetZoomRange, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_ZoomRangeGet( ZoomHandle, MinZoomPos, MaxZoomPos );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::GetZoomRange, GMI_ZoomRangeGet return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::GetZoomRange end, ZoomHandle=%p, MinZoomPos=%d, MaxZoomPos=%d, function return %x \n", ZoomHandle, *MinZoomPos, *MaxZoomPos, (uint32_t) Result );
#if MONITOR_ZOOM_OPERATION_TIME
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::GetZoomRange end, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif
    return Result;
}

GMI_RESULT MediaCenter::ResetZoomMotor( FD_HANDLE ZoomHandle )
{
#if MONITOR_ZOOM_OPERATION_TIME
    struct timeval Time;
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::ResetZoomMotor begin, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif

    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::ResetZoomMotor begin, passed ZoomHandle=%p \n", ZoomHandle );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ResetZoomMotor, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_ZoomMotorReset( ZoomHandle );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ResetZoomMotor, GMI_ZoomMotorReset return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::ResetZoomMotor end, ZoomHandle=%p, function return %x \n", ZoomHandle, (uint32_t) Result );
#if MONITOR_ZOOM_OPERATION_TIME
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::ResetZoomMotor end, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif
    return Result;
}

GMI_RESULT MediaCenter::ControlZoom( FD_HANDLE ZoomHandle, int8_t ZoomMode )
{
#if MONITOR_ZOOM_OPERATION_TIME
    struct timeval Time;
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::ControlZoom begin, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif

    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::ControlZoom begin, passed ZoomHandle=%p, ZoomMode=%d \n", ZoomHandle, ZoomMode );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ControlZoom, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_ZoomCtrol( ZoomHandle, ZoomMode );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::ControlZoom, GMI_ZoomCtrol return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::ControlZoom end, ZoomHandle=%p, function return %x \n", ZoomHandle, (uint32_t) Result );
#if MONITOR_ZOOM_OPERATION_TIME
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::ControlZoom end, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif
    return Result;
}

GMI_RESULT MediaCenter::SetZoomStep( FD_HANDLE ZoomHandle, int32_t ZoomStep )
{
#if MONITOR_ZOOM_OPERATION_TIME
    struct timeval Time;
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::SetZoomStep begin, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif

    // now implement in MediaCenterProxy, 2013/07/08
    // transfer auto focus to MediaCenter, 2013/08/12

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetZoomStep begin, passed ZoomHandle=%p, ZoomStep=%d \n", ZoomHandle, ZoomStep );
    if ( NULL == ZoomHandle )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetZoomStep, zoom device is not opened or already closed, function return %x \n", (uint32_t) GMI_DEVICE_NOT_OPENED );
        return GMI_DEVICE_NOT_OPENED;
    }

    GMI_RESULT Result = GMI_ZoomStepSet( ZoomHandle, ZoomStep );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "MediaCenter::SetZoomStep, GMI_ZoomStepSet return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "MediaCenter::SetZoomStep end, ZoomHandle=%p, function return %x \n", ZoomHandle, (uint32_t) Result );
#if MONITOR_ZOOM_OPERATION_TIME
    gettimeofday1( &Time, NULL );
    printf( "MediaCenter::SetZoomStep end, Time=%d_%.6d \n", (uint32_t) Time.tv_sec, (uint32_t) Time.tv_usec );
#endif
    return Result;
}
