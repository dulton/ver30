#pragma once

#include "audio_handler_filter.h"

#define SAVE_G711A_AUDIO_DECODE_FILE 0

class G711A_AudioDecodeHandlerFilter : public AudioHandlerFilter
{
protected:
    G711A_AudioDecodeHandlerFilter(void);
    virtual ~G711A_AudioDecodeHandlerFilter(void);

public:
    static  G711A_AudioDecodeHandlerFilter*  CreateNew();
    friend class BaseMemoryManager;

    virtual GMI_RESULT	Initialize( int32_t FilterId, const char_t *FilterName, size_t FilterNameLength, void_t *Argument, size_t ArgumentSize );
    virtual GMI_RESULT  Deinitialize();
    virtual GMI_RESULT  Play();
    virtual GMI_RESULT  Stop();
    virtual GMI_RESULT  Receive( int32_t InputPinIndex, const uint8_t *Frame, size_t FrameLength, const struct timeval *FrameTS, const void_t *ExtraData, size_t ExtraDataLength );

    GMI_RESULT GetDecodeConfig( void_t *DecodeParameter, size_t *DecodeParameterLength );
    GMI_RESULT SetDecodeConfig( const void_t *DecodeParameter, size_t DecodeParameterLength );

private:
    FD_HANDLE  m_HardwareSource;
#if SAVE_G711A_AUDIO_DECODE_FILE
    FILE       *m_AudioFile;
#endif
};
