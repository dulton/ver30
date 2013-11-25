#ifndef __SERVICE_GET_FILE_H__
#define __SERVICE_GET_FILE_H__

#include <vector>
#include <string>

#include "service.h"
#include "md5_engine.h"

class ServiceGetFile : public IService, public IParser, protected CThread, public Singleton<ServiceGetFile>
{
public:
    ServiceGetFile();
    ~ServiceGetFile();

    // Interface for service
    virtual void_t Execute(GtpTransHandle Handle, IParser *Parser);

    virtual void_t OnSent(GtpTransHandle Handle, GMI_RESULT Errno);
    virtual void_t OnRecv(GtpTransHandle Handle, const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type);

    // Interface for parser
    virtual const char_t * Name() const;
    virtual boolean_t ParseXML(TiXmlDocument &XmlDoc);
    virtual boolean_t ParseBinary(const uint8_t *Buffer, uint32_t BufferLength);

private:
    typedef enum tagState
    {
        eIdle = 0,
        ePackingFile,
        eSendingACK,
        eReceivingACK,
        eSendingFile,
        eSendingResponse,
    } State;

    inline boolean_t StateIdle() const { return m_State == eIdle; }
    inline boolean_t StatePackingFile() const { return m_State == ePackingFile; }
    inline boolean_t StateSendingACK() const { return m_State == eSendingACK; }
    inline boolean_t StateReceivingACK() const { return m_State == eReceivingACK; }
    inline boolean_t StateSendingFile() const { return m_State == eSendingFile; }
    inline boolean_t StateSendingResponse() const { return m_State == eSendingResponse; }

    inline void_t SwitchState(State NewState) { m_State = NewState; }

    // Interface for service
    virtual void_t Bind(GtpTransHandle Handle);
    virtual void_t Unbind(GtpTransHandle Handle);

    static void_t OnTimeProc(void_t * Data);

    void_t OnTime();

    // Interface for thread
    virtual GMI_RESULT ThreadEntry();

    boolean_t ParseACK(const uint8_t * Buffer, uint32_t BufferLength);

    GMI_RESULT SendNextPacket();

    // Current binding transaction
    GtpTransHandle             m_TransHandle;

    // Result of parser
    std::vector<std::string>   m_FileList;
    boolean_t                  m_UsingSupper;

    State                      m_State;
    FILE                     * m_File;
    MD5Engine                  m_MD5Engine;
    boolean_t                  m_Compressed;
    boolean_t                  m_Encrypted;
};

#endif // __SERVICE_GET_FILE_H__


