#include <stdlib.h>

#include "gtp_buffer.h"

#include "debug.h"

void_t GtpBufferDestroy(GtpBuffer * GtpBuf)
{
    ASSERT(GtpBuf != NULL, "GtpBuf MUST NOT be non-pointer");
    if (GtpBuf->s_DataBuf != NULL)
    {
        free(GtpBuf->s_DataBuf);
    }

    GtpBufferInit(GtpBuf);
}

GMI_RESULT GtpBufferPutData(GtpBuffer * GtpBuf, const uint8_t * Buf, uint32_t BufLength)
{
    uint32_t DataLength = 0;

    ASSERT(GtpBuf != NULL, "GtpBuf MUST NOT be non-pointer");

    DataLength = GtpBuf->s_BufLength + BufLength;

    if (DataLength > GtpBuf->s_BufMaxLength)
    {
        if (GtpBuf->s_DataBuf)
        {
            GtpBuf->s_DataBuf = (uint8_t *)realloc(GtpBuf->s_DataBuf, DataLength);
        }
        else
        {
            GtpBuf->s_DataBuf = (uint8_t *)malloc(DataLength);
        }

        if (NULL == GtpBuf->s_DataBuf)
        {
            PRINT_LOG(ERROR, "Not enough memory");
            return GMI_OUT_OF_MEMORY;
        }

        GtpBuf->s_BufMaxLength = DataLength;
    }

    memcpy(GtpBuf->s_DataBuf + GtpBuf->s_BufLength, Buf, BufLength);
    GtpBuf->s_BufLength = DataLength;

    return GMI_SUCCESS;
}

