#ifndef __SERVICE_PUT_FILE_H__
#define __SERVICE_PUT_FILE_H__

#include <string>

#include "service.h"
#include "md5_engine.h"

class ServicePutFile : public IService, public IParser, protected CThread, public Singleton<ServicePutFile>
{
public:
    ServicePutFile();
    ~ServicePutFile();

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
        eSendingACK,
        eReceivingFile,
        eProcessingFile,
        eSendingResponse,
    } State;

    inline boolean_t StateIdle() const { return m_State == eIdle; }
    inline boolean_t StateSendingACK() const { return m_State == eSendingACK; }
    inline boolean_t StateReceivingFile() const { return m_State == eReceivingFile; }
    inline boolean_t StateProcessingFile() const { return m_State == eProcessingFile; }
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

    // Current binding transaction
    GtpTransHandle   m_TransHandle;

    // Result of parser
    std::string      m_FilePath;
    std::string      m_MD5Value;
    boolean_t        m_Compressed;
    boolean_t        m_Encrypted;
    boolean_t        m_NeedReserved;
    boolean_t        m_NeedReboot;
    boolean_t        m_UsingSupper;

    State            m_State;
    FILE           * m_File;
    MD5Engine        m_MD5Engine;
};

#endif // __SERVICE_PUT_FILE_H__

