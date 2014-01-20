// media_center_client_unitest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "gmi_system_headers.h"
#include "media_center_proxy.h"
#include "media_transport_proxy.h"
#include "timer_task_queue.h"

#include "ipc_media_data_client.h"
#include "ipc_media_data_server.h"

#define DEFAULT_MEDIA_CENTER_SERVER_IP   "192.168.0.247"
#define DEFAULT_MEDIA_CENTER_SERVER_PORT 2000
#define DEFAULT_MEDIA_CENTER_CLIENT_PORT 3000

#if defined( __linux__ )
void_t SIGPIPE_SignalProcess( int32_t SignalNumber )
{
    printf( "media_center_server: read side of pipe or socket is already close. SignalNumber=%d\n", SignalNumber );
}

void_t SignalProcess( int32_t SignalNumber )
{
    printf( "media_center_server: SignalNumber=%d\n", SignalNumber );
}
#endif

#define RTSP_SERVER_PORT  554
#define MAX_STREAM_NUMBER 2

#define VEDIO_ENCODE_MODE  1
#define AUDIO_ENCODE_MODE  2
#define AUDIO_DECODE_MODE  3
#define AUDIO_CODEC_MODE   4

class NetworkInitializer;

struct VideoInfo
{
    int32_t   VideoId;
    FD_HANDLE Encode;
    FD_HANDLE Transport;
};

GMI_RESULT TestVideoCodec( uint32_t RunTime );
GMI_RESULT TestAudioCodec( uint32_t RunTime, int32_t CodecMode );
GMI_RESULT TestAudioCodec2( uint32_t RunTime );

GMI_RESULT StartVideo( int32_t VideoId, VideoEncodeParam *EncParam, VideoInfo *Info );
GMI_RESULT StopVideo( VideoInfo *Info );
GMI_RESULT GetCodecConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE VideoEncodeHandle );
GMI_RESULT SetCodecConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE VideoEncodeHandle );
GMI_RESULT GetOsdConfiguration( MediaCenterProxy& MediaCenter,   FD_HANDLE VideoEncodeHandle );
GMI_RESULT SetOsdConfiguration( MediaCenterProxy& MediaCenter,   FD_HANDLE VideoEncodeHandle );
GMI_RESULT ForceGenerateIdrFrame( MediaCenterProxy& MediaCenter, FD_HANDLE VideoEncodeHandle );

GMI_RESULT TestVinVoutOperation( MediaCenterProxy& MediaCenter, FD_HANDLE VinVoutHandle );
GMI_RESULT GetVinVoutConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE VinVoutHandle );
GMI_RESULT SetVinVoutConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE VinVoutHandle );

GMI_RESULT TestImageOperation( MediaCenterProxy& MediaCenter, FD_HANDLE VinVoutHandle );
GMI_RESULT GetBaseImageConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle );
GMI_RESULT SetBaseImageConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle );
GMI_RESULT GetAdvancedImageConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle );
GMI_RESULT SetAdvancedImageConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle );
GMI_RESULT GetAutoFocusConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle );
GMI_RESULT SetAutoFocusConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle );
GMI_RESULT GetDaynightConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle );
GMI_RESULT SetDaynightConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle );
GMI_RESULT GetWhiteBalanceConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle );
GMI_RESULT SetWhiteBalanceConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle );

GMI_RESULT TestAutoFocusOperation( MediaCenterProxy& MediaCenter, FD_HANDLE AutoFocusHandle );
GMI_RESULT TestZoomOperation( MediaCenterProxy& MediaCenter, FD_HANDLE ZoomHandle );

GMI_RESULT StartCodecAudio( boolean_t EncodeMode, int32_t AudioId, void_t *CodecParam, FD_HANDLE *Handle );
GMI_RESULT StopCodecAudio( FD_HANDLE Handle );

MediaCenterProxy    g_MediaCenter;
MediaTransportProxy g_MediaTransport;
int32_t             g_ForceGenerateIdrFrame = 0;

void_t RTSPEventProcess( void_t *UserData, uint32_t EventType, ReferrencePtr<uint8_t,DefaultObjectsDeleter>& EventData, size_t EventDataLength )
{
    switch ( EventType )
    {
    case RTSP_ET_PLAY:
        if ( 0 != g_ForceGenerateIdrFrame )
        {
            g_MediaCenter.ForceGenerateIdrFrame( ((VideoInfo*) UserData)->Encode );
        }
        break;

    default:
        break;
    }

    printf( "RTSPEventProcess, EventType=%d, EventDataLength=%d \n", EventType, EventDataLength );
}

#if defined( _WIN32 )
int32_t _tmain( int32_t argc, _TCHAR* argv[] )
#elif defined( __linux__ )
int32_t main( int32_t argc, char_t* argv[] )
#endif
{
#if defined( __linux__ )
    signal( SIGPIPE , SIGPIPE_SignalProcess );
    //signal( SIGINT,  SignalProcess );
    //signal( SIGTERM, SignalProcess );
#endif


    const size_t ClientIPLength = 128;
    char_t   ClientIP[ClientIPLength];
    memset( ClientIP, 0, ClientIPLength );
    uint32_t ClientPort = 0;

    printf( "please input client IP, for example: %s\n", DEFAULT_MEDIA_CENTER_SERVER_IP );
#if defined( __linux__ )
    int32_t ScanfResult = scanf( "%s", ClientIP );
#elif defined( _WIN32 )
    int32_t ScanfResult = scanf_s( "%s", ClientIP, ClientIPLength );
#endif
    READ_MYSELF( ScanfResult );

    printf( "please input client udp port, for example: %d\n", DEFAULT_MEDIA_CENTER_CLIENT_PORT );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &ClientPort );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &ClientPort );
#endif

    const size_t ServerIPLength = ClientIPLength;
    char_t   ServerIP[ServerIPLength];
    memset( ServerIP, 0, ServerIPLength );
    uint32_t ServerPort = 0;

    printf( "please input server IP, for example: %s\n", DEFAULT_MEDIA_CENTER_SERVER_IP );
#if defined( __linux__ )
    ScanfResult = scanf( "%s", ServerIP );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%s", ServerIP, ServerIPLength );
#endif

    printf( "please input server udp port, for example: %d\n", DEFAULT_MEDIA_CENTER_SERVER_PORT );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &ServerPort );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &ServerPort );
#endif

    uint32_t RunTime = 60;
    printf( "please input run time(second unit), such as %d \n", RunTime );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &RunTime );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &RunTime );
#endif

    printf( "we will run %d seconds \n", RunTime );

    struct timeval StartTime;
    gettimeofday1( &StartTime, NULL );

    int32_t CodecMode = 1;
    printf( "please choose codec mode, 1:video encode, 2: audio encode, 3: audio decode, 4: audio codec \n" );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &CodecMode );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &CodecMode );
#endif

    if ( VEDIO_ENCODE_MODE != CodecMode && AUDIO_ENCODE_MODE != CodecMode && AUDIO_DECODE_MODE != CodecMode && AUDIO_CODEC_MODE != CodecMode )
    {
        printf( "input codec mode is wrong, codec mode =%d \n", CodecMode );
        return -1;
    }

    SafePtr<NetworkInitializer> Initializer( BaseMemoryManager::Instance().New<NetworkInitializer>() );
    if ( NULL == Initializer.GetPtr() )
    {
        return -1;
    }

    GMI_RESULT Result = GMI_FAIL;

    Result = g_MediaTransport.Initialize( RTSP_SERVER_PORT );
    if ( FAILED( Result ) )
    {
        printf( "failed to MediaTransport.Initialize, Result=%x \n", (int32_t)Result );
        return -1;
    }
    printf( "MediaTransport.Initialize, Result=%x \n", (int32_t)Result );

    Result = g_MediaCenter.Initialize( inet_addr(ClientIP), htons((uint16_t)ClientPort), UDP_SESSION_BUFFER_SIZE, inet_addr(ServerIP), htons((uint16_t)ServerPort) );
    if ( FAILED( Result ) )
    {
        g_MediaTransport.Deinitialize();
        printf( "failed to MediaCenter.Initialize, Result=%x \n", (int32_t)Result );
        return -1;
    }
    printf( "MediaCenter.Initialize, Result=%x \n", (int32_t)Result );

    if ( VEDIO_ENCODE_MODE == CodecMode )
    {
        TestVideoCodec( RunTime );
    }
    else if ( AUDIO_ENCODE_MODE == CodecMode || AUDIO_DECODE_MODE == CodecMode )
    {
        TestAudioCodec( RunTime, CodecMode );
    }
    else
    {
        TestAudioCodec2( RunTime );
    }

    struct timeval EndTime;
    gettimeofday1( &EndTime, NULL );

    int32_t duration = EndTime.tv_sec - StartTime.tv_sec +( EndTime.tv_usec - StartTime.tv_usec )/1000000;
    printf( "we already run %d seconds \n", duration );

    Result = g_MediaCenter.Deinitialize();
    if ( FAILED( Result ) )
    {
        g_MediaTransport.Deinitialize();
        printf( "failed to MediaCenter.Deinitialize, Result=%x \n", (int32_t)Result );
        return -1;
    }

    Result = g_MediaTransport.Deinitialize();
    if ( FAILED( Result ) )
    {
        printf( "failed to MediaTransport.Deinitialize, Result=%x \n", (int32_t)Result );
        return -1;
    }

    printf( "MediaTransport.Deinitialize, Result=%x \n", (int32_t)Result );

    return 0;
}

GMI_RESULT TestVideoCodec( uint32_t RunTime )
{
    int32_t TestMode = 0;
    printf( "please choose test mode, 0: single stream. 1: multiple stream.\n" );
#if defined( __linux__ )
    int32_t ScanfResult = scanf( "%d", &TestMode );
#elif defined( _WIN32 )
    int32_t ScanfResult = scanf_s( "%d", &TestMode );
#endif
    READ_MYSELF( ScanfResult );

    if ( 0 != TestMode && 1 != TestMode )
    {
        printf( "input parameter is wrong, test mode =%d \n", TestMode );
        return -1;
    }

    printf( "if or not force to generate IDR frame, 0: no. 1: yes.\n" );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &g_ForceGenerateIdrFrame );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &g_ForceGenerateIdrFrame );
#endif
    if ( 0 != g_ForceGenerateIdrFrame && 1 != g_ForceGenerateIdrFrame )
    {
        printf( "force to generate IDR frame parameter is wrong, ForceGenerateIdrFrame =%d \n", g_ForceGenerateIdrFrame );
        return -1;
    }

    VideoEncodeParam EncParam[MAX_STREAM_NUMBER];
    for ( int32_t i = 0; i < MAX_STREAM_NUMBER; ++i )
    {
        EncParam[i].s_EncodeType     = CODEC_H264;
        EncParam[i].s_FrameRate      = 25;
        EncParam[i].s_EncodeWidth    = 1920;
        EncParam[i].s_EncodeHeight   = 1080;
        EncParam[i].s_BitRateType    = BIT_CBR;
        EncParam[i].s_BitRateAverage = 4000;
        EncParam[i].s_BitRateUp      = 0;
        EncParam[i].s_BitRateDown    = 0;
        EncParam[i].s_FrameInterval  = 25;
        EncParam[i].s_EncodeQulity   = 100;
        EncParam[i].s_Rotate         = 0;

        if ( 0 == i )
        {
            EncParam[i].s_EncodeWidth    = 1920;
            EncParam[i].s_EncodeHeight   = 1080;
            EncParam[i].s_BitRateAverage = 4000;
        }
        else if ( 1 == i )
        {
            EncParam[i].s_EncodeWidth    = 720;
            EncParam[i].s_EncodeHeight   = 480;
            EncParam[i].s_BitRateAverage = 2000;
        }
        else if ( 2 == i )
        {
            EncParam[i].s_EncodeWidth    = 720;
            EncParam[i].s_EncodeHeight   = 480;
            EncParam[i].s_BitRateAverage = 1000;
        }
        else if ( 3 == i )
        {
            EncParam[i].s_EncodeWidth    = 720;
            EncParam[i].s_EncodeHeight   = 480;
            EncParam[i].s_BitRateAverage = 500;
        }
    }

    // Test Vin Vout
    FD_HANDLE VinVoutHandle = NULL;
    GMI_RESULT Result = g_MediaCenter.OpenVinVoutDevice( 0, 0, &VinVoutHandle );
    if ( FAILED( Result ) )
    {
        printf( "failed to MediaCenter.OpenVinVoutDevice, Result=%x \n", (int32_t)Result );
        return -1;
    }

    Result = TestVinVoutOperation( g_MediaCenter, VinVoutHandle );
    if ( FAILED( Result ) )
    {
        g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
        printf( "failed to TestVinVoutOperation, Result=%x \n", (int32_t)Result );
        return -1;
    }

    // Test Codec
    VideoInfo Info[MAX_STREAM_NUMBER];

    if ( 0 == TestMode )
    {
        int32_t StreamId = 0;
        printf( "please choose stream, 0: first stream. 1: second stream. amba must first start main stream(0), then start other stream \n" );
#if defined( __linux__ )
        ScanfResult = scanf( "%d", &StreamId );
#elif defined( _WIN32 )
        ScanfResult = scanf_s( "%d", &StreamId );
#endif
        if ( StreamId < 0 || 3 < StreamId )
        {
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            printf( "Input stream id is wrong(%d), should be in range of 0~3 \n", StreamId );
            return -1;
        }

        FD_HANDLE VideoEncode;
        Result = g_MediaCenter.CreateCodec( true, 0, StreamId, MEDIA_VIDEO, EncParam[StreamId].s_EncodeType, &EncParam[StreamId], sizeof(VideoEncodeParam), &VideoEncode );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            printf( "MediaCenter.CreateCodec, Result=%x \n", (int32_t)Result );
            return -1;
        }
        printf( "MediaCenter.CreateCodec, Result=%x \n", (int32_t)Result );

        Result = g_MediaCenter.StartCodec2( VideoEncode );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            printf( "MediaCenter.StartCodec, Result=%x \n", (int32_t)Result );
            return -1;
        }
        printf( "MediaCenter.StartCodec, Result=%x \n", (int32_t)Result );

        FD_HANDLE VideoTransport;
        Result = g_MediaTransport.Start( true, 0, StreamId, MEDIA_VIDEO, EncParam[StreamId].s_EncodeType, &EncParam[StreamId], sizeof(VideoEncodeParam), &VideoTransport );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.StopCodec2( VideoEncode );
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            printf( "failed to MediaTransport.Start, Result=%x \n", (int32_t)Result );
            return -1;
        }
        printf( "MediaTransport.Start, Result=%x \n", (int32_t)Result );

        Result = GetCodecConfiguration( g_MediaCenter, VideoEncode );

        uint32_t SetCodecConfigurationRunTimes = 0;
#if 0
        printf( "please input set encode configuration time, such as 20.\n" );
#if defined( __linux__ )
        ScanfResult = scanf( "%d", &SetCodecConfigurationRunTimes );
#elif defined( _WIN32 )
        ScanfResult = scanf_s( "%d", &SetCodecConfigurationRunTimes );
#endif
#endif
        if ( 0 >= SetCodecConfigurationRunTimes )
        {
            SetCodecConfigurationRunTimes = 1;
        }

        uint32_t i = 0;
        uint32_t BeginTime = (uint32_t) time( NULL );
        printf( "----------------------------------------------------- SetCodecConfiguration begin(%d) system time=%d \n", SetCodecConfigurationRunTimes, BeginTime );
        do
        {
            Result = SetCodecConfiguration( g_MediaCenter, VideoEncode );
        }
        while ( ++i < SetCodecConfigurationRunTimes );
        uint32_t EndTime = (uint32_t) time( NULL );
        printf( "----------------------------------------------------- SetCodecConfiguration end(%d) system time=%d, duration=%d \n", SetCodecConfigurationRunTimes, EndTime, EndTime-BeginTime );

        Result = GetOsdConfiguration( g_MediaCenter, VideoEncode );
        Result = SetOsdConfiguration( g_MediaCenter, VideoEncode );

        Result = ForceGenerateIdrFrame( g_MediaCenter, VideoEncode );

        // Test Image
        FD_HANDLE ImageHandle = NULL;
        Result = g_MediaCenter.OpenImageDevice( 0, 0, &ImageHandle );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.StopCodec2( VideoEncode );
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            printf( "failed to MediaCenter.OpenImageDevice, Result=%x \n", (int32_t)Result );
            return -1;
        }

        Result = TestImageOperation( g_MediaCenter, ImageHandle );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.CloseImageDevice( ImageHandle );
            g_MediaCenter.StopCodec2( VideoEncode );
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            printf( "failed to TestImageOperation, Result=%x \n", (int32_t)Result );
            return -1;
        }

        // Test Auto Focus
        FD_HANDLE AutoFocusHandle = NULL;
        Result = g_MediaCenter.OpenAutoFocusDevice( 0, &AutoFocusHandle );
        if ( SUCCEEDED( Result ) )
        {
            Result = TestAutoFocusOperation( g_MediaCenter, AutoFocusHandle );
            if ( FAILED( Result ) )
            {
                g_MediaCenter.CloseAutoFocusDevice( AutoFocusHandle );
                g_MediaCenter.CloseImageDevice( ImageHandle );
                g_MediaCenter.StopCodec2( VideoEncode );
                g_MediaCenter.DestroyCodec( VideoEncode );
                g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
                printf( "failed to TestAutoFocusOperation, Result=%x \n", (int32_t)Result );
                return -1;
            }

            // Test Zoom
            FD_HANDLE ZoomHandle = NULL;
            Result = g_MediaCenter.OpenZoomDevice( 0, &ZoomHandle );
            if ( SUCCEEDED( Result ) )
            {
                Result = TestZoomOperation( g_MediaCenter, ZoomHandle );
                if ( FAILED( Result ) )
                {
                    g_MediaCenter.CloseZoomDevice( ZoomHandle );
                    g_MediaCenter.CloseAutoFocusDevice( AutoFocusHandle );
                    g_MediaCenter.CloseImageDevice( ImageHandle );
                    g_MediaCenter.StopCodec2( VideoEncode );
                    g_MediaCenter.DestroyCodec( VideoEncode );
                    g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
                    printf( "failed to TestZoomOperation, Result=%x \n", (int32_t)Result );
                    return -1;
                }

#define TEST_ZOOM_OPERATIOIN_COUNT 100
                for ( int32_t i = 0; i < TEST_ZOOM_OPERATIOIN_COUNT; ++i )
                {
                    struct timeval StartZoomTime;
                    gettimeofday1( &StartZoomTime, NULL );

                    Result = g_MediaCenter.StartZoom( AutoFocusHandle, 1, ZoomHandle, 1 );// zoom in
                    if ( FAILED( Result ) )
                    {
                        break;
                    }

                    int32_t ZoomPosition = 0;
                    Result = g_MediaCenter.StopZoom( AutoFocusHandle, 0, ZoomHandle, 0, &ZoomPosition );
                    if ( FAILED( Result ) )
                    {
                        break;
                    }

                    struct timeval StopZoomTime;
                    gettimeofday1( &StopZoomTime, NULL );
                    uint64_t Duration = (StopZoomTime.tv_sec - StartZoomTime.tv_sec)*1000000 + StopZoomTime.tv_usec - StartZoomTime.tv_usec;
                    printf( "The %d time zoom in spent %llu(us), ZoomPosition=%d \n", i, Duration, ZoomPosition );
                }

                for ( int32_t i = 0; i < TEST_ZOOM_OPERATIOIN_COUNT; ++i )
                {
                    struct timeval StartZoomTime;
                    gettimeofday1( &StartZoomTime, NULL );

                    Result = g_MediaCenter.StartZoom( AutoFocusHandle, 1, ZoomHandle, 2 );// zoom out
                    if ( FAILED( Result ) )
                    {
                        break;
                    }

                    int32_t ZoomPosition = 0;
                    Result = g_MediaCenter.StopZoom( AutoFocusHandle, 0, ZoomHandle, 0, &ZoomPosition );
                    if ( FAILED( Result ) )
                    {
                        break;
                    }

                    struct timeval StopZoomTime;
                    gettimeofday1( &StopZoomTime, NULL );
                    uint64_t Duration = (StopZoomTime.tv_sec - StartZoomTime.tv_sec)*1000000 + StopZoomTime.tv_usec - StartZoomTime.tv_usec;
                    printf( "The %d time zoom out spent %llu(us), ZoomPosition=%d \n", i, Duration, ZoomPosition );
                }

                Result = g_MediaCenter.CloseZoomDevice( ZoomHandle );
                if ( FAILED( Result ) )
                {
                    g_MediaCenter.CloseAutoFocusDevice( AutoFocusHandle );
                    g_MediaCenter.CloseImageDevice( ImageHandle );
                    g_MediaCenter.StopCodec2( VideoEncode );
                    g_MediaCenter.DestroyCodec( VideoEncode );
                    g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
                    printf( "failed to MediaCenter.CloseZoomDevice, Result=%x \n", (int32_t)Result );
                    return -1;
                }
            }
            else
            {
                printf( "failed to MediaCenter.OpenZoomDevice, Result=%x \n", (int32_t)Result );
            }

            Result = g_MediaCenter.CloseAutoFocusDevice( AutoFocusHandle );
            if ( FAILED( Result ) )
            {
                g_MediaCenter.CloseImageDevice( ImageHandle );
                g_MediaCenter.StopCodec2( VideoEncode );
                g_MediaCenter.DestroyCodec( VideoEncode );
                g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
                printf( "failed to MediaCenter.CloseAutoFocusDevice, Result=%x \n", (int32_t)Result );
                return -1;
            }
        }
        else
        {
            printf( "failed to MediaCenter.OpenAutoFocusDevice, Result=%x \n", (int32_t)Result );
        }

        Result = g_MediaCenter.CloseImageDevice( ImageHandle );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.StopCodec2( VideoEncode );
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            printf( "failed to MediaCenter.CloseImageDevice, Result=%x \n", (int32_t)Result );
            return -1;
        }

        Info[0].VideoId = 0;
        Info[0].Encode = VideoEncode;
        Info[0].Transport = VideoTransport;
        g_MediaTransport.SetCallback( RTSPEventProcess, &Info[0] );

        do
        {
            GMI_Sleep( 1000 );
        }
        while ( --RunTime );

        Result = g_MediaTransport.Stop( VideoTransport );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.StopCodec2( VideoEncode );
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            printf( "failed to MediaTransport.Stop, Result=%x \n", (int32_t)Result );
            return -1;
        }
        printf( "MediaTransport.Stop, Result=%x \n", (int32_t)Result );

        Result = g_MediaCenter.StopCodec2( VideoEncode );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            printf( "failed to MediaCenter.StopCodec, Result=%x \n", (int32_t)Result );
            return -1;
        }
        printf( "MediaCenter.StopCodec, Result=%x \n", (int32_t)Result );

        Result = g_MediaCenter.DestroyCodec( VideoEncode );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            printf( "failed to MediaCenter.DestroyCodec, Result=%x \n", (int32_t)Result );
            return -1;
        }
        printf( "MediaCenter.DestroyCodec, Result=%x \n", (int32_t)Result );
    }
    else
    {
        for ( int32_t i = 0; i < MAX_STREAM_NUMBER; ++i )
        {
            Result = StartVideo( i, &EncParam[i], &Info[i] );
            if ( FAILED( Result ) )
            {
                g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
                printf( "failed to StartVideo, Result=%x, i =%d \n", (int32_t)Result, i );
                return Result;
            }
        }

        do
        {
            GMI_Sleep( 1000 );
        }
        while ( --RunTime );

        for ( int32_t i = 0; i < MAX_STREAM_NUMBER; ++i )
        {
            printf( "prepare to StopVideo, i =%d \n", i );
            Result = StopVideo( &Info[i] );
            if ( FAILED( Result ) )
            {
                g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
                printf( "failed to StopVideo, Result=%x, i =%d \n", (int32_t)Result, i );
                return Result;
            }
        }

    }

    Result = g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
    if ( FAILED( Result ) )
    {
        printf( "failed to MediaCenter.CloseVinVoutDevice, Result=%x \n", (int32_t)Result );
        return -1;
    }

    return GMI_SUCCESS;
}

GMI_RESULT TestAudioCodec( uint32_t RunTime, int32_t CodecMode )
{
    uint32_t TestNumber = 5;
    if ( AUDIO_ENCODE_MODE == CodecMode )
    {
        printf( "please choose encode audio test number, such as %d \n", TestNumber );
    }
    else
    {
        printf( "please choose decode audio test number, such as %d \n", TestNumber );
    }

#if defined( __linux__ )
    int32_t ScanfResult = scanf( "%d", &TestNumber );
#elif defined( _WIN32 )
    int32_t ScanfResult = scanf_s( "%d", &TestNumber );
#endif
    READ_MYSELF( ScanfResult );

    if ( 1 > TestNumber )
    {
        printf( "run_times parameter is wrong, test number = %d \n", TestNumber );
        return -1;
    }

    uint32_t FrameRate = 25;
    printf( "please input frame rate, such as %d \n", FrameRate );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &FrameRate );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &FrameRate );
#endif

    uint32_t BitRate = 64;
    printf( "please input bit rate(uint:kpbs), such as %d \n", BitRate );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &BitRate );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &BitRate );
#endif

    AudioDecParam AudioDecodeParam;
    AudioDecodeParam.s_Codec        = CODEC_G711A;
    AudioDecodeParam.s_SampleFreq   = 8000;
    AudioDecodeParam.s_BitWidth     = 16;
    AudioDecodeParam.s_ChannelNum   = 1;
    AudioDecodeParam.s_FrameRate    = (uint16_t) FrameRate;
    AudioDecodeParam.s_BitRate      = (uint16_t) BitRate;
    AudioDecodeParam.s_Volume       = 50;
    AudioDecodeParam.s_AecFlag      = 0;
    AudioDecodeParam.s_AecDelayTime = 0;
    FD_HANDLE AudioDecode = NULL;

    GMI_RESULT Result = GMI_FAIL;
    do
    {
        printf( "TestAudioCodec, StartCodecAudio at %d time \n", TestNumber );

        Result = StartCodecAudio( AUDIO_ENCODE_MODE == CodecMode, 0, &AudioDecodeParam, &AudioDecode );
        if ( FAILED( Result ) )
        {
            printf( "TestAudioCodec, failed to StartCodecAudio, Result=%x \n", (int32_t)Result );
            return -1;
        }

        uint32_t TempRunTime = RunTime;
        do
        {
            GMI_Sleep( 1000 );
        }
        while ( --TempRunTime );

        Result = StopCodecAudio( AudioDecode );
        if ( FAILED( Result ) )
        {
            printf( "TestAudioCodec, failed to StopCodecAudio, Result=%x \n", (int32_t)Result );
            return -1;
        }

        printf( "TestAudioCodec, StopCodecAudio at %d time \n", TestNumber );
    }
    while ( --TestNumber );

    return GMI_SUCCESS;
}

#define IPC_MEDIA_DATA_DISPATCH_ENCODE_SERVER_PORT      56654
#define IPC_MEDIA_DATA_DISPATCH_ENCODE_CLIENT_PORT      56644

#define IPC_MEDIA_DATA_DISPATCH_DECODE_SERVER_PORT      56645
#define IPC_MEDIA_DATA_DISPATCH_DECODE_CLIENT_PORT      56655

#define IPC_MEDIA_DATA_DISPATCH_CLIENT_APPLICATION_ID   1
#define IPC_MEDIA_DATA_DISPATCH_MEDIA_TYPE              516
#define IPC_MEDIA_DATA_DISPATCH_MEDIA_ID                0
#define IPC_MEDIA_DATA_DISPATCH_MEDIA_SHARE_MEMORY_KEY  1239
#define IPC_MEDIA_DATA_DISPATCH_MEDIA_IPC_MUTEX_KEY     2356
#define IPC_MEDIA_DATA_DISPATCH_MEDIA_SHARE_MEMORY_SIZE 16*1024*1024
#define IPC_MEDIA_DATA_DISPATCH_MEDIA_MAX_DATA_SIZE     512*1024
#define IPC_MEDIA_DATA_DISPATCH_MEDIA_MIN_DATA_SIZE     8*1024

GMI_RESULT TestAudioCodec2( uint32_t RunTime )
{
    uint32_t TestNumber = 5;
    printf( "please choose codec audio test number, such as %d \n", TestNumber );
#if defined( __linux__ )
    int32_t ScanfResult = scanf( "%d", &TestNumber );
#elif defined( _WIN32 )
    int32_t ScanfResult = scanf_s( "%d", &TestNumber );
#endif
    READ_MYSELF( ScanfResult );

    if ( 1 > TestNumber )
    {
        g_MediaCenter.Deinitialize();
        g_MediaTransport.Deinitialize();
        printf( "run_times parameter is wrong, test number = %d \n", TestNumber );
        return -1;
    }

    // audio codec parameter
    uint32_t FrameRate = 25;
    printf( "please input frame rate, such as %d \n", FrameRate );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &FrameRate );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &FrameRate );
#endif

    uint32_t BitRate = 64;
    printf( "please input bit rate(uint:kpbs), such as %d \n", BitRate );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &BitRate );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &BitRate );
#endif

    // ipc media data parameter begin
    uint32_t EncodeServerPort = 0;
    printf( "please input encode server port, for example: %d\n", IPC_MEDIA_DATA_DISPATCH_ENCODE_SERVER_PORT );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &EncodeServerPort );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &EncodeServerPort );
#endif

    uint32_t EncodeClientPort = 0;
    printf( "please input encode client port, for example: %d\n", IPC_MEDIA_DATA_DISPATCH_ENCODE_CLIENT_PORT );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &EncodeClientPort );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &EncodeClientPort );
#endif

    uint32_t DecodeServerPort = 0;
    printf( "please input decode server port, for example: %d\n", IPC_MEDIA_DATA_DISPATCH_DECODE_SERVER_PORT );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &DecodeServerPort );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &DecodeServerPort );
#endif

    uint32_t ApplicationId = 0;
    printf( "please input application id, for example: %d\n", IPC_MEDIA_DATA_DISPATCH_CLIENT_APPLICATION_ID );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &ApplicationId );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &ApplicationId );
#endif

    uint32_t MediaType = 0;
    printf( "please input media type, for example: %d\n", IPC_MEDIA_DATA_DISPATCH_MEDIA_TYPE );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &MediaType );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &MediaType );
#endif

    uint32_t MediaId = 0;
    printf( "please input media id, for example: %d\n", IPC_MEDIA_DATA_DISPATCH_MEDIA_ID );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &MediaId );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &MediaId );
#endif

    long_t ShareMemoryKey = 0;
    printf( "please input share memory key, for example: %d\n", IPC_MEDIA_DATA_DISPATCH_MEDIA_SHARE_MEMORY_KEY );
#if defined( __linux__ )
    ScanfResult = scanf( "%ld", &ShareMemoryKey );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%ld", &ShareMemoryKey );
#endif

    long_t IpcMutexKey = 0;
    printf( "please input ipc mutex key, for example: %d\n", IPC_MEDIA_DATA_DISPATCH_MEDIA_IPC_MUTEX_KEY );
#if defined( __linux__ )
    ScanfResult = scanf( "%ld", &IpcMutexKey );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%ld", &IpcMutexKey );
#endif

    int32_t PullMode = 0;
    printf( "please input work mode, 0: pull, 1: push\n" );
#if defined( __linux__ )
    ScanfResult = scanf( "%d", &PullMode );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%d", &PullMode );
#endif
    if ( 0 != PullMode && 1 != PullMode )
    {
        printf( "work mode is wrong, input is %d \n", PullMode );
        return -1;
    }
    // ipc media data parameter end
    uint32_t SleepTime = 1000/FrameRate;

    AudioDecParam AudioEncodeParam;
    AudioEncodeParam.s_Codec        = CODEC_G711A;
    AudioEncodeParam.s_SampleFreq   = 8000;
    AudioEncodeParam.s_BitWidth     = 16;
    AudioEncodeParam.s_ChannelNum   = 1;
    AudioEncodeParam.s_FrameRate    = (uint16_t) FrameRate;
    AudioEncodeParam.s_BitRate      = (uint16_t) BitRate;
    AudioEncodeParam.s_Volume       = 50;
    AudioEncodeParam.s_AecFlag      = 0;
    AudioEncodeParam.s_AecDelayTime = 0;

    AudioDecParam AudioDecodeParam;
    AudioDecodeParam.s_Codec        = CODEC_G711A;
    AudioDecodeParam.s_SampleFreq   = 8000;
    AudioDecodeParam.s_BitWidth     = 16;
    AudioDecodeParam.s_ChannelNum   = 1;
    AudioDecodeParam.s_FrameRate    = (uint16_t) FrameRate;
    AudioDecodeParam.s_BitRate      = (uint16_t) BitRate;
    AudioDecodeParam.s_Volume       = 50;
    AudioDecodeParam.s_AecFlag      = 0;
    AudioDecodeParam.s_AecDelayTime = 0;

    FD_HANDLE AudioEncode = NULL;
    FD_HANDLE AudioDecode = NULL;

    IPC_MediaDataClient MediaDataClient;
    IPC_MediaDataServer MediaDataServer;

    SafePtr<uint8_t, DefaultObjectsDeleter> Frame( BaseMemoryManager::Instance().News<uint8_t>( IPC_MEDIA_DATA_DISPATCH_MEDIA_MAX_DATA_SIZE ) );
    if ( NULL == Frame.GetPtr() )
    {
        return -1;
    }

    SafePtr<uint8_t, DefaultObjectsDeleter> ExtraData( BaseMemoryManager::Instance().News<uint8_t>( IPC_MEDIA_DATA_DISPATCH_MEDIA_MIN_DATA_SIZE ) );
    if ( NULL == ExtraData.GetPtr() )
    {
        return -1;
    }

    size_t FrameLength = 0;
    struct timeval FrameTS = { 0, 0 };
    size_t ExtraDataLength = 0;

    GMI_RESULT Result = GMI_FAIL;
    do
    {
        printf( "TestAudioCodec2, StartCodecAudio at %d time \n", TestNumber );

        Result = MediaDataServer.Initialize( (uint16_t) DecodeServerPort, MediaType, MediaId,
                                             ShareMemoryKey, IPC_MEDIA_DATA_DISPATCH_MEDIA_SHARE_MEMORY_SIZE, IPC_MEDIA_DATA_DISPATCH_MEDIA_MAX_DATA_SIZE, IPC_MEDIA_DATA_DISPATCH_MEDIA_MIN_DATA_SIZE, IpcMutexKey );
        if ( FAILED( Result ) )
        {
            printf( "TestAudioCodec2, MediaDataServer.Initialize failed, Result = %x \n", (uint32_t) Result );
            return -1;
        }

        Result = StartCodecAudio( false, 0, &AudioDecodeParam, &AudioDecode );
        if ( FAILED( Result ) )
        {
            MediaDataServer.Deinitialize();
            printf( "TestAudioCodec2, failed to StartCodecAudio(decoding audio), Result=%x \n", (int32_t)Result );
            return -1;
        }

        Result = StartCodecAudio( true, 0, &AudioEncodeParam, &AudioEncode );
        if ( FAILED( Result ) )
        {
            StopCodecAudio( AudioDecode );
            MediaDataServer.Deinitialize();
            printf( "TestAudioCodec2, failed to StartCodecAudio(encoding audio), Result=%x \n", (int32_t)Result );
            return -1;
        }

        Result = MediaDataClient.Initialize( (uint16_t)EncodeClientPort, ApplicationId );
        if ( FAILED( Result ) )
        {
            StopCodecAudio( AudioEncode );
            StopCodecAudio( AudioDecode );
            MediaDataServer.Deinitialize();
            printf( "TestAudioCodec2, failed to MediaDataClient.Initialize, Result=%x \n", (int32_t)Result );
            return -1;
        }

        if ( 0 == PullMode )
        {
            Result = MediaDataClient.Register( (uint16_t)EncodeServerPort, MediaType, MediaId, true, NULL, NULL );
            if ( FAILED( Result ) )
            {
                MediaDataClient.Deinitialize();
                StopCodecAudio( AudioEncode );
                StopCodecAudio( AudioDecode );
                MediaDataServer.Deinitialize();
                printf( "TestAudioCodec2, failed to MediaDataClient.Register, Result=%x \n", (int32_t)Result );
                return -1;
            }
        }
        else
        {
            MediaDataClient.Deinitialize();
            StopCodecAudio( AudioEncode );
            StopCodecAudio( AudioDecode );
            MediaDataServer.Deinitialize();
            printf( "for now, push mode is not supported \n" );
            return -1;
        }

        uint32_t RunCount = RunTime*FrameRate;
        do
        {
            FrameLength = IPC_MEDIA_DATA_DISPATCH_MEDIA_MAX_DATA_SIZE;
            ExtraDataLength = IPC_MEDIA_DATA_DISPATCH_MEDIA_MIN_DATA_SIZE;
            Result = MediaDataClient.Read( Frame.GetPtr(), &FrameLength, &FrameTS, ExtraData.GetPtr(), &ExtraDataLength );
            if ( SUCCEEDED( Result ) )
            {
                if ( 0 == RunCount%10 )
                {
                    printf( "TestAudioCodec2, WriteFrame, Frame=%p, FrameLength=%d, time=%d:%6d, ExtraData=%p, ExtraDataLength=%d \n",
                            Frame.GetPtr(), FrameLength, (uint32_t) FrameTS.tv_sec, (uint32_t) FrameTS.tv_usec, ExtraData.GetPtr(), ExtraDataLength );
                }

                Result = MediaDataServer.Write( Frame.GetPtr(), FrameLength, &FrameTS, ExtraData.GetPtr(), ExtraDataLength );
            }

            GMI_Sleep( SleepTime );
        }
        while ( --RunCount );

        MediaDataClient.Deinitialize();
        StopCodecAudio( AudioEncode );
        StopCodecAudio( AudioDecode );
        MediaDataServer.Deinitialize();

        printf( "TestAudioCodec2, StopCodecAudio at %d time \n", TestNumber );
    }
    while ( --TestNumber );

    return GMI_SUCCESS;
}

GMI_RESULT StartVideo( int32_t VideoId, VideoEncodeParam *EncParam, VideoInfo *Info )
{
    FD_HANDLE VideoEncode;
    GMI_RESULT Result = g_MediaCenter.CreateCodec( true, 0, VideoId, MEDIA_VIDEO, EncParam->s_EncodeType, EncParam, sizeof(VideoEncodeParam), &VideoEncode );
    if ( FAILED( Result ) )
    {
        printf( "StartVideo, failed to MediaCenter.CreateCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    Result = g_MediaCenter.StartCodec2( VideoEncode );
    if ( FAILED( Result ) )
    {
        g_MediaCenter.DestroyCodec( VideoEncode );
        printf( "StartVideo, failed to MediaCenter.StartCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    FD_HANDLE VideoTransport;
    Result = g_MediaTransport.Start( true, 0, VideoId, MEDIA_VIDEO, EncParam->s_EncodeType, EncParam, sizeof(VideoEncodeParam), &VideoTransport );
    if ( FAILED( Result ) )
    {
        g_MediaCenter.StopCodec2( VideoEncode );
        g_MediaCenter.DestroyCodec( VideoEncode );
        printf( "StartVideo, failed to MediaTransport.Start, Result=%x \n", (int32_t)Result );
        return Result;
    }

    Info->VideoId   = VideoId;
    Info->Encode    = VideoEncode;
    Info->Transport = VideoTransport;

    g_MediaTransport.SetCallback( RTSPEventProcess, Info );
    return GMI_SUCCESS;
}

GMI_RESULT StopVideo( VideoInfo *Info )
{
    GMI_RESULT Result = g_MediaTransport.Stop( Info->Transport );
    if ( FAILED( Result ) )
    {
        printf( "StopVideo, failed to MediaTransport.Stop, Result=%x \n", (int32_t)Result );
        return Result;
    }

    Result = g_MediaCenter.StopCodec2( Info->Encode );
    if ( FAILED( Result ) )
    {
        printf( "StopVideo, failed to MediaCenter.StopCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    Result = g_MediaCenter.DestroyCodec( Info->Encode );
    if ( FAILED( Result ) )
    {
        printf( "StopVideo, failed to MediaCenter.DestroyCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    return GMI_SUCCESS;
}

GMI_RESULT GetCodecConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE VideoEncodeHandle )
{
    printf( " ====== GetCodecConfiguration ====== \n" );

    VideoEncodeParam EncParam;

    size_t ParamLength = sizeof(VideoEncodeParam);
    GMI_RESULT Result = MediaCenter.GetCodecConfig( VideoEncodeHandle, &EncParam, &ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetCodecConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "StreamId = %d\n", EncParam.s_StreamId );
    printf( "EncodeType = %d\n", EncParam.s_EncodeType );
    printf( "FrameRate = %d\n", EncParam.s_FrameRate );
    printf( "EncodeWidth = %d\n", EncParam.s_EncodeWidth );
    printf( "EncodeHeight = %d\n", EncParam.s_EncodeHeight );
    printf( "BitRateType = %d\n", EncParam.s_BitRateType );
    printf( "BitRateAverage = %d\n", EncParam.s_BitRateAverage );
    printf( "BitRateUp = %d\n", EncParam.s_BitRateUp );
    printf( "BitRateDown = %d\n", EncParam.s_BitRateDown );
    printf( "FrameInterval = %d\n", EncParam.s_FrameInterval );
    printf( "EncodeQulity = %d\n", EncParam.s_EncodeQulity );
    printf( "Rotate = %d\n", EncParam.s_Rotate );

    printf( " ====== GetCodecConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT SetCodecConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE VideoEncodeHandle )
{
    printf( " ====== SetCodecConfiguration ====== \n" );
    VideoEncodeParam EncParam;
    memset( &EncParam, 0, sizeof(VideoEncodeParam) );

    EncParam.s_StreamId = 0;
    EncParam.s_EncodeType = CODEC_H264;
    EncParam.s_FrameRate = 25;
    EncParam.s_EncodeWidth = 1280;
    EncParam.s_EncodeHeight = 720;
    EncParam.s_BitRateType = BIT_VBR;
    EncParam.s_BitRateAverage = 2000;
    EncParam.s_BitRateUp = 4000;
    EncParam.s_BitRateDown = 32;
    EncParam.s_FrameInterval = EncParam.s_FrameRate;
    EncParam.s_EncodeQulity = 50;
    EncParam.s_Rotate = 0;

    printf( "StreamId = %d\n", EncParam.s_StreamId );
    printf( "EncodeType = %d\n", EncParam.s_EncodeType );
    printf( "FrameRate = %d\n", EncParam.s_FrameRate );
    printf( "EncodeWidth = %d\n", EncParam.s_EncodeWidth );
    printf( "EncodeHeight = %d\n", EncParam.s_EncodeHeight );
    printf( "BitRateType = %d\n", EncParam.s_BitRateType );
    printf( "BitRateAverage = %d\n", EncParam.s_BitRateAverage );
    printf( "BitRateUp = %d\n", EncParam.s_BitRateUp );
    printf( "BitRateDown = %d\n", EncParam.s_BitRateDown );
    printf( "FrameInterval = %d\n", EncParam.s_FrameInterval );
    printf( "EncodeQulity = %d\n", EncParam.s_EncodeQulity );
    printf( "Rotate = %d\n", EncParam.s_Rotate );

    size_t ParamLength = sizeof(VideoEncodeParam);
    GMI_RESULT Result = MediaCenter.SetCodecConfig( VideoEncodeHandle, &EncParam, ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== SetCodecConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( " ====== SetCodecConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT GetOsdConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE VideoEncodeHandle )
{
    printf( " ====== GetOsdConfiguration ====== \n" );
    VideoOSDParam OsdParam;

    size_t ParamLength = sizeof(VideoOSDParam);
    GMI_RESULT Result = MediaCenter.GetOsdConfig( VideoEncodeHandle, &OsdParam, &ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetOsdConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "StreamId    = %d\n", OsdParam.s_StreamId );
    printf( "DisplayType = %d\n", OsdParam.s_DisplayType );
    printf( "Flag        = %d\n", OsdParam.s_Flag );
    printf( "Language    = %d\n", OsdParam.s_Language );

    printf( "TimeDisplay.Flag      = %d\n", OsdParam.s_TimeDisplay.s_Flag );
    printf( "TimeDisplay.DateStyle = %d\n", OsdParam.s_TimeDisplay.s_DateStyle );
    printf( "TimeDisplay.TimeStyle = %d\n", OsdParam.s_TimeDisplay.s_TimeStyle );
    printf( "TimeDisplay.FontColor = %d\n", OsdParam.s_TimeDisplay.s_FontColor );
    printf( "TimeDisplay.FontStyle = %d\n", OsdParam.s_TimeDisplay.s_FontStyle );
    printf( "TimeDisplay.FontBlod  = %d\n", OsdParam.s_TimeDisplay.s_FontBlod );
    printf( "TimeDisplay.Rotate    = %d\n", OsdParam.s_TimeDisplay.s_Rotate );
    printf( "TimeDisplay.Italic    = %d\n", OsdParam.s_TimeDisplay.s_Italic );
    printf( "TimeDisplay.Outline   = %d\n", OsdParam.s_TimeDisplay.s_Outline );
    printf( "TimeDisplay.DisplayX  = %d\n", OsdParam.s_TimeDisplay.s_DisplayX );
    printf( "TimeDisplay.DisplayY  = %d\n", OsdParam.s_TimeDisplay.s_DisplayY );
    printf( "TimeDisplay.DisplayH  = %d\n", OsdParam.s_TimeDisplay.s_DisplayH );
    printf( "TimeDisplay.DisplayW  = %d\n", OsdParam.s_TimeDisplay.s_DisplayW );

    for ( int32_t i = 0; i < OSD_TEXT_NUM; ++i )
    {
        printf( "TextDisplay[%d].Flag            = %d\n", i, OsdParam.s_TextDisplay[i].s_Flag );
        printf( "TextDisplay[%d].FontColor       = %d\n", i, OsdParam.s_TextDisplay[i].s_FontColor );
        printf( "TextDisplay[%d].FontStyle       = %d\n", i, OsdParam.s_TextDisplay[i].s_FontStyle );
        printf( "TextDisplay[%d].FontBlod        = %d\n", i, OsdParam.s_TextDisplay[i].s_FontBlod );
        printf( "TextDisplay[%d].Rotate          = %d\n", i, OsdParam.s_TextDisplay[i].s_Rotate );
        printf( "TextDisplay[%d].Italic          = %d\n", i, OsdParam.s_TextDisplay[i].s_Italic );
        printf( "TextDisplay[%d].Outline         = %d\n", i, OsdParam.s_TextDisplay[i].s_Outline );
        printf( "TextDisplay[%d].TextContentLen  = %d\n", i, OsdParam.s_TextDisplay[i].s_TextContentLen );
        if ( 0 < OsdParam.s_TextDisplay[i].s_Flag && 0 < OsdParam.s_TextDisplay[i].s_TextContentLen )
        {
            printf( "TextDisplay[%d].TextContent = %s\n", i, (char_t*)(OsdParam.s_TextDisplay[i].s_TextContent) );
        }
        printf( "TextDisplay[%d].DisplayX = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayX );
        printf( "TextDisplay[%d].DisplayY = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayY );
        printf( "TextDisplay[%d].DisplayH = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayH );
        printf( "TextDisplay[%d].DisplayW = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayW );
    }

    printf( " ====== GetOsdConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT SetOsdConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE VideoEncodeHandle )
{
    printf( " ====== SetOsdConfiguration ====== \n" );
    char_t textBuf[] = "helloworld&123好好123";

    VideoOSDParam OsdParam;
    memset( &OsdParam, 0, sizeof(VideoOSDParam) );

    OsdParam.s_StreamId    = 0;
    OsdParam.s_DisplayType = 0;
    OsdParam.s_Flag        = 1;
    OsdParam.s_Language    = 0;

    OsdParam.s_TimeDisplay.s_Flag      = 1;
    OsdParam.s_TimeDisplay.s_DateStyle = 1;
    OsdParam.s_TimeDisplay.s_TimeStyle = 0;
    OsdParam.s_TimeDisplay.s_FontColor = 7;//COLOR_BLACK;
    OsdParam.s_TimeDisplay.s_FontStyle = 32;
    OsdParam.s_TimeDisplay.s_FontBlod  = 0;
    OsdParam.s_TimeDisplay.s_Rotate    = 0;
    OsdParam.s_TimeDisplay.s_Italic    = 0;
    OsdParam.s_TimeDisplay.s_Outline   = 0;
    OsdParam.s_TimeDisplay.s_DisplayX  = 0;
    OsdParam.s_TimeDisplay.s_DisplayY  = 0;
    OsdParam.s_TimeDisplay.s_DisplayH  = 100;
    OsdParam.s_TimeDisplay.s_DisplayW  = 100;

    int32_t i = 0;
    for ( ; i < OSD_TEXT_NUM-1; ++i )
    {
        OsdParam.s_TextDisplay[i].s_Flag           = 1;
        OsdParam.s_TextDisplay[i].s_Outline        = 0;
        OsdParam.s_TextDisplay[i].s_Italic         = 0;
        OsdParam.s_TextDisplay[i].s_FontColor      = 2;//COLOR_RED;
        OsdParam.s_TextDisplay[i].s_DisplayX       = 5;
        OsdParam.s_TextDisplay[i].s_DisplayY       = 50;
        OsdParam.s_TextDisplay[i].s_TextContentLen = (uint16_t) strlen(textBuf);
        OsdParam.s_TextDisplay[i].s_FontStyle      = 24;
        OsdParam.s_TextDisplay[i].s_DisplayH       = 50;
        OsdParam.s_TextDisplay[i].s_DisplayW       = 30;
        memcpy(OsdParam.s_TextDisplay[i].s_TextContent, textBuf, OsdParam.s_TextDisplay[i].s_TextContentLen);
        OsdParam.s_TextDisplay[i].s_FontBlod       = 0;
        OsdParam.s_TextDisplay[i].s_Rotate         = 0;
    }

#if 1 // display zoom parameter
    OsdParam.s_TextDisplay[i].s_DisplayX           = 0;
    OsdParam.s_TextDisplay[i].s_DisplayY           = 100;
#endif

    // printf parameter
    printf( "StreamId    = %d\n", OsdParam.s_StreamId );
    printf( "DisplayType = %d\n", OsdParam.s_DisplayType );
    printf( "Flag        = %d\n", OsdParam.s_Flag );
    printf( "Language    = %d\n", OsdParam.s_Language );

    printf( "TimeDisplay.Flag      = %d\n", OsdParam.s_TimeDisplay.s_Flag );
    printf( "TimeDisplay.DateStyle = %d\n", OsdParam.s_TimeDisplay.s_DateStyle );
    printf( "TimeDisplay.TimeStyle = %d\n", OsdParam.s_TimeDisplay.s_TimeStyle );
    printf( "TimeDisplay.FontColor = %d\n", OsdParam.s_TimeDisplay.s_FontColor );
    printf( "TimeDisplay.FontStyle = %d\n", OsdParam.s_TimeDisplay.s_FontStyle );
    printf( "TimeDisplay.FontBlod  = %d\n", OsdParam.s_TimeDisplay.s_FontBlod );
    printf( "TimeDisplay.Rotate    = %d\n", OsdParam.s_TimeDisplay.s_Rotate );
    printf( "TimeDisplay.Italic    = %d\n", OsdParam.s_TimeDisplay.s_Italic );
    printf( "TimeDisplay.Outline   = %d\n", OsdParam.s_TimeDisplay.s_Outline );
    printf( "TimeDisplay.DisplayX  = %d\n", OsdParam.s_TimeDisplay.s_DisplayX );
    printf( "TimeDisplay.DisplayY  = %d\n", OsdParam.s_TimeDisplay.s_DisplayY );
    printf( "TimeDisplay.DisplayH  = %d\n", OsdParam.s_TimeDisplay.s_DisplayH );
    printf( "TimeDisplay.DisplayW  = %d\n", OsdParam.s_TimeDisplay.s_DisplayW );

    for ( i = 0; i < OSD_TEXT_NUM; ++i )
    {
        printf( "TextDisplay[%d].Flag           = %d\n", i, OsdParam.s_TextDisplay[i].s_Flag );
        printf( "TextDisplay[%d].FontColor      = %d\n", i, OsdParam.s_TextDisplay[i].s_FontColor );
        printf( "TextDisplay[%d].FontStyle      = %d\n", i, OsdParam.s_TextDisplay[i].s_FontStyle );
        printf( "TextDisplay[%d].FontBlod       = %d\n", i, OsdParam.s_TextDisplay[i].s_FontBlod );
        printf( "TextDisplay[%d].Rotate         = %d\n", i, OsdParam.s_TextDisplay[i].s_Rotate );
        printf( "TextDisplay[%d].Italic         = %d\n", i, OsdParam.s_TextDisplay[i].s_Italic );
        printf( "TextDisplay[%d].Outline        = %d\n", i, OsdParam.s_TextDisplay[i].s_Outline );
        printf( "TextDisplay[%d].TextContentLen = %d\n", i, OsdParam.s_TextDisplay[i].s_TextContentLen );
        if ( 0 < OsdParam.s_TextDisplay[i].s_Flag && 0 < OsdParam.s_TextDisplay[i].s_TextContentLen )
        {
            printf( "TextDisplay[%d].TextContent= %s\n", i, (char_t*)(OsdParam.s_TextDisplay[i].s_TextContent) );
        }
        printf( "TextDisplay[%d].DisplayX       = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayX );
        printf( "TextDisplay[%d].DisplayY       = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayY );
        printf( "TextDisplay[%d].DisplayH       = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayH );
        printf( "TextDisplay[%d].DisplayW       = %d\n", i, OsdParam.s_TextDisplay[i].s_DisplayW );
    }

    size_t ParamLength = sizeof(VideoOSDParam);
    GMI_RESULT Result = MediaCenter.SetOsdConfig( VideoEncodeHandle, &OsdParam, ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== SetOsdConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( " ====== SetOsdConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT ForceGenerateIdrFrame( MediaCenterProxy& MediaCenter, FD_HANDLE VideoEncodeHandle )
{
    printf( " ====== ForceGenerateIdrFrame ====== \n" );

    GMI_RESULT Result = MediaCenter.ForceGenerateIdrFrame( VideoEncodeHandle );
    if ( FAILED( Result ) )
    {
        printf( " ====== ForceGenerateIdrFrame Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( " ====== ForceGenerateIdrFrame ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT TestVinVoutOperation( MediaCenterProxy& MediaCenter, FD_HANDLE VinVoutHandle )
{
    GMI_RESULT Result = SetVinVoutConfiguration( MediaCenter, VinVoutHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestVinVoutOperation, SetVinVoutConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = GetVinVoutConfiguration( MediaCenter, VinVoutHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestVinVoutOperation, GetVinVoutConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    return GMI_SUCCESS;
}

GMI_RESULT GetVinVoutConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE VinVoutHandle )
{
    printf( " ====== GetVinVoutConfiguration ====== \n" );
    VideoInParam  Vin;
    size_t        VinParameterLength = sizeof(VideoInParam);
    VideoOutParam Vout;
    size_t        VoutParameterLength = sizeof(VideoOutParam);

    GMI_RESULT Result = MediaCenter.GetVinVoutConfig( VinVoutHandle, &Vin, &VinParameterLength, &Vout, &VoutParameterLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetVinVoutConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "Vin.s_VinFlag=%d \n",          Vin.s_VinFlag );
    printf( "Vin.s_VinMode=%d \n",          Vin.s_VinMode );
    printf( "Vin.s_VinFrameRate=%d \n",     Vin.s_VinFrameRate );
    printf( "Vin.s_VinMirrorPattern=%d \n", Vin.s_VinMirrorPattern );
    printf( "Vin.s_VinBayerPattern=%d \n",  Vin.s_VinBayerPattern );

    printf( "Vout.s_VoutMode=%d \n",  Vout.s_VoutMode );
    printf( "Vout.s_VoutType=%d \n",  Vout.s_VoutType );
    printf( "Vout.s_VoutFlip=%d \n",  Vout.s_VoutFlip );

    printf( " ====== GetVinVoutConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT SetVinVoutConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE VinVoutHandle )
{
    printf( " ====== SetVinVoutConfiguration ====== \n" );
    VideoInParam  Vin;
    size_t        VinParameterLength = sizeof(VideoInParam);
    VideoOutParam Vout;
    size_t        VoutParameterLength = sizeof(VideoOutParam);

    Vin.s_VinFlag          = 1;
    Vin.s_VinMode          = 0;
    Vin.s_VinFrameRate     = 30;
    Vin.s_VinMirrorPattern = 4;
    Vin.s_VinBayerPattern  = 4;

    printf( "Vin.s_VinFlag=%d \n",          Vin.s_VinFlag );
    printf( "Vin.s_VinMode=%d \n",          Vin.s_VinMode );
    printf( "Vin.s_VinFrameRate=%d \n",     Vin.s_VinFrameRate );
    printf( "Vin.s_VinMirrorPattern=%d \n", Vin.s_VinMirrorPattern );
    printf( "Vin.s_VinBayerPattern=%d \n",  Vin.s_VinBayerPattern );

    Vout.s_VoutMode = 1;
    Vout.s_VoutType = 0;
    Vout.s_VoutFlip = 0;

    printf( "Vout.s_VoutMode=%d \n",  Vout.s_VoutMode );
    printf( "Vout.s_VoutType=%d \n",  Vout.s_VoutType );
    printf( "Vout.s_VoutFlip=%d \n",  Vout.s_VoutFlip );

    GMI_RESULT Result = MediaCenter.SetVinVoutConfig( VinVoutHandle, &Vin, VinParameterLength, &Vout, VoutParameterLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== SetVinVoutConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( " ====== SetVinVoutConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT TestImageOperation( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    GMI_RESULT Result = SetBaseImageConfiguration( MediaCenter, ImageHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestImageOperation, SetBaseImageConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = GetBaseImageConfiguration( MediaCenter, ImageHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestImageOperation, GetBaseImageConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = SetAdvancedImageConfiguration( MediaCenter, ImageHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestImageOperation, SetAdvancedImageConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = GetAdvancedImageConfiguration( MediaCenter, ImageHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestImageOperation, GetAdvancedImageConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = SetAutoFocusConfiguration( MediaCenter, ImageHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestImageOperation, SetAutoFocusConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = GetAutoFocusConfiguration( MediaCenter, ImageHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestImageOperation, GetAutoFocusConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = SetDaynightConfiguration( MediaCenter, ImageHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestImageOperation, SetDaynightConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = GetDaynightConfiguration( MediaCenter, ImageHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestImageOperation, GetDaynightConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = SetWhiteBalanceConfiguration( MediaCenter, ImageHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestImageOperation, SetWhiteBalanceConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = GetWhiteBalanceConfiguration( MediaCenter, ImageHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestImageOperation, GetWhiteBalanceConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    return GMI_SUCCESS;
}

GMI_RESULT GetBaseImageConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    printf( " ====== GetBaseImageConfiguration ====== \n" );

    ImageBaseParam ImageParam;

    size_t ParamLength = sizeof(ImageBaseParam);
    GMI_RESULT Result = MediaCenter.GetBaseImageConfig( ImageHandle, &ImageParam, &ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetBaseImageConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "ExposureMode = %d\n", ImageParam.s_ExposureMode );
    printf( "ExposureValueMin = %d\n", ImageParam.s_ExposureValueMin );
    printf( "ExposureValueMax = %d\n", ImageParam.s_ExposureValueMax );
    printf( "GainMax = %d\n", ImageParam.s_GainMax );
    printf( "Brightness = %d\n", ImageParam.s_Brightness );
    printf( "Contrast = %d\n", ImageParam.s_Contrast );
    printf( "Saturation = %d\n", ImageParam.s_Saturation );
    printf( "Hue = %d\n", ImageParam.s_Hue );
    printf( "Sharpness = %d\n", ImageParam.s_Sharpness );

    printf( " ====== GetBaseImageConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT SetBaseImageConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    printf( " ====== SetImageConfiguration ====== \n" );
    ImageBaseParam ImageParam;
    memset( &ImageParam, 0, sizeof(ImageBaseParam) );

    ImageParam.s_ExposureMode = 0;
    ImageParam.s_ExposureValueMin = 5000;
    ImageParam.s_ExposureValueMax = 40000;
    ImageParam.s_GainMax = 36;
    ImageParam.s_Brightness = 0;
    ImageParam.s_Contrast = 128;
    ImageParam.s_Saturation = 64;
    ImageParam.s_Hue = 0;
    ImageParam.s_Sharpness = 128;

    printf( "ExposureMode = %d\n", ImageParam.s_ExposureMode );
    printf( "ExposureValueMin = %d\n", ImageParam.s_ExposureValueMin );
    printf( "ExposureValueMax = %d\n", ImageParam.s_ExposureValueMax );
    printf( "GainMax = %d\n", ImageParam.s_GainMax );
    printf( "Brightness = %d\n", ImageParam.s_Brightness );
    printf( "Contrast = %d\n", ImageParam.s_Contrast );
    printf( "Saturation = %d\n", ImageParam.s_Saturation );
    printf( "Hue = %d\n", ImageParam.s_Hue );
    printf( "Sharpness = %d\n", ImageParam.s_Sharpness );

    size_t ParamLength = sizeof(ImageBaseParam);
    GMI_RESULT Result = MediaCenter.SetBaseImageConfig( ImageHandle, &ImageParam, ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== SetImageConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( " ====== SetImageConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT GetAdvancedImageConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    printf( " ====== GetAdvancedImageConfiguration ====== \n" );

    ImageAdvanceParam ImageParam;

    size_t ParamLength = sizeof(ImageAdvanceParam);
    GMI_RESULT Result = MediaCenter.GetAdvancedImageConfig( ImageHandle, &ImageParam, &ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetAdvancedImageConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "ImageParam.s_MeteringMode = %d\n",      ImageParam.s_MeteringMode );
    printf( "ImageParam.s_BackLightCompFlag = %d\n", ImageParam.s_BackLightCompFlag );
    printf( "ImageParam.s_DcIrisFlag = %d\n",        ImageParam.s_DcIrisFlag );
    printf( "ImageParam.s_LocalExposure = %d\n",     ImageParam.s_LocalExposure );
    printf( "ImageParam.s_MctfStrength = %d\n",      ImageParam.s_MctfStrength );
    printf( "ImageParam.s_DcIrisDuty = %d\n",        ImageParam.s_DcIrisDuty );
    printf( "ImageParam.s_AeTargetRatio = %d\n",     ImageParam.s_AeTargetRatio );

    printf( " ====== GetAdvancedImageConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT SetAdvancedImageConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    printf( " ====== SetAdvancedImageConfiguration ====== \n" );

    ImageAdvanceParam ImageParam;
    memset( &ImageParam, 0, sizeof(ImageAdvanceParam) );
    ImageParam.s_MeteringMode      = 0;
    ImageParam.s_BackLightCompFlag = 1;
    ImageParam.s_DcIrisFlag        = 1;
    ImageParam.s_LocalExposure     = 1;
    ImageParam.s_MctfStrength      = 32;
    ImageParam.s_DcIrisDuty        = 100;
    ImageParam.s_AeTargetRatio     = 100;

    printf( "ImageParam.s_MeteringMode = %d\n",      ImageParam.s_MeteringMode );
    printf( "ImageParam.s_BackLightCompFlag = %d\n", ImageParam.s_BackLightCompFlag );
    printf( "ImageParam.s_DcIrisFlag = %d\n",        ImageParam.s_DcIrisFlag );
    printf( "ImageParam.s_LocalExposure = %d\n",     ImageParam.s_LocalExposure );
    printf( "ImageParam.s_MctfStrength = %d\n",      ImageParam.s_MctfStrength );
    printf( "ImageParam.s_DcIrisDuty = %d\n",        ImageParam.s_DcIrisDuty );
    printf( "ImageParam.s_AeTargetRatio = %d\n",     ImageParam.s_AeTargetRatio );

    size_t ParamLength = sizeof(ImageAdvanceParam);
    GMI_RESULT Result = MediaCenter.SetAdvancedImageConfig( ImageHandle, &ImageParam, ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetAdvancedImageConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( " ====== SetAdvancedImageConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT GetAutoFocusConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    printf( " ====== GetAutoFocusConfiguration ====== \n" );

    ImageAfParam ImageParam;

    size_t ParamLength = sizeof(ImageAfParam);
    GMI_RESULT Result = MediaCenter.GetAutoFocusConfig( ImageHandle, &ImageParam, &ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetAutoFocusConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "ImageParam.s_AfFlag = %d\n",   ImageParam.s_AfFlag );
    printf( "ImageParam.s_AfMode = %d\n",   ImageParam.s_AfMode );
    printf( "ImageParam.s_LensType = %d\n", ImageParam.s_LensType );
    printf( "ImageParam.s_ZmDist = %d\n",   ImageParam.s_ZmDist );
    printf( "ImageParam.s_FsDist = %d\n",   ImageParam.s_FsDist );
    printf( "ImageParam.s_FsNear = %d\n",   ImageParam.s_FsNear );
    printf( "ImageParam.s_FsFar = %d\n",    ImageParam.s_FsFar );

    printf( " ====== GetAutoFocusConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT SetAutoFocusConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    printf( " ====== SetAutoFocusConfiguration ====== \n" );

    ImageAfParam ImageParam;
    memset( &ImageParam, 0, sizeof(ImageAfParam) );
    ImageParam.s_AfFlag   = 1;
    ImageParam.s_AfMode   = 0;
    ImageParam.s_LensType = 0;
    ImageParam.s_ZmDist   = 0;
    ImageParam.s_FsDist   = 0;
    ImageParam.s_FsNear   = 0;
    ImageParam.s_FsFar    = 0;

    printf( "ImageParam.s_AfFlag = %d\n",   ImageParam.s_AfFlag );
    printf( "ImageParam.s_AfMode = %d\n",   ImageParam.s_AfMode );
    printf( "ImageParam.s_LensType = %d\n", ImageParam.s_LensType );
    printf( "ImageParam.s_ZmDist = %d\n",   ImageParam.s_ZmDist );
    printf( "ImageParam.s_FsDist = %d\n",   ImageParam.s_FsDist );
    printf( "ImageParam.s_FsNear = %d\n",   ImageParam.s_FsNear );
    printf( "ImageParam.s_FsFar = %d\n",    ImageParam.s_FsFar );

    size_t ParamLength = sizeof(ImageAfParam);
    GMI_RESULT Result = MediaCenter.SetAutoFocusConfig( ImageHandle, &ImageParam, ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetAutoFocusConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( " ====== SetAutoFocusConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT GetDaynightConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    printf( " ====== GetDaynightConfiguration ====== \n" );

    ImageDnParam ImageParam;

    size_t ParamLength = sizeof(ImageDnParam);
    GMI_RESULT Result = MediaCenter.GetDaynightConfig( ImageHandle, &ImageParam, &ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetDaynightConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "ImageParam.s_DnMode = %d\n",        ImageParam.s_DnMode );
    printf( "ImageParam.s_DnDurtime = %d\n",     ImageParam.s_DnDurtime );
    printf( "ImageParam.s_NightToDayThr = %d\n", ImageParam.s_NightToDayThr );
    printf( "ImageParam.s_DayToNightThr = %d\n", ImageParam.s_DayToNightThr );

    for ( int32_t i = 0; i < SCHEDULE_WEEK_DAYS; ++i )
    {
        printf( "ImageParam.s_DnSchedule.s_DnSchedFlag[i] = %d\n", ImageParam.s_DnSchedule.s_DnSchedFlag[i] );
        printf( "ImageParam.s_DnSchedule.s_StartTime[i] = %d\n",   ImageParam.s_DnSchedule.s_StartTime[i] );
        printf( "ImageParam.s_DnSchedule.s_EndTime[i] = %d\n",     ImageParam.s_DnSchedule.s_EndTime[i] );
    }

    printf( " ====== GetDaynightConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT SetDaynightConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    printf( " ====== SetDaynightConfiguration ====== \n" );

    ImageDnParam ImageParam;
    memset( &ImageParam, 0, sizeof(ImageDnParam) );

    ImageParam.s_DnMode        = 2;
    ImageParam.s_DnDurtime     = 30;
    ImageParam.s_NightToDayThr = 100;
    ImageParam.s_DayToNightThr = 100;

    for ( int32_t i = 0; i < SCHEDULE_WEEK_DAYS; ++i )
    {
        ImageParam.s_DnSchedule.s_DnSchedFlag[i] = 1;
        ImageParam.s_DnSchedule.s_StartTime[i]   = 4*3600;
        ImageParam.s_DnSchedule.s_EndTime[i]     = 20*3600;
    }

    printf( "ImageParam.s_DnMode = %d\n",        ImageParam.s_DnMode );
    printf( "ImageParam.s_DnDurtime = %d\n",     ImageParam.s_DnDurtime );
    printf( "ImageParam.s_NightToDayThr = %d\n", ImageParam.s_NightToDayThr );
    printf( "ImageParam.s_DayToNightThr = %d\n", ImageParam.s_DayToNightThr );

    for ( int32_t i = 0; i < SCHEDULE_WEEK_DAYS; ++i )
    {
        printf( "ImageParam.s_DnSchedule.s_DnSchedFlag[i] = %d\n", ImageParam.s_DnSchedule.s_DnSchedFlag[i] );
        printf( "ImageParam.s_DnSchedule.s_StartTime[i] = %d\n",   ImageParam.s_DnSchedule.s_StartTime[i] );
        printf( "ImageParam.s_DnSchedule.s_EndTime[i] = %d\n",     ImageParam.s_DnSchedule.s_EndTime[i] );
    }

    size_t ParamLength = sizeof(ImageDnParam);
    GMI_RESULT Result = MediaCenter.SetDaynightConfig( ImageHandle, &ImageParam, ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetDaynightConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( " ====== SetDaynightConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT GetWhiteBalanceConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    printf( " ====== GetWhiteBalanceConfiguration ====== \n" );

    ImageWbParam ImageParam;

    size_t ParamLength = sizeof(ImageWbParam);
    GMI_RESULT Result = MediaCenter.GetWhiteBalanceConfig( ImageHandle, &ImageParam, &ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetWhiteBalanceConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( "ImageParam.s_WbMode = %d\n",  ImageParam.s_WbMode );
    printf( "ImageParam.s_WbRgain = %d\n", ImageParam.s_WbRgain );
    printf( "ImageParam.s_WbBgain = %d\n", ImageParam.s_WbBgain );

    printf( " ====== GetWhiteBalanceConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT SetWhiteBalanceConfiguration( MediaCenterProxy& MediaCenter, FD_HANDLE ImageHandle )
{
    printf( " ====== SetWhiteBalanceConfiguration ====== \n" );

    ImageWbParam ImageParam;
    memset( &ImageParam, 0, sizeof(ImageWbParam) );

    ImageParam.s_WbMode  = 0;
    ImageParam.s_WbRgain = 512;
    ImageParam.s_WbBgain = 512;

    printf( "ImageParam.s_WbMode = %d\n",  ImageParam.s_WbMode );
    printf( "ImageParam.s_WbRgain = %d\n", ImageParam.s_WbRgain );
    printf( "ImageParam.s_WbBgain = %d\n", ImageParam.s_WbBgain );

    size_t ParamLength = sizeof(ImageWbParam);
    GMI_RESULT Result = MediaCenter.SetWhiteBalanceConfig( ImageHandle, &ImageParam, ParamLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== GetDaynightConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( " ====== SetWhiteBalanceConfiguration ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT TestAutoFocusOperation( MediaCenterProxy& MediaCenter, FD_HANDLE AutoFocusHandle )
{
    printf( " ====== TestAutoFocusOperation ====== \n" );

    GMI_RESULT Result = MediaCenter.StartAutoFocus( AutoFocusHandle );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation StartAutoFocus failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    //GMI_Sleep( 1000 );
    Result = MediaCenter.PauseAutoFocus( AutoFocusHandle, 1 );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation PauseAutoFocus failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    //GMI_Sleep( 1000 );
    Result = MediaCenter.AutoFocusGlobalScan( AutoFocusHandle );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation AutoFocusGlobalScan failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    //GMI_Sleep( 1000 );
    Result = MediaCenter.SetAutoFocusMode( AutoFocusHandle, 1 );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation SetAutoFocusMode failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    Result = MediaCenter.NotifyAutoFocus( AutoFocusHandle, 0, NULL, 0 );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation NotifyAutoFocus failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    Result = MediaCenter.SetFocusPosition( AutoFocusHandle, 100 );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation SetFocusPosition failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    int32_t  FocusPos = 0;
    Result = MediaCenter.GetFocusPosition( AutoFocusHandle, &FocusPos );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation GetFocusPosition failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }
    printf( "FocusPos = %d \n", FocusPos );

    int32_t  MinFocusPos = 0, MaxFocusPos = 0;
    Result = MediaCenter.GetFocusRange( AutoFocusHandle, &MinFocusPos, &MaxFocusPos );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation GetFocusRange failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }
    printf( "MinFocusPos = %d, MaxFocusPos = %d \n", MinFocusPos, MaxFocusPos );

#if 0
    Result = MediaCenter.ResetFocusMotor( AutoFocusHandle );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation ResetFocusMotor failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }
#endif

    Result = MediaCenter.ControlAutoFocus( AutoFocusHandle, AF_DIR_MODE_IN );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation ControlAutoFocus failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    Result = MediaCenter.SetAutoFocusStep( AutoFocusHandle, 15 );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation SetAutoFocusStep failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    //GMI_Sleep( 1000 );
    Result = MediaCenter.StopAutoFocus( AutoFocusHandle );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestAutoFocusOperation StopAutoFocus failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    printf( " ====== TestAutoFocusOperation ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT TestZoomOperation( MediaCenterProxy& MediaCenter, FD_HANDLE ZoomHandle )
{
    printf( " ====== TestZoomOperation ====== \n" );

    GMI_RESULT Result = MediaCenter.SetZoomPosition( ZoomHandle, 100 );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestZoomOperation SetZoomPosition failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    int32_t ZoomPos = 0;
    Result = MediaCenter.GetZoomPosition( ZoomHandle, &ZoomPos );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestZoomOperation GetZoomPosition failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }
    printf( "ZoomPos = %d \n", ZoomPos );

    int32_t MinZoomPos = 0, MaxZoomPos = 0;
    Result = MediaCenter.GetZoomRange( ZoomHandle, &MinZoomPos, &MaxZoomPos );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestZoomOperation GetZoomRange failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }
    printf( "MinZoomPos = %d, MaxZoomPos = %d \n", MinZoomPos, MaxZoomPos );

    Result = MediaCenter.ResetZoomMotor( ZoomHandle );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestZoomOperation ResetZoomMotor failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    Result = MediaCenter.ControlZoom( ZoomHandle, 2 );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestZoomOperation ControlZoom failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    Result = MediaCenter.SetZoomStep( ZoomHandle, 20 );
    if ( FAILED( Result ) )
    {
        printf( " ====== TestZoomOperation SetZoomStep failed, Result=%x ====== \n", (uint32_t)Result );
        return Result;
    }

    printf( " ====== TestZoomOperation ====== \n" );
    return GMI_SUCCESS;
}

GMI_RESULT StartCodecAudio( boolean_t EncodeMode, int32_t AudioId, void_t *CodecParam, FD_HANDLE *Handle )
{
    GMI_RESULT Result = GMI_FAIL;
    if ( EncodeMode )
    {
        Result = g_MediaCenter.CreateCodec( true, 0, AudioId, MEDIA_AUDIO, ((AudioEncParam*)CodecParam)->s_Codec, CodecParam, sizeof(AudioEncParam), Handle );
    }
    else
    {
        Result = g_MediaCenter.CreateCodec( false, 0, AudioId, MEDIA_AUDIO, ((AudioDecParam*)CodecParam)->s_Codec, CodecParam, sizeof(AudioDecParam), Handle );
    }
    if ( FAILED( Result ) )
    {
        printf( "StartCodecAudio, failed to MediaCenter.CreateCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    Result = g_MediaCenter.StartCodec2( *Handle );
    if ( FAILED( Result ) )
    {
        g_MediaCenter.DestroyCodec( *Handle );
        printf( "StartCodecAudio, failed to MediaCenter.StartCodec2, Result=%x \n", (int32_t)Result );
        return Result;
    }

    return GMI_SUCCESS;
}

GMI_RESULT StopCodecAudio( FD_HANDLE Handle )
{
    GMI_RESULT Result = g_MediaCenter.StopCodec2( Handle );
    if ( FAILED( Result ) )
    {
        printf( "StopCodecAudio, failed to MediaCenter.StopCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    Result = g_MediaCenter.DestroyCodec( Handle );
    if ( FAILED( Result ) )
    {
        printf( "StopCodecAudio, failed to MediaCenter.DestroyCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    return GMI_SUCCESS;
}

class NetworkInitializer
{
public:
    NetworkInitializer()
    {
#if defined( _WIN32 )
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;

        wVersionRequested = MAKEWORD( 2, 2 );

        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 )
        {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            return;
        }

        /* Confirm that the WinSock DLL supports 2.2.*/
        /* Note that if the DLL supports versions greater    */
        /* than 2.2 in addition to 2.2, it will still return */
        /* 2.2 in wVersion since that is the version we      */
        /* requested.                                        */

        if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
        {
            /* Tell the user that we could not find a usable */
            /* WinSock DLL.                                  */
            WSACleanup();
            return;
        }

        /* The WinSock DLL is acceptable. Proceed. */
#elif defined( __linux__ )
#endif
    }

    ~NetworkInitializer()
    {
#if defined( _WIN32 )
        WSACleanup();
#elif defined( __linux__ )
#endif
    }
};
