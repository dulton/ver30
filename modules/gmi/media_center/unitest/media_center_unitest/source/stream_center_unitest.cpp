// media_center_unitest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "gmi_system_headers.h"
#include "media_center.h"
#include "media_center_proxy.h"
#include "media_transport_proxy.h"
#include "timer_task_queue.h"

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

class NetworkInitializer;

struct VideoInfo
{
    int32_t   VideoId;
    FD_HANDLE Encode;
    FD_HANDLE Transport;
};

GMI_RESULT StartVideo( MediaCenter& Media, MediaTransportProxy& Transport, int32_t VideoId, VideoEncodeParam *EncParam, VideoInfo *Info );
GMI_RESULT StopVideo( MediaCenter& Media, MediaTransportProxy& Transport, VideoInfo *Info );

GMI_RESULT TestVinVoutOperation( MediaCenter& Media, FD_HANDLE VinVoutHandle );
GMI_RESULT GetVinVoutConfiguration( MediaCenter& Media, FD_HANDLE VinVoutHandle );
GMI_RESULT SetVinVoutConfiguration( MediaCenter& Media, FD_HANDLE VinVoutHandle );

MediaCenter         g_MediaCenter;
MediaTransportProxy g_MediaTransport;
int32_t             g_ForceGenerateIdrFrame = 0;

//MediaCenterProxy    g_MediaCenterProxy;

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

    SafePtr<NetworkInitializer> Initializer( BaseMemoryManager::Instance().New<NetworkInitializer>() );
    if ( NULL == Initializer.GetPtr() )
    {
        return -1;
    }

    VideoEncodeParam EncParam;
    EncParam.s_EncodeType     = CODEC_H264;
    EncParam.s_FrameRate      = 25;
    EncParam.s_EncodeWidth    = 1920;
    EncParam.s_EncodeHeight   = 1080;
    EncParam.s_BitRateType    = BIT_CBR;
    EncParam.s_BitRateAverage = 4000;
    EncParam.s_BitRateUp      = 0;
    EncParam.s_BitRateDown    = 0;
#if 1
    EncParam.s_FrameInterval  = 25;
#else
    EncParam.s_FrameInterval  = 120;
#endif
    EncParam.s_EncodeQulity   = 100;
    EncParam.s_Rotate         = 0;

    GMI_RESULT Result = g_MediaTransport.Initialize( RTSP_SERVER_PORT );
    if ( FAILED( Result ) )
    {
        printf( "failed to MediaTransport.Initialize, Result=%x \n", (int32_t)Result );
        return -1;
    }

    long_t count = 60;
    printf( "please input run time(second unit), such as %ld \n", count );
#if defined( __linux__ )
    ScanfResult = scanf( "%ld", &count );
#elif defined( _WIN32 )
    ScanfResult = scanf_s( "%ld", &count );
#endif

    printf( "we will run %ld seconds \n", count );

    FD_HANDLE VinVoutHandle = NULL;
    Result = g_MediaCenter.OpenVinVoutDevice( 0, 0, &VinVoutHandle );
    if ( FAILED( Result ) )
    {
        g_MediaTransport.Deinitialize();
        printf( "failed to MediaCenterProxy.OpenVinVoutDevice, Result=%x \n", (int32_t)Result );
        return -1;
    }

    Result = TestVinVoutOperation( g_MediaCenter, VinVoutHandle );
    if ( FAILED( Result ) )
    {
        g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
        g_MediaTransport.Deinitialize();
        printf( "failed to TestVinVoutOperation, Result=%x \n", (int32_t)Result );
        return -1;
    }

    struct timeval StartTime;
    gettimeofday1( &StartTime, NULL );

    VideoInfo Info[MAX_STREAM_NUMBER];

    if ( 0 == TestMode )
    {
        FD_HANDLE VideoEncode;
        Result = g_MediaCenter.CreateCodec( true, 0, 0, MEDIA_VIDEO, EncParam.s_EncodeType, &EncParam, sizeof(VideoEncodeParam), &VideoEncode );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            g_MediaTransport.Deinitialize();
            printf( "failed to MediaCenter.CreateCodec, Result=%x \n", (int32_t)Result );
            return -1;
        }

        Result = g_MediaCenter.StartCodec( VideoEncode );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            g_MediaTransport.Deinitialize();
            printf( "failed to MediaCenter.StartCodec, Result=%x \n", (int32_t)Result );
            return -1;
        }

        FD_HANDLE VideoTransport;
        Result = g_MediaTransport.Start( true, 0, 0, MEDIA_VIDEO, EncParam.s_EncodeType, &EncParam, sizeof(VideoEncodeParam), &VideoTransport );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.StopCodec( VideoEncode );
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            g_MediaTransport.Deinitialize();
            printf( "failed to MediaTransport.Start, Result=%x \n", (int32_t)Result );
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
        while ( --count );

        Result = g_MediaTransport.Stop( VideoTransport );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.StopCodec( VideoEncode );
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            g_MediaTransport.Deinitialize();
            printf( "failed to MediaTransport.Stop, Result=%x \n", (int32_t)Result );
            return -1;
        }

        Result = g_MediaCenter.StopCodec( VideoEncode );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.DestroyCodec( VideoEncode );
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            g_MediaTransport.Deinitialize();
            printf( "failed to MediaCenter.StopCodec, Result=%x \n", (int32_t)Result );
            return -1;
        }

        Result = g_MediaCenter.DestroyCodec( VideoEncode );
        if ( FAILED( Result ) )
        {
            g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
            g_MediaTransport.Deinitialize();
            printf( "failed to MediaCenter.DestroyCodec, Result=%x \n", (int32_t)Result );
            return -1;
        }

        printf( "MediaCenter.Stop, Result=%x \n", (int32_t)Result );
    }
    else
    {
        for ( int32_t i = 0; i < MAX_STREAM_NUMBER; ++i )
        {
            if ( 0 == i )
            {
                EncParam.s_EncodeWidth    = 1920;
                EncParam.s_EncodeHeight   = 1080;
                EncParam.s_BitRateAverage = 4000;
            }
            else if ( 1 == i )
            {
                EncParam.s_EncodeWidth    = 720;
                EncParam.s_EncodeHeight   = 480;
                EncParam.s_BitRateAverage = 2000;
            }
            else if ( 2 == i )
            {
                EncParam.s_EncodeWidth    = 720;
                EncParam.s_EncodeHeight   = 480;
                EncParam.s_BitRateAverage = 1000;
            }
            else if ( 3 == i )
            {
                EncParam.s_EncodeWidth    = 720;
                EncParam.s_EncodeHeight   = 480;
                EncParam.s_BitRateAverage = 500;
            }

            GMI_RESULT Result = StartVideo( g_MediaCenter, g_MediaTransport, i, &EncParam, &Info[i] );
            if ( FAILED( Result ) )
            {
                g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
                g_MediaTransport.Deinitialize();
                printf( "failed to StartVideo, Result=%x, i =%d \n", (int32_t)Result, i );
                return Result;
            }
        }

        do
        {
            GMI_Sleep( 1000 );
        }
        while ( --count );

        for ( int32_t i = 0; i < MAX_STREAM_NUMBER; ++i )
        {
            printf( "prepare to StopVideo, i =%d \n", i );
            GMI_RESULT Result = StopVideo( g_MediaCenter, g_MediaTransport, &Info[i] );
            if ( FAILED( Result ) )
            {
                g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
                g_MediaTransport.Deinitialize();
                printf( "failed to StopVideo, Result=%x, i =%d \n", (int32_t)Result, i );
                return Result;
            }
        }

    }

    Result = g_MediaCenter.CloseVinVoutDevice( VinVoutHandle );
    if ( FAILED( Result ) )
    {
        g_MediaTransport.Deinitialize();
        printf( "failed to MediaCenterProxy.CloseVinVoutDevice, Result=%x \n", (int32_t)Result );
        return -1;
    }

    struct timeval EndTime;
    gettimeofday1( &EndTime, NULL );

    int32_t duration = EndTime.tv_sec - StartTime.tv_sec +( EndTime.tv_usec - StartTime.tv_usec )/1000000;
    printf( "we already run %d seconds \n", duration );

    Result = g_MediaTransport.Deinitialize();
    if ( FAILED( Result ) )
    {
        printf( "failed to MediaTransport.Deinitialize, Result=%x \n", (int32_t)Result );
        return -1;
    }

    printf( "MediaTransport.Deinitialize, Result=%x \n", (int32_t)Result );

    return 0;
}

GMI_RESULT StartVideo( MediaCenter& Media, MediaTransportProxy& Transport, int32_t VideoId, VideoEncodeParam *EncParam, VideoInfo *Info )
{
    FD_HANDLE VideoEncode;
    GMI_RESULT Result = Media.CreateCodec( true, 0, VideoId, MEDIA_VIDEO, EncParam->s_EncodeType, EncParam, sizeof(VideoEncodeParam), &VideoEncode );
    if ( FAILED( Result ) )
    {
        printf( "failed to MediaCenter.CreateCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    Result = Media.StartCodec( VideoEncode );
    if ( FAILED( Result ) )
    {
        Media.DestroyCodec( VideoEncode );
        printf( "failed to MediaCenter.StartCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    FD_HANDLE VideoTransport;
    Result = Transport.Start( true, 0, VideoId, MEDIA_VIDEO, EncParam->s_EncodeType, EncParam, sizeof(VideoEncodeParam), &VideoTransport );
    if ( FAILED( Result ) )
    {
        Media.StopCodec( VideoEncode );
        Media.DestroyCodec( VideoEncode );
        printf( "failed to MediaTransport.Start, Result=%x \n", (int32_t)Result );
        return Result;
    }

    Info->VideoId   = VideoId;
    Info->Encode    = VideoEncode;
    Info->Transport = VideoTransport;

    Transport.SetCallback( RTSPEventProcess, Info );
    return GMI_SUCCESS;
}

GMI_RESULT StopVideo( MediaCenter& Media, MediaTransportProxy& Transport, VideoInfo *Info )
{
    GMI_RESULT Result = Transport.Stop( Info->Transport );
    if ( FAILED( Result ) )
    {
        printf( "failed to MediaTransport.Stop, Result=%x \n", (int32_t)Result );
        return Result;
    }

    Result = Media.StopCodec( Info->Encode );
    if ( FAILED( Result ) )
    {
        printf( "failed to MediaCenter.StopCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    Result = Media.DestroyCodec( Info->Encode );
    if ( FAILED( Result ) )
    {
        printf( "failed to MediaCenter.DestroyCodec, Result=%x \n", (int32_t)Result );
        return Result;
    }

    return GMI_SUCCESS;
}

GMI_RESULT TestVinVoutOperation( MediaCenter& Media, FD_HANDLE VinVoutHandle )
{
    GMI_RESULT Result = SetVinVoutConfiguration( Media, VinVoutHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestVinVoutOperation, SetVinVoutConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    Result = GetVinVoutConfiguration( Media, VinVoutHandle );
    if ( FAILED( Result ) )
    {
        printf( "TestVinVoutOperation, GetVinVoutConfiguration failed, Result=%x", (uint32_t) Result );
        return Result;
    }

    return GMI_SUCCESS;
}

GMI_RESULT GetVinVoutConfiguration( MediaCenter& Media, FD_HANDLE VinVoutHandle )
{
    printf( " ====== GetVinVoutConfiguration ====== \n" );
    VideoInParam  Vin;
    size_t        VinParameterLength = sizeof(VideoInParam);
    VideoOutParam Vout;
    size_t        VoutParameterLength = sizeof(VideoOutParam);

    GMI_RESULT Result = Media.GetVinVoutConfig( VinVoutHandle, &Vin, &VinParameterLength, &Vout, &VoutParameterLength );
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

GMI_RESULT SetVinVoutConfiguration( MediaCenter& Media, FD_HANDLE VinVoutHandle )
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

    GMI_RESULT Result = Media.SetVinVoutConfig( VinVoutHandle, &Vin, VinParameterLength, &Vout, VoutParameterLength );
    if ( FAILED( Result ) )
    {
        printf( " ====== SetVinVoutConfiguration Result=%x \n", (uint32_t) Result );
        return Result;
    }

    printf( " ====== SetVinVoutConfiguration ====== \n" );
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
