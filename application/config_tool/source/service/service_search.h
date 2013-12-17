#ifndef __SERVICE_SEARCH_H__
#define __SERVICE_SEARCH_H__

#include <string>
#include <vector>

#include "service.h"

class ServiceSearch : public IService, public IParser, protected CThread, public Singleton<ServiceSearch>
{
public:
    ServiceSearch();
    ~ServiceSearch();

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

    boolean_t NeedUpdate() const;

    // Binding transactions
    std::vector<GtpTransHandle> m_TransList;

    // Last update time of response string
    struct timespec             m_LastUpdateTime;

    // Response string
    std::string                 m_ResponseString;
};

#endif // __SERVICE_SEARCH_H__

