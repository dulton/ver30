#ifndef __RTSP_STREAM_CTRL_H__
#define __RTSP_STREAM_CTRL_H__

#include <gmi_type_definitions.h>
#include <gmi_errors.h>

typedef struct tagSubStreamInfo
{
    uint32_t s_EncodeType; // Defined in media_ctrl.h
    uint32_t s_StreamId;   // Base from 0 
    uint32_t s_FrameRate;  // Frames per second
    uint32_t s_MaxBitRate; // Bits pre second
} SubStreamInfo;

enum
{
    eStreamUnknown = 0,
    eStreamOpened,
    eStreamClosed,
};

/*
 * StreamId     : Base from 1
 * SubStreamNum : Number of sub stream, MUST be at least 1 sub stream
 * Infos        : Sub stream information
 */
GMI_RESULT RtspOpenStream(uint32_t StreamId, uint32_t SubStreamNum, const SubStreamInfo * Infos);

/*
 * StreamId     : Base from 1
 */
GMI_RESULT RtspCloseStream(uint32_t StreamId);

/*
 * StreamId     : Base from 1
 * StreamStatus : Return the status of the stream
 */
GMI_RESULT RtspQueryStreamStatus(uint32_t StreamId, uint32_t * StreamStatus);

#endif // __RTSP_STREAM_CTRL_H__

