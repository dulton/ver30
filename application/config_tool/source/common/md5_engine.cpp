#include <string.h>

#include "md5_engine.h"

/*
 * ========================== MD5 Calculate Engine ==========================
 */

// Encodes input (uint32_t) into output (uint8_t). Assumes len is
// a multiple of 4.
inline static void_t MD5Encode(uint8_t * Output, const uint32_t * Input, uint32_t Length)
{
    uint32_t i, j;
    for (i = 0, j = 0; j < Length; i++, j += 4)
    {
        Output[j]   = (uint8_t)( Input[i]        & 0xff);
        Output[j+1] = (uint8_t)((Input[i] >> 8)  & 0xff);
        Output[j+2] = (uint8_t)((Input[i] >> 16) & 0xff);
        Output[j+3] = (uint8_t)((Input[i] >> 24) & 0xff);
    }
}

// Decodes input (uint8_t) into output (uint32_t). Assumes len is
// a multiple of 4.
inline static void_t MD5Decode(uint32_t * Output, const uint8_t * Input, uint32_t Length)
{
    uint32_t i, j;
    for (i = 0, j = 0; j < Length; i++, j += 4)
    {
        Output[i] = ((uint32_t)Input[j])            |
                   (((uint32_t)Input[j + 1]) << 8)  |
                   (((uint32_t)Input[j + 2]) << 16) |
                   (((uint32_t)Input[j + 3]) << 24);
    }
}

// ROTATE_LEFT rotates x left n bits.
inline static uint32_t RotateLeft(uint32_t x, uint32_t n)
{
    return (x << n) | (x >> (32 - n));
}

// F, G, H and I are basic MD5 functions.
inline static uint32_t F(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) | (~x & z);
}

inline static uint32_t G(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & z) | (y & ~z);
}

inline static uint32_t H(uint32_t x, uint32_t y, uint32_t z)
{
    return x ^ y ^ z;
}

inline static uint32_t I(uint32_t x, uint32_t y, uint32_t z)
{
    return y ^ (x | ~z);
}

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
inline static uint32_t FF(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
{
    a += F(b, c, d) + x + ac;
    return RotateLeft(a, s) + b;
}

inline static uint32_t GG(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
{
    a += G(b, c, d) + x + ac;
    return RotateLeft(a, s) + b;
}

inline static uint32_t HH(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
{
    a += H(b, c, d) + x + ac;
    return RotateLeft(a, s) + b;
}

inline static uint32_t II(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac)
{
    a += I(b, c, d) + x + ac;
    return RotateLeft(a, s) + b;
}

// Constants for MD5Transform routine.
// Although we could use C++ style constants, defines are actually better,
// since they let us easily evade scope clashes.
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

// MD5 basic transformation. Transforms state based on block.
void_t MD5Engine::Transform(const uint8_t block[64])
{
    uint32_t a = m_State[0];
    uint32_t b = m_State[1];
    uint32_t c = m_State[2];
    uint32_t d = m_State[3];
    uint32_t x[16];

    ASSERT(!m_Finalized, "MD5 engine MUST NOT be finalized");

    // Zeroize sensitive information.
    memset (x, 0x00, sizeof(x));

    MD5Decode(x, block, 64);

    /* Round 1 */
    a = FF(a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    d = FF(d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    c = FF(c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    b = FF(b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    a = FF(a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    d = FF(d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    c = FF(c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    b = FF(b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    a = FF(a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    d = FF(d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    c = FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    b = FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    a = FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    d = FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    c = FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    b = FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /* Round 2 */
    a = GG(a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    d = GG(d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    c = GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    b = GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    a = GG(a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    d = GG(d, a, b, c, x[10], S22, 0x02441453); /* 22 */
    c = GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    b = GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    a = GG(a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    d = GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    c = GG(c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    b = GG(b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    a = GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    d = GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    c = GG(c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    b = GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    a = HH(a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    d = HH(d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    c = HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    b = HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    a = HH(a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    d = HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    c = HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    b = HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    a = HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    d = HH(d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    c = HH(c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    b = HH(b, c, d, a, x[ 6], S34, 0x04881d05); /* 44 */
    a = HH(a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    d = HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    c = HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    b = HH(b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
    a = II(a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    d = II(d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    c = II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    b = II(b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    a = II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    d = II(d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    c = II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    b = II(b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    a = II(a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    d = II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    c = II(c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    b = II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    a = II(a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    d = II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    c = II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    b = II(b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    m_State[0] += a;
    m_State[1] += b;
    m_State[2] += c;
    m_State[3] += d;
}

MD5Engine::MD5Engine()
    : m_Finalized(false)
{
    Reset();
}

void_t MD5Engine::Reset()
{
    m_Finalized = false;

    // Nothing counted, so count = 0
    m_Count[0] = 0;
    m_Count[1] = 0;

    // Load magic initialization constants.
    m_State[0] = 0x67452301;
    m_State[1] = 0xefcdab89;
    m_State[2] = 0x98badcfe;
    m_State[3] = 0x10325476;

    memset(m_Buffer, 0x00, sizeof(m_Buffer));
    memset(m_Digest, 0x00, sizeof(m_Digest));
}

// MD5 block update operation. Continues an MD5 message-digest
// operation, processing another message block, and updating the
// context.
void_t MD5Engine::Update(const uint8_t * Buffer, uint32_t Length)
{
    uint32_t InputIndex  = 0;
    uint32_t BufferIndex = 0;
    uint32_t BufferSpace = 0; // How much space is left in buffer

    if (NULL == Buffer || 0 == Length)
    {
        PRINT_LOG(ERROR, "Bad parameter ...");
        return;
    }

    if (m_Finalized)
    {
        PRINT_LOG(ERROR, "MD5 engine is already finalized ...");
        return;
    }

    // Compute number of bytes mod 64
    BufferIndex = (unsigned int)((m_Count[0] >> 3) & 0x3F);

    // Update number of bits
    if ((m_Count[0] += ((uint32_t)Length << 3)) < ((uint32_t)Length << 3))
    {
        m_Count[1] ++;
    }

    m_Count[1] += ((uint32_t)Length >> 29);

    BufferSpace = 64 - BufferIndex;  // how much space is left in buffer

    // Transform as many times as possible.
    if (Length >= BufferSpace)
    { 
        // ie. We have enough to fill the buffer
        // fill the rest of the buffer and transform
        memcpy(m_Buffer + BufferIndex, Buffer, BufferSpace);
        Transform(m_Buffer);

        // now, transform each 64-byte piece of the input, bypassing the buffer
        for (InputIndex = BufferSpace; InputIndex + 63 < Length; InputIndex += 64)
        {
            Transform((uint8_t *)Buffer + InputIndex);
        }

        BufferIndex = 0;  // So we can buffer remaining
    }
    else
    {
        InputIndex = 0; // So we can buffer the whole input
    }

    // And here we do the buffering:
    memcpy(m_Buffer + BufferIndex, Buffer + InputIndex, Length - InputIndex);
}

// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context.
void_t MD5Engine::Finalize()
{
    static const uint8_t PADDING[64] = {
        0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    uint8_t  Bits[8];
    uint32_t Index  = 0;
    uint32_t PadLen = 0;

    if (m_Finalized)
    {
        PRINT_LOG(ERROR, "MD5 engine is already finalized ...");
        return;
    }

    memset(Bits, 0x00, sizeof(Bits));

    // Save number of bits
    MD5Encode(Bits, m_Count, 8);

    // Pad out to 56 mod 64.
    Index = (uint32_t)((m_Count[0] >> 3) & 0x3f);
    PadLen = (Index < 56) ? (56 - Index) : (120 - Index);
    Update(PADDING, PadLen);

    // Append length (before padding)
    Update(Bits, 8);

    // Store state in digest
    MD5Encode(m_Digest, m_State, 16);

    // Zeroize sensitive information
    memset(m_Buffer, 0x00, sizeof(m_Buffer));

    m_Finalized = true;
}

void_t MD5Engine::HexDigest(char_t * Buffer, uint32_t Length) const
{
    int i = 0;

    if (NULL == Buffer || Length < 33)
    {
        PRINT_LOG(ERROR, "Bad parameter ...");
        return;
    }

    if (!m_Finalized)
    {
        PRINT_LOG(ERROR, "MD5 engine is not finalized ...");
        return;
    }

    for (i = 0; i < 16; i ++)
    {
        sprintf(Buffer + (i * 2), "%02x", m_Digest[i]);
    }

    Buffer[32]='\0';
}

