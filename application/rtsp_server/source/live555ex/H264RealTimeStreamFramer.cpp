#include "H264RealTimeStreamFramer.hh"

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

// UDP max packet size
#define MAX_BYTES_PER_UDP_PACKET    1448

// RTP header size
#define RTP_HEADER_SIZE             12

H264RealTimeStreamFramer * H264RealTimeStreamFramer::createNew(UsageEnvironment & env,  const SubStreamInfo & info) {
    H264RealTimeStreamFramer * newSource    = NULL;
    MediaFrameCapture        * frameCapture = NULL;

    do
    {
        frameCapture = MediaFrameCapture::GetMediaFrameCapture(info);
        if (NULL == frameCapture) {
            break;
        }

        if (frameCapture->StartCapture() != GMI_SUCCESS) {
            break;
        }

        newSource = new H264RealTimeStreamFramer(env, frameCapture);
        if (NULL == newSource) {
            break;
        }

        return newSource;
    } while (0);

    // Release resources when failed to create "source"
    if (frameCapture != NULL) {
        frameCapture->StopCapture();
    }

    return NULL;
}

H264RealTimeStreamFramer::H264RealTimeStreamFramer(UsageEnvironment & env, MediaFrameCapture * frameCapture)
    : H264VideoStreamFramer(env, NULL, False, False)
    , fFrameCapture(frameCapture)
    , fCurrentFrame(NULL)
    , fNeedNextFrame(True)
    , fCurrentNaluUnitIndex(0)
    , fOffset(0)
    , fNeedSPS(True)
    , fNeedPPS(True)
    , fFirstFrame(True)
{
}

H264RealTimeStreamFramer::~H264RealTimeStreamFramer() {
    fFrameCapture->RecycleLastFrame(fCurrentFrame);
    fFrameCapture->StopCapture();
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void H264RealTimeStreamFramer::doGetNextFrame() {
    struct timespec TimeSpec = {0, 0};

    if (fNeedNextFrame) {
        if (fFirstFrame) {
            nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc *)&tryGetNextFrame, this);
            fFirstFrame = False;
            PRINT_LOG(ASSERT, "First H264 frame");
            return;
        }

        MediaFrame * nextFrame = fFrameCapture->GetNextFrame(fCurrentFrame);
        if (NULL == nextFrame) {
            nextTask() = envir().taskScheduler().scheduleDelayedTask(10000, (TaskFunc *)&tryGetNextFrame, this);
            return;
        }

        // ASSERT(nextFrame->IsH264Frame(), "nextFrame MUST be H264Frame");

        PRINT_LOG(ASSERT, "Get %c frame, size = %u", nextFrame->IsKeyFrame() ? 'I' : 'P', nextFrame->Length());

        clock_gettime(CLOCK_MONOTONIC, &TimeSpec);
        PRINT_LOG(VERBOSE, "Time to get frame :  %12ld.%9ld", TimeSpec.tv_sec, TimeSpec.tv_nsec);

        fCurrentFrame = dynamic_cast<H264Frame *>(nextFrame);
        fNeedNextFrame = False;
        fCurrentNaluUnitIndex = 0;
        fOffset = 1;

        // ASSERT(fCurrentFrame->NaluCount() > 0, "H264 frame MUST have at least 1 nalu unit");
    }

    const H264Frame::NaluUnit & Nalu     = fCurrentFrame->GetNaluUnit(fCurrentNaluUnitIndex);
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

#ifdef USE_H264_VIDEO_RTP_SINK

    fFrameSize = (size > fMaxSize) ? fMaxSize : size;
    fNumTruncatedBytes = size - fFrameSize;

    memmove(fTo, addr, fFrameSize);

    fPresentationTime = fCurrentFrame->TimeStamp();

    fCurrentNaluUnitIndex ++;

#else // USE_H264_VIDEO_RTP_SINK

    // Make sure max size of the data MUST NOT be greater then (MAX_BYTES_PER_UDP_PACKET - RTP_HEADER_SIZE)
    if (fMaxSize > MAX_BYTES_PER_UDP_PACKET - RTP_HEADER_SIZE) {
        fMaxSize = MAX_BYTES_PER_UDP_PACKET - RTP_HEADER_SIZE;
    }

    if (size > fMaxSize) {
        fFrameSize = size - fOffset + 2;
        fFrameSize = (fFrameSize > fMaxSize) ? fMaxSize : fFrameSize;
        memmove(fTo + 2, addr + fOffset, fFrameSize - 2);

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
        memmove(fTo, addr, fFrameSize);
        fCurrentNaluUnitIndex ++;
    }

    fPresentationTime = fCurrentFrame->TimeStamp();
    fNumTruncatedBytes = 0;

#endif // USE_H264_VIDEO_RTP_SINK

    if (fCurrentNaluUnitIndex >= fCurrentFrame->NaluCount()) {
        fPictureEndMarker = True;
        fNeedNextFrame = True;
        fDurationInMicroseconds = fCurrentFrame->Duration();
    } else {
        fPictureEndMarker = False;
        fDurationInMicroseconds = 0;
    }

    afterGetting(this);

    if (fNeedNextFrame)
    {
        clock_gettime(CLOCK_MONOTONIC, &TimeSpec);
        PRINT_LOG(VERBOSE, "Time to sent frame : %12ld.%9ld", TimeSpec.tv_sec, TimeSpec.tv_nsec);
    }
}

void H264RealTimeStreamFramer::doStopGettingFrames() {
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void H264RealTimeStreamFramer::tryGetNextFrame(H264RealTimeStreamFramer * source) {
    if (!source->isCurrentlyAwaitingData()) {
        source->doStopGettingFrames();
        return;
    }

    source->doGetNextFrame();
}

