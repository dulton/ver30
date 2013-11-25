#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#include "common_def.h"

class Configure : public Instance<Configure>
{
friend class Instance<Configure>;

public:
    uint16_t GetVideoStreamLocalPort(uint32_t StreamId) const;
    uint16_t GetVideoStreamRemotePort(uint32_t StreamId) const;
    uint16_t GetAudioStreamLocalPort(uint32_t StreamId) const;
    uint16_t GetAudioStreamRemotePort(uint32_t StreamId) const;

    uint32_t GetVideoStreamCount() const;
    uint32_t GetAudioStreamCount() const;

    uint32_t GetStreamApplicationId() const;
    uint16_t GetRtspServicePort() const;

private:
    Configure();
    ~Configure();
};

#endif // __CONFIGURE_H__

