#include "media_capture.h"
#include "configure.h"

#include "h264_capture.h"
#include "g711_capture.h"

std::map<SubStreamInfo, MediaFrameCapture *> MediaFrameCapture::ms_MediaFrameCaptureTable;

MediaFrameCapture * MediaFrameCapture::GetMediaFrameCapture(const SubStreamInfo & Info)
{
    std::map<SubStreamInfo, MediaFrameCapture *>::iterator it = MediaFrameCapture::ms_MediaFrameCaptureTable.find(Info);
    if (it == MediaFrameCapture::ms_MediaFrameCaptureTable.end() || NULL == it->second)
    {
        MediaFrameCapture * MediaCapture = NULL;

        switch (Info.s_EncodeType)
        {
            case MEDIA_VIDEO_H264:
                MediaCapture = new H264FrameCapture(Info);
                if (NULL == MediaCapture)
                {
                    PRINT_LOG(ERROR, "Failed to create H264FrameCapture");
                    return NULL;
                }
                break;

            case MEDIA_AUDIO_G711A:
                MediaCapture = new G711FrameCapture(Info);
                if (NULL == MediaCapture)
                {
                    PRINT_LOG(ERROR, "Failed to create G711FrameCapture");
                    return NULL;
                }
                break;

            case MEDIA_VIDEO_MJPEG:
            case MEDIA_VIDEO_MPEG4:
            case MEDIA_AUDIO_G711U:
            case MEDIA_AUDIO_G726:
                PRINT_LOG(ERROR, "Unsupported encode type");
                return NULL;

            default:
                PRINT_LOG(ERROR, "Unknown encode type");
                return NULL;
        }

        MediaFrameCapture::ms_MediaFrameCaptureTable[Info] = MediaCapture;
        return MediaCapture;
    }

    return it->second;
}

void_t MediaFrameCapture::DestroyAllMediaFrameCaptures()
{
    std::map<SubStreamInfo, MediaFrameCapture *>::iterator it = MediaFrameCapture::ms_MediaFrameCaptureTable.begin();
    while (it != MediaFrameCapture::ms_MediaFrameCaptureTable.end())
    {
        delete it->second;
        it = MediaFrameCapture::ms_MediaFrameCaptureTable.begin();
    }
}

MediaFrameCapture::MediaFrameCapture(const SubStreamInfo & Info)
    : m_CaptureCount(0), m_SubStreamInfo(Info)
    , m_FrameDuration(1000000 / Info.s_FrameRate)
    , m_Sleep(m_FrameDuration / 1000 + 10)
{
}

MediaFrameCapture::~MediaFrameCapture()
{
    // Remove media frame capture instance from table
    std::map<SubStreamInfo, MediaFrameCapture *>::iterator it = MediaFrameCapture::ms_MediaFrameCaptureTable.find(m_SubStreamInfo);
    if (it != MediaFrameCapture::ms_MediaFrameCaptureTable.end())
    {
        MediaFrameCapture::ms_MediaFrameCaptureTable.erase(it);
    }
    else
    {
        PRINT_LOG(ERROR, "Media frame capture instance MUST in the table");
    }
}

GMI_RESULT MediaFrameCapture::StartCapture()
{
    if (!IsCapturing())
    {
        GMI_RESULT RetVal = StartCaptureImpl();
        if (RetVal != GMI_SUCCESS)
        {
            return RetVal;
        }
    }

    m_CaptureCount ++;
    return GMI_SUCCESS;
}

GMI_RESULT MediaFrameCapture::StopCapture()
{
    if (!IsCapturing())
    {
        return GMI_SUCCESS;
    }

    m_CaptureCount --;
    if (0 == m_CaptureCount)
    {
        return StopCaptureImpl();
    }
    return GMI_SUCCESS;
}

