#include "application.h"
#include "configure.h"

#include "H264RealTimeStreamFramerEx.hh"

#ifndef MILLION
#define MILLION                     1000000
#endif

// Definition of nalu unit type
#define NALU_TYPE_NONIDR            1
#define NALU_TYPE_IDR               5
#define NALU_TYPE_SEI               6
#define NALU_TYPE_SPS               7
#define NALU_TYPE_PPS               8
#define NALU_TYPE_AUD               9

static inline int32_t operator - (const struct timeval & Time1, const struct timeval & Time2)
{
    int32_t Sec  = Time1.tv_sec  - Time2.tv_sec;
    int32_t USec = Time1.tv_usec - Time2.tv_usec;
    USec += Sec * 1000000;

    return USec;
}

H264RealTimeStreamFramerEx * H264RealTimeStreamFramerEx::createNew(UsageEnvironment & env, const SubStreamInfo & info) {
    H264RealTimeStreamFramerEx * newSource       = NULL;
    IPC_MediaDataClient        * mediaDataClient = NULL;
    uint16_t                     LocalPort       = Configure::GetInstance().GetVideoStreamLocalPort(info.s_StreamId);
    uint16_t                     RemotePort      = Configure::GetInstance().GetVideoStreamRemotePort(info.s_StreamId);


    do
    {
        mediaDataClient = new IPC_MediaDataClient;
        if (NULL == mediaDataClient) {
            break;
        }

        if (mediaDataClient->Initialize(LocalPort, Configure::GetInstance().GetStreamApplicationId()) != GMI_SUCCESS) {
            break;
        }

        if (mediaDataClient->Register(RemotePort, info.s_EncodeType, info.s_StreamId, true, NULL, NULL) != GMI_SUCCESS) {
            break;
        }

        newSource = new H264RealTimeStreamFramerEx(env, mediaDataClient, info);
        if (NULL == newSource) {
            break;
        }

        return newSource;
    } while (0);

    // Release resources when failed to create "source"
    if (mediaDataClient != NULL) {
        mediaDataClient->Unregister();
        mediaDataClient->Deinitialize();
    }

    return NULL;
}

H264RealTimeStreamFramerEx::H264RealTimeStreamFramerEx(UsageEnvironment & env, IPC_MediaDataClient * mediaDataClient, const SubStreamInfo & info)
    : H264VideoStreamFramer(env, NULL, False, False)
    , fMediaDataClient(mediaDataClient)
    , fSubStreamInfo(info)
    , fCurrentFrame()
    , fNeedNextFrame(True)
    , fCurrentNaluUnitIndex(0)
    , fOffset(0)
    , fNeedSPS(True)
    , fNeedPPS(True)
    , fFirstFrame(True)
    , fNeedReleaseFrame(False)
    , fFrameDuration(1000000 / info.s_FrameRate)
{
    fTimeStamp.tv_sec = fTimeStamp.tv_usec = 0;
}

H264RealTimeStreamFramerEx::~H264RealTimeStreamFramerEx() {
    if (fNeedReleaseFrame)
    {
        fMediaDataClient->ReleaseFrame();
    }

    fMediaDataClient->Unregister();
    fMediaDataClient->Deinitialize();

    envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void H264RealTimeStreamFramerEx::doGetNextFrame() {
    if (fNeedNextFrame) {
        uint8_t        * addr             = NULL;
        unsigned         size             = 0;
        uint8_t        * exData           = NULL;
        unsigned         exDataSize       = 0;
        struct timeval   presentationTime = {0, 0};

        GMI_RESULT RetVal = fMediaDataClient->GetFrame((const void **)&addr, &size, &presentationTime, (const void **)&exData, &exDataSize, 1);
        if (RetVal != GMI_SUCCESS) {
            // PRINT_LOG(WARNING, "RetVal = 0x%08lx", RetVal);
            nextTask() = envir().taskScheduler().scheduleDelayedTask(10000, (TaskFunc *)&tryGetNextFrame, this);
            return;
        }

        fNeedReleaseFrame = True;
        fNeedNextFrame = False;

        fCurrentFrame.Reset();
        fCurrentFrame.Assign(addr, size);
        fCurrentFrame.SetLength(size);
        fCurrentFrame.SetDuration(0);
        fCurrentFrame.Parse();

        fCurrentNaluUnitIndex = 0;
        fOffset = 1;

        if (!fFirstFrame) {
            fTimeStamp.tv_usec += fFrameDuration;
            if (fTimeStamp.tv_usec > 1000000)
            {
                fTimeStamp.tv_usec -= 1000000;
                fTimeStamp.tv_sec ++;
            }

            fCurrentFrame.SetTimeStamp(fTimeStamp);

        } else {
            fFirstFrame = false;

            gettimeofday(&fTimeStamp, NULL);
            fCurrentFrame.SetTimeStamp(fTimeStamp);

            nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc *)&tryGetNextFrame, this);
            return;
        }
    }

    const H264Frame::NaluUnit & Nalu     = fCurrentFrame.GetNaluUnit(fCurrentNaluUnitIndex);
    const uint8_t             * addr     = Nalu.s_Addr;
    uint32_t                    size     = Nalu.s_Length;
    uint8_t                     naluType = (addr[0] & 0x1F);

    if (naluType == NALU_TYPE_SPS && fNeedSPS) {
        // Save SPS(Sequence Parameter Set)
        saveCopyOfSPS((u_int8_t *)addr, size);
        fNeedSPS = False;
    } else if (naluType == NALU_TYPE_PPS && fNeedPPS) {
        // Save PPS(Picture Parameter Set)
        saveCopyOfPPS((u_int8_t *)addr, size);
        fNeedPPS = False;
    }

    if (size > fMaxSize) {
        fFrameSize = size - fOffset + 2;
        fFrameSize = (fFrameSize > fMaxSize) ? fMaxSize : fFrameSize;
        memcpy(fTo + 2, addr + fOffset, fFrameSize - 2);

        fTo[0] = (addr[0] & 0xE0) | 0x1C;
        if (fOffset == 1) {
            fTo[1] = (addr[0] & 0x1F) | 0x80;
        } else if (fOffset + fFrameSize - 2 >= size) {
            fTo[1] = (addr[0] & 0x1F) | 0x40;
            fCurrentNaluUnitIndex ++;
            fOffset = 1;
        } else {
            fTo[1] = (addr[0] & 0x1F);
        }

        fOffset += fFrameSize - 2;

    } else {
        fFrameSize = size;
        memcpy(fTo, addr, fFrameSize);
        fCurrentNaluUnitIndex ++;
    }

    fNumTruncatedBytes = 0;
    fPresentationTime = fCurrentFrame.TimeStamp();

    if (fCurrentNaluUnitIndex >= fCurrentFrame.NaluCount()) {
        fPictureEndMarker = True;
        fNeedNextFrame = True;
        fDurationInMicroseconds = fCurrentFrame.Duration();
    } else {
        fPictureEndMarker = False;
        fDurationInMicroseconds = 0;
    }

    afterGetting(this);

    if (fNeedNextFrame)
    {
        fMediaDataClient->ReleaseFrame();
        fNeedReleaseFrame = False;
    }
}

void H264RealTimeStreamFramerEx::doStopGettingFrames() {
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void H264RealTimeStreamFramerEx::tryGetNextFrame(H264RealTimeStreamFramerEx * source) {
    if (!source->isCurrentlyAwaitingData()) {
        source->doStopGettingFrames();
        return;
    }

    source->doGetNextFrame();
}

