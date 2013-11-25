#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <tinyxml.h>

#include <tool_protocol.h>

#include "common_def.h"

// Help to parse XML
inline TiXmlElement * GetXmlElement(TiXmlElement * Parent, const char_t * Name)
{
    TiXmlNode * XmlNode = Parent->FirstChild(Name);
    if (NULL == XmlNode)
    {
        PRINT_LOG(INFO, "No '%s' element", Name);
        return NULL;
    }

    if (XmlNode != Parent->LastChild(Name))
    {
        PRINT_LOG(WARNING, "More than one '%s' element", Name);
        return NULL;
    }

    return XmlNode->ToElement();
}

// Help to generate XML, you should use these two macro funcion in "do { ... } while (0)"
#define NEW_XML_ELEMENT(element, name, parent, retval) \
    element = new TiXmlElement(name); \
    if (NULL == element) { \
        PRINT_LOG(ERROR, "Failed to create element '%s'", name); \
        retval = GMI_OUT_OF_MEMORY; \
        break; \
    } \
    (parent)->LinkEndChild(element)

#define NEW_XML_TEXT(text, value, parent, retval) \
    text = new TiXmlText(value); \
    if (NULL == text) { \
        PRINT_LOG(ERROR, "Failed to create text '%s'", value); \
        retval = GMI_OUT_OF_MEMORY; \
        break; \
    } \
    (parent)->LinkEndChild(text)

#define XML_SIMPLE_RESPONSE "<Response operation=\"%s\" result=\"%d\"/>"

// Generate simple response and send it
inline GMI_RESULT SendSimpleResponse(GtpTransHandle Handle, const char_t * Operation, GMI_RESULT RetVal)
{
    static char_t XMLResponseString[128];
    snprintf(XMLResponseString, sizeof(XMLResponseString), XML_SIMPLE_RESPONSE, Operation, RetVal2Int(RetVal));
    return GtpTransSendData(Handle, reinterpret_cast<const uint8_t *>(XMLResponseString), strlen(XMLResponseString), GTP_DATA_TYPE_UTF8_XML);
}

// Define interface for parser
class IParser
{
public:
    virtual const char_t * Name() const = 0;
    virtual boolean_t ParseBinary(const uint8_t * Buffer, uint32_t BufferLength) = 0;
    virtual boolean_t ParseXML(TiXmlDocument & XmlDoc) = 0;
};

// Define interface for service
class IService
{
public:
    virtual void_t Execute(GtpTransHandle Handle, IParser * Parser) = 0;

    virtual void_t OnSent(GtpTransHandle Handle, GMI_RESULT Errno);
    virtual void_t OnRecv(GtpTransHandle Handle, const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type);
    virtual void_t OnTransactionKilled(GtpTransHandle Handle);

protected:
    virtual void_t Bind(GtpTransHandle Handle) = 0;
    virtual void_t Unbind(GtpTransHandle Handle) = 0;
};

#endif // __SERVICE_H__

