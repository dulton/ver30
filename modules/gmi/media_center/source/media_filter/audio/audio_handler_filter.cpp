#include "audio_handler_filter.h"

#include "log_client.h"
#include "media_codec_parameter.h"

AudioHandlerFilter::AudioHandlerFilter(void)
{
}

AudioHandlerFilter::~AudioHandlerFilter(void)
{
}

GMI_RESULT	AudioHandlerFilter::Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize )
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "AudioHandlerFilter::Initialize begin \n" );

    MediaSourceParameter *SourceParameter = reinterpret_cast<MediaSourceParameter *> (Argument);
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "AudioHandlerFilter::Initialize, SourceId=%d, MediaId=%d, MediaType=%d, CodecType=%d\n", SourceParameter->s_SourceId, SourceParameter->s_MediaId, SourceParameter->s_MediaType, SourceParameter->s_CodecType );
    if ( MEDIA_AUDIO != SourceParameter->s_MediaType )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "AudioHandlerFilter::Initialize, SourceParameter->MediaType=%d, not MEDIA_AUDIO(%d), function return %x \n", SourceParameter->s_MediaType, MEDIA_AUDIO, GMI_INVALID_PARAMETER );
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = StreamingMediaHandler::Initialize( FilterId, FilterName, FilterNameLength, Argument, ArgumentSize );
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "AudioHandlerFilter::Initialize, StreamingMediaHandler::Initialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    // input pin
    StreamingMediaInputPin *InputPin = StreamingMediaInputPin::CreateNew( false ,this, 0 );
    if ( NULL == InputPin )
    {
        StreamingMediaHandler::Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "AudioHandlerFilter::Initialize, StreamingMediaInputPin::CreateNew failed, function return %x \n", (uint32_t) GMI_OUT_OF_MEMORY );
        return GMI_OUT_OF_MEMORY;
    }

    uint32_t ApplicationMediaType = (SourceParameter->s_MediaType<<16)+SourceParameter->s_CodecType;
    uint32_t ApplicationMediaId = (SourceParameter->s_SourceId<<16)+SourceParameter->s_MediaId;

    Result = InputPin->Initialize( ApplicationMediaType, ApplicationMediaId );
    if ( FAILED( Result ) )
    {
        InputPin->ReleaseRef();
        StreamingMediaHandler::Deinitialize();
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "AudioHandlerFilter::Initialize, InputPin::Initialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    m_Inputs.push_back( InputPin );

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "AudioHandlerFilter::Initialize end \n" );
    return GMI_SUCCESS;
}

GMI_RESULT  AudioHandlerFilter::Deinitialize()
{
    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "AudioHandlerFilter::Deinitialize begin \n" );

    GMI_RESULT Result = StreamingMediaHandler::Deinitialize();
    if ( FAILED( Result ) )
    {
        DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Exception, "AudioHandlerFilter::Deinitialize, StreamingMediaHandler::Deinitialize failed, function return %x \n", (uint32_t) Result );
        return Result;
    }

    DEBUG_LOG( g_DefaultLogClient, e_DebugLogLevel_Info, "AudioHandlerFilter::Deinitialize end \n" );
    return GMI_SUCCESS;
}
