#ifndef __GMI_GTP_BUFFER_H__
#define __GMI_GTP_BUFFER_H__

#include <string.h>

#include <gmi_type_definitions.h>
#include <gmi_errors.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct tagGtpBuffer
{
    uint8_t  * s_DataBuf;
    uint32_t   s_BufMaxLength;
    uint32_t   s_BufLength;
} GtpBuffer;

#define GtpBufferInit(p)          do { memset(p, 0x00, sizeof(GtpBuffer)); } while (0)
#define GtpBufferGetDataLength(p) ((p)->s_BufLength)
#define GtpBufferGetDataBuffer(p) ((p)->s_DataBuf)
#define GtpBufferClear(p)         do { (p)->s_BufLength = 0; } while (0)

void_t GtpBufferDestroy(GtpBuffer * GtpBuf);
GMI_RESULT GtpBufferPutData(GtpBuffer * GtpBuf, const uint8_t * Buf, uint32_t BufLength);

#ifdef __cplusplus
}
#endif

#endif // __GMI_GTP_BUFFER_H__
