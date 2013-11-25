#include "G711RealTimeStreamSource.hh"

G711RealTimeStreamSource * G711RealTimeStreamSource::createNew(UsageEnvironment & env, const SubStreamInfo & info) {
    G711RealTimeStreamSource * newSource    = NULL;
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

        newSource = new G711RealTimeStreamSource(env, frameCapture);
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

G711RealTimeStreamSource::G711RealTimeStreamSource(UsageEnvironment & env, MediaFrameCapture * frameCapture)
    : FramedSource(env)
    , fFrameCapture(frameCapture)
    , fCurrentFrame(NULL)
    , fFirstFrame(True)
{
}

G711RealTimeStreamSource::~G711RealTimeStreamSource() {
    fFrameCapture->RecycleLastFrame(fCurrentFrame);
    fFrameCapture->StopCapture();
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void G711RealTimeStreamSource::doGetNextFrame() {
    if (fFirstFrame) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc *)&tryGetNextFrame, this);
        fFirstFrame = False;
        PRINT_LOG(ASSERT, "First G711 frame");
        return;
    }

    MediaFrame * nextFrame = fFrameCapture->GetNextFrame(fCurrentFrame);
    if (NULL == nextFrame) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(10000, (TaskFunc *)&tryGetNextFrame, this);
        return;
    }

    // ASSERT(nextFrame->IsG711Frame(), "nextFrame MUST be G711Frame");

    fCurrentFrame = dynamic_cast<G711Frame *>(nextFrame);

    fFrameSize = (nextFrame->Length() > fMaxSize) ? fMaxSize : nextFrame->Length();
    fNumTruncatedBytes = nextFrame->Length() - fFrameSize;

    memmove(fTo, nextFrame->Addr(), fFrameSize);

    fPresentationTime = nextFrame->TimeStamp();
    fDurationInMicroseconds = nextFrame->Duration();

    afterGetting(this);
}

void G711RealTimeStreamSource::doStopGettingFrames() {
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
}

void G711RealTimeStreamSource::tryGetNextFrame(G711RealTimeStreamSource * source) {
    if (!source->isCurrentlyAwaitingData()) {
        source->doStopGettingFrames();
        return;
    }

    source->doGetNextFrame();
}

