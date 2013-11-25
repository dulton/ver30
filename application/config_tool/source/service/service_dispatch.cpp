#include "service_dispatch.h"

// Dispatch all event from protocol layer
void_t ServiceDispatch::OnSentProc(void_t * Data, GtpTransHandle Handle, GMI_RESULT Errno)
{
    ASSERT_VALID_POINT(Data);
    ServiceDispatch * This = static_cast<ServiceDispatch *>(Data);
    This->Dispatch(Handle, Errno);
}

void_t ServiceDispatch::OnRecvProc(void_t * Data, GtpTransHandle Handle, const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type)
{
    ASSERT_VALID_POINT(Data);
    ServiceDispatch * This = static_cast<ServiceDispatch *>(Data);
    This->Dispatch(Handle, Buffer, BufferLength, Type);
}

void_t ServiceDispatch::OnTransactionKilledProc(void_t * Data, GtpTransHandle Handle)
{
    ASSERT_VALID_POINT(Data);
    ServiceDispatch * This = static_cast<ServiceDispatch *>(Data);
    This->Dispatch(Handle);
}

ServiceDispatch::ServiceDispatch()
    : m_ServiceTable()
{
    // Nothing to do
}

ServiceDispatch::~ServiceDispatch()
{
    // Nothing to do
}

GMI_RESULT ServiceDispatch::Register(IParser * Parser, IService * Service)
{
    if (NULL == Parser || NULL == Service)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    std::map<IParser *, IService *>::iterator it = m_ServiceTable.find(Parser);
    if (it != m_ServiceTable.end())
    {
        PRINT_LOG(ERROR, "%s is already registered", Parser->Name());
        return GMI_ALREADY_OPERATED;
    }

    m_ServiceTable[Parser] = Service;
    return GMI_SUCCESS;
}

GMI_RESULT ServiceDispatch::Unregister(IParser * Parser)
{
    if (NULL == Parser)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    std::map<IParser *, IService *>::iterator it = m_ServiceTable.find(Parser);
    if (it == m_ServiceTable.end())
    {
        PRINT_LOG(ERROR, "%s is not registered yet", Parser->Name());
        return GMI_INVALID_OPERATION;
    }

    m_ServiceTable.erase(it);
    return GMI_SUCCESS;
}

void_t ServiceDispatch::Dispatch(GtpTransHandle Handle, GMI_RESULT Errno)
{
    ASSERT_NOT_EQUAL(Handle, GTP_INVALID_HANDLE);
    IService * Service = static_cast<IService *>(GtpTransGetReferenceInstance(Handle));
    if (Service != NULL)
    {
        Service->OnSent(Handle, Errno);
        return;
    }

    PRINT_LOG(WARNING, "Failed to dispatch event");
    GtpDestroyTransHandle(Handle);
}

void_t ServiceDispatch::Dispatch(GtpTransHandle Handle, const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type)
{
    ASSERT_NOT_EQUAL(Handle, GTP_INVALID_HANDLE);
    ASSERT_VALID_POINT(Buffer);
    ASSERT_GREATER(BufferLength, 0);
    IService * Service = static_cast<IService *>(GtpTransGetReferenceInstance(Handle));
    if (Service != NULL)
    {
        Service->OnRecv(Handle, Buffer, BufferLength, Type);
        return;
    }

    std::map<IParser *, IService *>::iterator it;
    if (GTP_DATA_TYPE_UTF8_XML != Type)
    {
        for (it = m_ServiceTable.begin(); it != m_ServiceTable.end(); ++it) 
        {
            if (it->first->ParseBinary(Buffer, BufferLength))
            {
                it->second->Execute(Handle, it->first);
                return;
            }
        }

        PRINT_LOG(WARNING, "Failed to parse binary data, drop it");
        DUMP_BUFFER(Buffer, BufferLength);
    }
    else
    {
        std::string XmlString(reinterpret_cast<const char_t *>(Buffer), BufferLength);

        // Parse xml string
        TiXmlDocument XmlDoc;
        XmlDoc.Parse(XmlString.c_str());

        for (it = m_ServiceTable.begin(); it != m_ServiceTable.end(); ++it) 
        {
            if (it->first->ParseXML(XmlDoc))
            {
                it->second->Execute(Handle, it->first);
                return;
            }
        }

        PRINT_LOG(WARNING, "Failed to parse XML text, drop it");
        DUMP_VARIABLE(XmlString.c_str());
    }

    PRINT_LOG(WARNING, "Failed to dispatch event");
    GtpDestroyTransHandle(Handle);
}

void_t ServiceDispatch::Dispatch(GtpTransHandle Handle)
{
    ASSERT_NOT_EQUAL(Handle, GTP_INVALID_HANDLE);
    IService * Service = static_cast<IService *>(GtpTransGetReferenceInstance(Handle));
    if (Service != NULL)
    {
        Service->OnTransactionKilled(Handle);
        return;
    }

    PRINT_LOG(WARNING, "Failed to dispatch event");
    // Here is in destroy proc funcion, so we MUST NOT destroy same transaction again
    // GtpDestroyTransHandle(Handle);
}
