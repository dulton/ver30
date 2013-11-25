#include "video_source_filter.h"

#include "log_client.h"
#include "media_codec_parameter.h"

VideoSourceFilter::VideoSourceFilter(void)
    : m_Buffer( NULL )
    , m_BufferSize( 0 )
    , m_DataPool( NULL )
{
}

VideoSourceFilter::~VideoSourceFilter(void)
{
}

GMI_RESULT	VideoSourceFilter::Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "VideoSourceFilter::Initialize begin \n" );

    MediaSourceParameter *SourceParameter = reinterpret_cast<MediaSourceParameter *> (Argument);
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "VideoSourceFilter::Initialize, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d\n", SourceParameter->s_SourceId, SourceParameter->s_MediaId, SourceParameter->s_MediaType, SourceParameter->s_CodecType );
    if ( MEDIA_VIDEO != SourceParameter->s_MediaType )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "VideoSourceFilter::Initialize, SourceParameter->MediaType=%d, not MEDIA_VIDEO(%d), function return %x \n", SourceParameter->s_MediaType, MEDIA_VIDEO, GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = ImageSourceFitler::Initialize( FilterId, FilterName, FilterNameLength, Argument, ArgumentSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "VideoSourceFilter::Initialize, ImageSourceFitler::Initialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    size_t   MemorySize = 0;
    size_t   MaxDataBlockSize = 0;
    size_t   MinDataBlockSize = 0;

    MemorySize = (SourceParameter->s_BitRate*1024/8);//use 1024, instead of 1000, consider memory buffer overhead usage.

    MaxDataBlockSize = MemorySize;
    MinDataBlockSize = MemorySize/4;

    if ( MEDIA_FILTER_MEMORY_POOL_MINIMUM_MEMORY_BLOCK_NUMBER < SourceParameter->s_FrameRate )
    {
        MemorySize *= SourceParameter->s_FrameRate;
    }
    else
    {
        MemorySize *= MEDIA_FILTER_MEMORY_POOL_MINIMUM_MEMORY_BLOCK_NUMBER;
    }
    MemorySize /= 4;//250ms buffer

    uint32_t ApplicationMediaType = (SourceParameter->s_MediaType<<16)+SourceParameter->s_CodecType;
    uint32_t ApplicationMediaId = (SourceParameter->s_SourceId<<16)+SourceParameter->s_MediaId;

    // buffer
    m_Buffer = BaseMemoryManager::Instance().News<uint8_t>( MemorySize );
    if ( NULL == m_Buffer )
    {
        ImageSourceFitler::Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "VideoSourceFilter::Initialize, memory used by memory pool allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    // memory pool
    m_DataPool = BaseMemoryManager::Instance().New<MemoryPoolParallelConsumers>();
    if ( NULL == m_DataPool )
    {
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_Buffer );
        m_Buffer = NULL;
        ImageSourceFitler::Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "VideoSourceFilter::Initialize, memory pool object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_DataPool->Initialize( ApplicationMediaType, ApplicationMediaId, m_Buffer, MemorySize, MaxDataBlockSize, MinDataBlockSize );
    if ( FAILED( Result ) )
    {
        BaseMemoryManager::Instance().Delete( m_DataPool );
        m_DataPool = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_Buffer );
        m_Buffer = NULL;
        ImageSourceFitler::Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "VideoSourceFilter::Initialize, memory pool initialization failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    // output pin
    StreamingMediaOutputPin *OutputPin = StreamingMediaOutputPin::CreateNew( false ,this, 0 );
    if ( NULL == OutputPin )
    {
        m_DataPool->Deinitialize();
        BaseMemoryManager::Instance().Delete( m_DataPool );
        m_DataPool = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_Buffer );
        m_Buffer = NULL;
        ImageSourceFitler::Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "VideoSourceFilter::Initialize, output pin object allocation failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    Result = OutputPin->Initialize( ApplicationMediaType, ApplicationMediaId, m_DataPool );
    if ( FAILED( Result ) )
    {
        OutputPin->ReleaseRef();
        m_DataPool->Deinitialize();
        BaseMemoryManager::Instance().Delete( m_DataPool );
        m_DataPool = NULL;
        BaseMemoryManager::Instance().Deletes( (uint8_t *)m_Buffer );
        m_Buffer = NULL;
        ImageSourceFitler::Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "VideoSourceFilter::Initialize, output pin initialization failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    m_Outputs.push_back( OutputPin );

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "VideoSourceFilter::Initialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT  VideoSourceFilter::Deinitialize()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "VideoSourceFilter::Deinitialize begin \n" );

    GMI_RESULT Result = ImageSourceFitler::Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "VideoSourceFilter::Deinitialize, ImageSourceFitler::Deinitialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    if ( NULL != m_DataPool )
    {
        Result = m_DataPool->Deinitialize();
        if ( FAILED( Result ) )
        {
            DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "VideoSourceFilter::Deinitialize, memory pool deinitialize failed, function return %x \n", (uint32_t) Result );
            return Result;
        }
    }

    BaseMemoryManager::Instance().Delete( m_DataPool );
    m_DataPool = NULL;

    BaseMemoryManager::Instance().Deletes( (uint8_t *)m_Buffer );
    m_Buffer = NULL;

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "VideoSourceFilter::Deinitialize end \n" );
    return GMI_SUCCESS;
}
