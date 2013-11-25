#ifndef __GMI_MD5_ENGINE_H__
#define __GMI_MD5_ENGINE_H__

#include "common_def.h"

class MD5Engine
{
public:
    MD5Engine();

    void_t Reset();
    void_t Update(const uint8_t * Buffer, uint32_t Length);
    void_t Finalize();
    void_t HexDigest(char_t * Buffer, uint32_t Length) const;

private:
    void_t Transform(const uint8_t block[64]);

    uint32_t  m_State[4];
    uint32_t  m_Count[2];
    uint8_t   m_Buffer[64];
    uint8_t   m_Digest[16];
    boolean_t m_Finalized;
};

#endif // __GMI_MD5_ENGINE_H__

