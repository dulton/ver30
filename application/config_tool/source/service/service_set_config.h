#ifndef __SERVICE_SET_CONFIG_H__
#define __SERVICE_SET_CONFIG_H__

#include "service.h"
#include "configure_service.h"

class ServiceSetConfig : public IService, public IParser, protected CThread, public Singleton<ServiceSetConfig>
{
public:
    ServiceSetConfig();
    ~ServiceSetConfig();

    // Interface for service
    virtual void_t Execute(GtpTransHandle Handle, IParser *Parser);

    // Interface for parser
    virtual const char_t * Name() const;
    virtual boolean_t ParseXML(TiXmlDocument &XmlDoc);
    virtual boolean_t ParseBinary(const uint8_t *Buffer, uint32_t BufferLength);

private:
    // Interface for service
    virtual void_t Bind(GtpTransHandle Handle);
    virtual void_t Unbind(GtpTransHandle Handle);

    static void_t OnTimeProc(void_t * Data);

    void_t OnTime();

    // Interface for thread
    virtual GMI_RESULT ThreadEntry();

    // Current binding transaction
    GtpTransHandle m_TransHandle;

    // Result of parser
    NetworkInfo    m_NetworkInfo;
    DeviceInfo     m_DeviceInfo;
    boolean_t      m_UsingSupper;
};

#endif // __SERVICE_SET_CONFIG_H__

