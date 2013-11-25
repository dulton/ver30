#ifndef __MEDIA_CAPTURE_H__
#define __MEDIA_CAPTURE_H__

#include <map>
#include <vector>

#include "common_def.h"
#include "substream_info.h"

/*
 * Media frame class
 */
class MediaFrame : public Entry
{
protected:
    MediaFrame() : m_RefCount(0), m_Addr(NULL), m_MaxLength(0), m_Length(0), m_Duration(0) {}
    virtual ~MediaFrame() {}

public:
    // Reference count of this frame
    inline uint32_t AddRef() { return ++ m_RefCount; }
    inline uint32_t Release() { ASSERT(m_RefCount > 0, "m_RefCount MUST greater than 0"); return -- m_RefCount; }
    inline uint32_t RefCount() const { return m_RefCount; }

    // Fill the data of this frame
    inline void_t Assign(uint8_t * Addr, uint32_t MaxLength) { m_Addr = Addr; m_MaxLength = MaxLength; }
    inline void_t SetLength(const uint32_t Length) { m_Length = Length; }
    inline void_t SetTimeStamp(const struct timeval TimeStamp) { m_TimeStamp = TimeStamp; }
    inline void_t SetDuration(const uint32_t Duration) { m_Duration = Duration; }

    // Get the data of this frame
    inline uint8_t * Addr() const { return m_Addr; }
    inline uint32_t MaxLength() const { return m_MaxLength; }
    inline uint32_t Length() const { return m_Length; }
    inline const struct timeval & TimeStamp() const { return m_TimeStamp; }
    inline uint32_t Duration() const { return m_Duration; }

    // Reset this frame
    virtual void_t Reset() { m_Length = 0; }

    // Check if this frame is empty
    inline boolean_t Empty() const { return 0 == m_Length; }

    // Check this frame type
    virtual boolean_t IsH264Frame() const { return false; }
    virtual boolean_t IsG711Frame() const { return false; }

    // Parse the data buffer of this frame
    virtual GMI_RESULT Parse() { return GMI_SUCCESS; }

    // Check if this frame is key frame
    virtual boolean_t IsKeyFrame() const { return true; }

private:
    uint32_t         m_RefCount;
    uint8_t        * m_Addr;
    uint32_t         m_MaxLength;
    uint32_t         m_Length;
    struct timeval   m_TimeStamp;
    uint32_t         m_Duration;
};

/*
 * G711 frame class
 */
class G711Frame : public MediaFrame
{
public:
    G711Frame() : MediaFrame() {}
    virtual ~G711Frame() {}

    virtual boolean_t IsG711Frame() const { return true; }
};

/*
 * H264 frame class
 */
class H264Frame : public MediaFrame
{
public:
    typedef struct tagNaluUnit
    {
        uint8_t  * s_Addr;
        uint32_t   s_Length;
    } NaluUnit;

    H264Frame() : MediaFrame(), m_TotalNalu(0), m_KeyFrame(false) { memset(m_NaluUnits, 0x00, sizeof(m_NaluUnits)); }
    virtual ~H264Frame() {}

    virtual void_t Reset()
    {
        m_TotalNalu = 0;
        m_KeyFrame = false;
        memset(m_NaluUnits, 0x00, sizeof(m_NaluUnits));
        MediaFrame::Reset();
    }

    virtual boolean_t IsH264Frame() const { return true; }

    virtual GMI_RESULT Parse();

    virtual boolean_t IsKeyFrame() const { return m_KeyFrame; }

    // Get nalu units of this H264 frame
    inline uint32_t NaluCount() const { return m_TotalNalu; }
    inline const NaluUnit & GetNaluUnit(uint32_t Index) const { return m_NaluUnits[Index]; }

private:
    NaluUnit  m_NaluUnits[10];
    uint32_t  m_TotalNalu;
    boolean_t m_KeyFrame;
};

/*
 * Meida Frame Capture
 */
class MediaFrameCapture
{
public:
    static MediaFrameCapture * GetMediaFrameCapture(const SubStreamInfo & Info);
    static void_t DestroyAllMediaFrameCaptures();

protected:
    static std::map<SubStreamInfo, MediaFrameCapture *> ms_MediaFrameCaptureTable;

public:
    virtual ~MediaFrameCapture();

    inline boolean_t IsCapturing() const { return m_CaptureCount > 0; }

    GMI_RESULT StartCapture();
    GMI_RESULT StopCapture();

    virtual MediaFrame * GetNextFrame(MediaFrame * PrevFrame) = 0;
    virtual void_t RecycleLastFrame(MediaFrame * LastFrame) = 0;

protected:
    MediaFrameCapture(const SubStreamInfo & Info);

    virtual GMI_RESULT StartCaptureImpl() = 0;
    virtual GMI_RESULT StopCaptureImpl() = 0;

    uint32_t      m_CaptureCount;
    SubStreamInfo m_SubStreamInfo;
    uint32_t      m_FrameDuration;
    uint32_t      m_Sleep;
};

#endif // __MEDIA_CAPTURE_H__

