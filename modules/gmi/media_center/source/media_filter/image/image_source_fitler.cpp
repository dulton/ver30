#include "image_source_fitler.h"

#include "gmi_brdwrapper.h"
#include "gmi_config_api.h"
#include "ipc_fw_v3.x_setting.h"
#include "share_memory_log_client.h"
#if defined( __linux__ )
#include "sys_info_readonly.h"
#endif

ImageSourceFitler::ImageSourceFitler(void)
    : m_FeedHardwareDog( false )
    , m_FeedDogInterval( 0 )
    , m_SourceId( 0 )
    , m_MediaId( 0 )
    , m_MediaType( 0 )
    , m_CodecType( 0 )
{
    m_LastFeedDogTime.tv_sec  = 0;
    m_LastFeedDogTime.tv_usec = 0;
}

ImageSourceFitler::~ImageSourceFitler(void)
{
}

GMI_RESULT	ImageSourceFitler::Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize )
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::Initialize begin \n" );

    GMI_RESULT Result = StreamingMediaSource::Initialize( FilterId, FilterName, FilterNameLength, Argument, ArgumentSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::Initialize, StreamingMediaSource::Initialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    uint32_t SourceId = 0xFF&(FilterId>>24);
    uint32_t MediaId = 0xFF&(FilterId>>16);
    uint32_t MediaType = 0xFF&(FilterId>>8);
    uint32_t CodecType = 0xFF&(FilterId>>0);
    if ( 0 == SourceId && 0 == MediaId && MEDIA_VIDEO == MediaType )
    {
        m_FeedHardwareDog = true;
        Result = GetHardwareDogEnableConfig( &m_FeedHardwareDog );
        if ( FAILED( Result ) )
        {
            StreamingMediaSource::Deinitialize();
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::Initialize, GetHardwareDogEnableConfig return %x \n", (uint32_t) Result );
            return Result;
        }
    }

    if ( m_FeedHardwareDog )
    {
        int32_t DogStatus = 0;
        Result = GMI_SysWdGuardIsOpened( &DogStatus );
        if ( FAILED( Result ) )
        {
            StreamingMediaSource::Deinitialize();
            DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::Initialize, GMI_SysWdGuardIsOpened failed, function return %x \n", (uint32_t) Result );
            return Result;
        }

        if ( !DogStatus )
        {
            uint32_t GuardTime = 0;
            Result = GetHardwareDogGuardTime( &GuardTime );
            if ( FAILED( Result ) )
            {
                StreamingMediaSource::Deinitialize();
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::Initialize, GetHardwareDogGuardTime failed, function return %x \n", (uint32_t) Result );
                return Result;
            }

            Result = GetFeedHardwareDogInterval( &m_FeedDogInterval );
            if ( FAILED( Result ) )
            {
                StreamingMediaSource::Deinitialize();
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::Initialize, GetFeedHardwareDogInterval failed, function return %x \n", (uint32_t) Result );
                return Result;
            }

            Result = GMI_SysOpenWdGuard( GuardTime );
            if ( FAILED( Result ) )
            {
                StreamingMediaSource::Deinitialize();
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::Initialize, GMI_SysOpenWdGuard failed, function return %x \n", (uint32_t) Result );
                return Result;
            }
        }

        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::Initialize, hardware dog is opened, GMI_SysWdGuardIsOpened return %x \n", (uint32_t) Result );
    }

    m_SourceId  = SourceId;
    m_MediaId   = MediaId;
    m_MediaType = MediaType;
    m_CodecType = CodecType;

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::Initialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT  ImageSourceFitler::Deinitialize()
{
    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::Deinitialize begin \n" );

    GMI_RESULT Result = StreamingMediaSource::Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::Deinitialize, StreamingMediaSource::Deinitialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    if ( m_FeedHardwareDog )
    {
        Result = GMI_SysCloseWdGuard();
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::Deinitialize, hardware dog is closed, GMI_SysCloseWdGuard return %x \n", (uint32_t) Result );
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::Deinitialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT  ImageSourceFitler::Receive( int32_t InputPinIndex, const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength )
{
    if ( m_FeedHardwareDog )
    {
        uint32_t ElapsedTime = FrameTS->tv_sec - m_LastFeedDogTime.tv_sec + (FrameTS->tv_usec - m_LastFeedDogTime.tv_usec)/1000000;
        if ( m_FeedDogInterval <= ElapsedTime )
        {
            m_LastFeedDogTime = *FrameTS;
            GMI_RESULT Result = GMI_SysNoticeWdGuard();
            if ( FAILED( Result ) )
            {
                DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::Receive, GMI_SysNoticeWdGuard, Result=%x, FrameTS=%ld:%ld \n", (uint32_t)Result, FrameTS->tv_sec, FrameTS->tv_usec );
            }
        }
    }
    return GMI_SUCCESS;
}

GMI_RESULT ImageSourceFitler::GetHardwareDogEnableConfig( boolean_t *Enable )
{
    int32_t DefaultEnable = GMI_HARDWARE_DOG_CONFIG_ENABLE_VALUE;
    int32_t IntEnable = GMI_HARDWARE_DOG_CONFIG_ENABLE_VALUE;

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::GetHardwareDogEnableConfig, GMI_XmlOpen failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::GetHardwareDogEnableConfig, DefaultEnable=%d \n", DefaultEnable );

    Result = GMI_XmlRead(Handle, GMI_HARDWARE_DOG_CONFIG_PATH, GMI_HARDWARE_DOG_CONFIG_ENABLE_KEY_NAME, DefaultEnable, &IntEnable, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::GetHardwareDogEnableConfig, GMI_XmlRead failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::GetHardwareDogEnableConfig, GMI_XmlFileSave failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::GetHardwareDogEnableConfig, DefaultEnable=%d, IntEnable=%d \n", DefaultEnable, IntEnable );
#endif

    // the following code facilitate unitest check execution, uncomment it to enable print info
    //printf( "ImageSourceFitler::GetHardwareDogEnableConfig, DefaultEnable=%d, IntEnable=%d \n", DefaultEnable, IntEnable );

    *Enable = (IntEnable == 0) ? false : true;
    return GMI_SUCCESS;
}

GMI_RESULT ImageSourceFitler::GetHardwareDogGuardTime( uint32_t *GuardTime )
{
    int32_t DefaultDogGuradTime = GMI_HARDWARE_DOG_CONFIG_GUARD_TIME_VALUE;
    int32_t IntDogGuradTime = GMI_HARDWARE_DOG_CONFIG_GUARD_TIME_VALUE;

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::GetHardwareDogGuardTime, GMI_XmlOpen failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::GetHardwareDogGuardTime, DefaultDogGuradTime=%d \n", DefaultDogGuradTime );

    Result = GMI_XmlRead(Handle, GMI_HARDWARE_DOG_CONFIG_PATH, GMI_HARDWARE_DOG_CONFIG_GUARD_TIME_KEY_NAME, DefaultDogGuradTime, &IntDogGuradTime, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::GetHardwareDogGuardTime, GMI_XmlRead failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::GetHardwareDogGuardTime, GMI_XmlFileSave failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::GetHardwareDogGuardTime, DefaultDogGuradTime=%d, IntDogGuradTime=%d \n", DefaultDogGuradTime, IntDogGuradTime );
#endif

    // the following code facilitate unitest check execution, uncomment it to enable print info
    //printf( "ImageSourceFitler::GetHardwareDogGuardTime, DefaultDogGuradTime=%d, IntDogGuradTime=%d \n", DefaultDogGuradTime, IntDogGuradTime );

    *GuardTime = (uint32_t) IntDogGuradTime;
    return GMI_SUCCESS;
}

GMI_RESULT ImageSourceFitler::GetFeedHardwareDogInterval( uint32_t *Interval )
{
    int32_t DefaultFeedDogInterval = GMI_HARDWARE_DOG_CONFIG_FEED_DOG_INTERVAL_VALUE;
    int32_t IntFeedDogInterval = GMI_HARDWARE_DOG_CONFIG_FEED_DOG_INTERVAL_VALUE;

#if defined( __linux__ )
    FD_HANDLE  Handle = NULL;
    GMI_RESULT Result = GMI_XmlOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::GetFeedHardwareDogInterval, GMI_XmlOpen failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::GetFeedHardwareDogInterval, DefaultFeedDogInterval=%d \n", DefaultFeedDogInterval );

    Result = GMI_XmlRead(Handle, GMI_HARDWARE_DOG_CONFIG_PATH, GMI_HARDWARE_DOG_CONFIG_FEED_DOG_INTERVAL_KEY_NAME, DefaultFeedDogInterval, &IntFeedDogInterval, GMI_CONFIG_READ_WRITE );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::GetFeedHardwareDogInterval, GMI_XmlRead failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Exception, "ImageSourceFitler::GetFeedHardwareDogInterval, GMI_XmlFileSave failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultShareMemoryLogClient, e_DebugLogLevel_Info, "ImageSourceFitler::GetFeedHardwareDogInterval, DefaultFeedDogInterval=%d, IntFeedDogInterval=%d \n", DefaultFeedDogInterval, IntFeedDogInterval );
#endif

    // the following code facilitate unitest check execution, uncomment it to enable print info
    //printf( "ImageSourceFitler::GetFeedHardwareDogInterval, DefaultFeedDogInterval=%d, IntFeedDogInterval=%d \n", DefaultFeedDogInterval, IntFeedDogInterval );

    *Interval = (uint32_t) IntFeedDogInterval;
    return GMI_SUCCESS;
}
