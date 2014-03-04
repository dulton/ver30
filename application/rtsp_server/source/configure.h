#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#include "common_def.h"

class Configure : public Instance<Configure>
{
friend class Instance<Configure>;

public:
    GMI_RESULT Initialize();
    GMI_RESULT Uninitialize();

    inline boolean_t Initialized() const { return m_Initialized; }

    uint16_t GetVideoStreamLocalPort(uint32_t StreamId) const;
    uint16_t GetVideoStreamRemotePort(uint32_t StreamId) const;
    uint16_t GetAudioStreamLocalPort(uint32_t StreamId) const;
    uint16_t GetAudioStreamRemotePort(uint32_t StreamId) const;

    uint32_t GetVideoStreamCount() const;
    uint32_t GetAudioStreamCount() const;

    uint32_t GetStreamApplicationId() const;
    uint16_t GetRtspServicePort() const;

    uint16_t GetAuthLocalPort() const;
    uint8_t GetAuthLocalModuleId() const;

private:
    Configure();
    ~Configure();

    boolean_t m_Initialized;

    // User information
    uint32_t  m_AuthValue;
    uint16_t  m_SessionId;
    uint8_t   m_UserFlag;
};

#endif // __CONFIGURE_H__

