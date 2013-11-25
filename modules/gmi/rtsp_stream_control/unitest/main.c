#include <stdlib.h>

#include <gmi_media_ctrl.h>

#include <rtsp_stream_ctrl.h>

#include "debug.h"

#ifndef COUNT_OF
#define COUNT_OF(array) (sizeof(array) / sizeof(array[0]))
#endif

static const SubStreamInfo StreamInfo1[] = {
    {MEDIA_VIDEO_H264,  0, 25, 4000000},
    {MEDIA_AUDIO_G711A, 0, 20, 64000  },
};

static const SubStreamInfo StreamInfo2[] = {
    {MEDIA_VIDEO_H264,  1, 25, 2000000},
    {MEDIA_AUDIO_G711A, 0, 20, 64000  },
};

typedef struct tagStreamInfo
{
    uint32_t              s_SubStreamNum;
    const SubStreamInfo * s_SubStreamInfo;
} StreamInfo;

static const StreamInfo StreamTable[] = {
    {COUNT_OF(StreamInfo1), StreamInfo1},
    {COUNT_OF(StreamInfo2), StreamInfo2},
};

static const char_t * Status2String(uint32_t StreamStatus)
{
    switch(StreamStatus)
    {
        case eStreamOpened: return "Opened";
        case eStreamClosed: return "Closed";
    }

    return "Unknown";
}

int main(int argc, const char * argv [])
{
    GMI_RESULT RetVal   = GMI_SUCCESS;
    uint32_t   StreamId = 0;

    if (argc < 3)
    {
        PRINT_LOG(INFO, "Not enough parameter");
        return 0;
    }

    StreamId = atoi(argv[2]);
    if (StreamId > COUNT_OF(StreamTable) || StreamId == 0)
    {
        PRINT_LOG(INFO, "Stream id should be in [1, %u]", COUNT_OF(StreamTable));
        return 0;
    }

    if (strcmp(argv[1], "open") == 0)
    {
        RetVal = RtspOpenStream(StreamId, StreamTable[StreamId - 1].s_SubStreamNum, StreamTable[StreamId - 1].s_SubStreamInfo);
        PRINT_LOG(INFO, "Open stream %u %s", StreamId, RetVal == GMI_SUCCESS ? "succeed" : "failed");
    }
    else if (strcmp(argv[1], "close") == 0)
    {
        RetVal = RtspCloseStream(StreamId);
        PRINT_LOG(INFO, "Close stream %u %s", StreamId, RetVal == GMI_SUCCESS ? "succeed" : "failed");
    }
    else if (strcmp(argv[1], "query") == 0)
    {
        uint32_t StreamStatus = eStreamUnknown;
        RetVal = RtspQueryStreamStatus(StreamId, &StreamStatus);
        PRINT_LOG(INFO, "Query status of stream %u %s", StreamId, RetVal == GMI_SUCCESS ? "succeed" : "failed");
        if (RetVal == GMI_SUCCESS)
        {
            PRINT_LOG(INFO, "Status of stream %u is %s", StreamId, Status2String(StreamStatus));
        }
    }
    else
    {
        PRINT_LOG(INFO, "Command should be in ('open', 'close' or 'query')");
    }

    return 0;
}

