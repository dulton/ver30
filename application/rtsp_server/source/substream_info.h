#ifndef __SUBSTREAM_INFO_H__
#define __SUBSTREAM_INFO_H__

#include <gmi_media_ctrl.h>

#include <common_def.h>

/*
 * Media sub stream information
 */
typedef struct tagSubStreamInfo
{
    uint32_t s_EncodeType;
    uint32_t s_StreamId;
    uint32_t s_FrameRate;
    uint32_t s_MaxBitRate;
} SubStreamInfo;

/*
 * Helper function
 */
inline bool operator == (const SubStreamInfo & Info1, const SubStreamInfo & Info2)
{
    return memcmp(&Info1, &Info2, sizeof(SubStreamInfo)) == 0;
}

inline bool operator > (const SubStreamInfo & Info1, const SubStreamInfo & Info2)
{
    return memcmp(&Info1, &Info2, sizeof(SubStreamInfo)) > 0;
}

inline bool operator < (const SubStreamInfo & Info1, const SubStreamInfo & Info2)
{
    return memcmp(&Info1, &Info2, sizeof(SubStreamInfo)) < 0;
}

#endif // __SUBSTREAM_INFO_H__
