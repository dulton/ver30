#pragma once

#include "video_source_filter.h"

#define SAVE_H264_FILE                            0
#define MONITOR_H264_VIDEO_FRAME_INTERVAL         0
#define MONITOR_H264_VIDEO_FRAME_INTERVAL_NUMBER  600
#define USE_SIMULATOR_VIDEO_SOUCRE                0

class H264_VideoSourceFilter : public VideoSourceFilter
{
protected:
    H264_VideoSourceFilter(void);
    virtual ~H264_VideoSourceFilter(void);

public:
    static  H264_VideoSourceFilter*  CreateNew();
    friend class BaseMemoryManager;

    virtual GMI_RESULT	Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize );
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT  Play();
    virtual GMI_RESULT  Stop();

    GMI_RESULT GetEncodeConfig( void_t *EncodeParameter, size_t *EncodeParameterLength );
    GMI_RESULT SetEncodeConfig( const void_t *EncodeParameter, size_t EncodeParameterLength );

    GMI_RESULT GetOsdConfig( void_t *OsdParameter, size_t *OsdParameterLength );
    GMI_RESULT SetOsdConfig( const void_t *OsdParameter, size_t OsdParameterLength );

    GMI_RESULT ForceGenerateIdrFrame();

private:
    static void_t MediaEncCallBack( void_t *UserDataPtr, MediaEncInfo *EncInfo, ExtMediaEncInfo *ExtEncInfo );
#if USE_SIMULATOR_VIDEO_SOUCRE
    static void_t* SimulatedDataSourceThread( void_t *Argument );
    void_t* SimulatedDataSourceEntry();
#endif

    GMI_RESULT GetVideoMonitorEnableConfig( boolean_t *Enable );
    GMI_RESULT GetVideoFrameCheckInterval( uint32_t *FrameNumber );

private:
    FD_HANDLE      m_HardwareSource;
#if SAVE_H264_FILE
    FILE           *m_VideoRecordFile;
#endif
    boolean_t      m_VideoMonitorEnable;
    uint32_t       m_VideoFrameCheckInterval;
    uint64_t       m_FrameNumber;
    uint64_t       m_IFrameNumber;
    struct timeval m_FirstFrameTime;

#if MONITOR_H264_VIDEO_FRAME_INTERVAL
    struct timeval m_LastFrameTimeStamp;
    uint64_t       m_TotalFrameInterval;
    uint64_t       m_MaxFrameInterval;
    uint64_t       m_MinFrameInterval;
#endif

#if USE_SIMULATOR_VIDEO_SOUCRE
    GMI_Thread	   m_SimulatedDataSourceThread;
    boolean_t      m_ThreadWorking;
    boolean_t      m_ThreadExitFlag;
    FILE           *m_VideoSourceFile;
    uint32_t       m_VideoSourceFileLength;
    uint8_t        *m_VideoData;
    float_t        m_FrameRate;
#endif
};
