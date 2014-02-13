#include "service_set_config.h"
#include "service_dispatch.h"
#include "application.h"

// Create service
static ServiceSetConfig l_ServiceSetConfigInstance;

template <>
ServiceSetConfig * Singleton<ServiceSetConfig>::ms_InstancePtr = NULL;

ServiceSetConfig::ServiceSetConfig()
    : m_TransHandle(GTP_INVALID_HANDLE)
    , m_NetworkInfo()
    , m_DeviceInfo()
    , m_UsingSupper(false)
{
    ServiceDispatch::GetInstance().Register(this, this);
}

ServiceSetConfig::~ServiceSetConfig()
{
    ServiceDispatch::GetInstance().Unregister(this);

    // Recycle resources of thread, if thread is still running
    Wait();
}

void_t ServiceSetConfig::Execute(GtpTransHandle Handle, IParser * Parser)
{
    // Ignore parser, because parser is itself

    GMI_RESULT RetVal = GMI_SUCCESS;

    if (IsRunning())
    {
        PRINT_LOG(WARNING, "Service is busy now");

        // Send error response
        RetVal = SendSimpleResponse(Handle, "SetConfig", GMI_SYSTEM_BUSY);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to send response");
            GtpDestroyTransHandle(Handle);
        }

        return;
    }

    do
    {
        RetVal = Application::GetSingleton().AddDelayTask(ServiceSetConfig::OnTimeProc, this, 0, 100000);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to add delay task");
            break;
        }

        RetVal = Create();
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to create thread");
            break;
        }

        // Bind transaction to this service
        Bind(Handle);
        return;
    } while (0);

    // Cancel task
    Application::GetSingleton().CancelDelayTask(ServiceSetConfig::OnTimeProc, this);

    // Send error response
    RetVal = SendSimpleResponse(Handle, "SetConfig", RetVal);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send response");
        GtpDestroyTransHandle(Handle);
    }
}

const char_t * ServiceSetConfig::Name() const
{
    return "SetConfig";
}

boolean_t ServiceSetConfig::ParseBinary(const uint8_t * Buffer, uint32_t BufferLength)
{
    // We could not parse binary data
    return false;
}

boolean_t ServiceSetConfig::ParseXML(TiXmlDocument & XmlDoc)
{
    // Check if root element is existed
    TiXmlElement * XmlRootElement = XmlDoc.RootElement();
    if (NULL == XmlRootElement)
    {
        PRINT_LOG(WARNING, "No root element");
        return false;
    }

    const char_t * Tag = XmlRootElement->Value();
    if (NULL == Tag || strcmp(Tag, "Request") != 0)
    {
        PRINT_LOG(VERBOSE, "Not a request");
        return false;
    }

    const char_t * Operation = XmlRootElement->Attribute("operation");
    if (NULL == Operation || strcmp(Operation, "SetConfig") != 0)
    {
        PRINT_LOG(VERBOSE, "Not a set configure request");
        return false;
    }

    PRINT_LOG(VERBOSE, "It is a set configure request");

    if (IsRunning())
    {
        PRINT_LOG(VERBOSE, "Service is busy now, drop the data");
        return true;
    }

    PRINT_LOG(VERBOSE, "Parsing the configure in XML ...");

    const char * ToolIdString = XmlRootElement->Attribute("toolid");
    if (ToolIdString != NULL && strcmp(ToolIdString, "product") == 0)
    {
        m_UsingSupper = true;
    }
    else
    {
        m_UsingSupper = false;
    }

   // Get device information
    TiXmlElement * XmlDevInfo = GetXmlElement(XmlRootElement, "DeviceInfo");
    if (XmlDevInfo != NULL)
    {
        TiXmlElement * XmlDevName = GetXmlElement(XmlDevInfo, "Name");
        if (XmlDevName != NULL && XmlDevName->GetText() != NULL)
        {
            // Get device name
            m_DeviceInfo.SetName(XmlDevName->GetText());
        }

        TiXmlElement * XmlSN = GetXmlElement(XmlDevInfo, "SN");
        if (XmlSN != NULL && XmlDevName->GetText() != NULL)
        {
            // Get serial number
            m_DeviceInfo.SetSerialNumber(XmlSN->GetText());
        }
    }

    // Get network information
    TiXmlElement * XmlNetInfo = GetXmlElement(XmlRootElement, "NetworkInfo");
    if (XmlNetInfo != NULL)
    {
        TiXmlElement * XmlIfaceList = GetXmlElement(XmlNetInfo, "InterfaceList");
        if (XmlIfaceList != NULL)
        {
            for (TiXmlNode * XmlNode = XmlIfaceList->FirstChild("Interface"); XmlNode != NULL; XmlNode = XmlIfaceList->IterateChildren("Interface", XmlNode))
            {
                TiXmlElement * XmlIface = XmlNode->ToElement();
                if (XmlIface != NULL)
                {
                    // Get interface name
                    const char_t  * IfaceName = XmlIface->Attribute("name");
                    if (NULL == IfaceName)
                    {
                        continue;
                    }

                    InterfaceInfo Iface(IfaceName);

                    // Check if interface should using dhcp
                    int32_t IsDHCP = 0;
                    if (XmlIface->QueryIntAttribute("dhcp", &IsDHCP) != TIXML_SUCCESS)
                    {
                        IsDHCP = 0;
                    }

                    Iface.SetDHCPStatus(IsDHCP == 1);

                    // Check if interface should be enabled
                    int32_t IsEnabled = 1;
                    if (XmlIface->QueryIntAttribute("enable", &IsDHCP) != TIXML_SUCCESS)
                    {
                        IsEnabled = 1;
                    }

                    Iface.SetEnabled(IsEnabled == 1);

                    // Get ip address
                    TiXmlElement * XmlElement = GetXmlElement(XmlIface, "Address");
                    if (NULL == XmlElement || NULL == XmlElement->GetText())
                    {
                        PRINT_LOG(WARNING, "Failed to parse ip address");
                        continue;
                    }

                    Iface.SetIpAddress(XmlElement->GetText());

                    // Get netmask
                    XmlElement = GetXmlElement(XmlIface, "Netmask");
                    if (NULL == XmlElement || NULL == XmlElement->GetText())
                    {
                        PRINT_LOG(WARNING, "Failed to parse netmask");
                        continue;
                    }

                    Iface.SetNetmask(XmlElement->GetText());

                    // Get gateway
                    XmlElement = GetXmlElement(XmlIface, "Gateway");
                    if (XmlElement != NULL && XmlElement->GetText() != NULL)
                    {
                        Iface.SetGateway(XmlElement->GetText());
                    }

                    // Get MAC address
                    XmlElement = GetXmlElement(XmlIface, "MAC");
                    if (XmlElement != NULL && XmlElement->GetText() != NULL)
                    {
                        Iface.SetMacAddress(XmlElement->GetText());
                    }

                    // Add interface to list
                    m_NetworkInfo.AddInterface(Iface);
                }
            }
        }

        TiXmlElement * XmlDNSList = GetXmlElement(XmlNetInfo, "DNSList");
        if (XmlDNSList != NULL)
        {
            for (TiXmlNode * XmlNode = XmlDNSList->FirstChild("DNS"); XmlNode != NULL; XmlNode = XmlDNSList->IterateChildren("DNS", XmlNode))
            {
                TiXmlElement * XmlDNS = XmlNode->ToElement();
                if (XmlDNS != NULL && XmlDNS->GetText() != NULL)
                {
                    m_NetworkInfo.AddDNSServer(XmlDNS->GetText());
                }
            }
        }
    }

    return true;
}

void_t ServiceSetConfig::Bind(GtpTransHandle Handle)
{
    GtpTransSetReferenceInstance(Handle, this);

    if (GTP_INVALID_HANDLE != m_TransHandle)
    {
        PRINT_LOG(ERROR, "Othre transaction is already attached to service set configure");
        DUMP_VARIABLE(Handle);
        return;
    }

    m_TransHandle = Handle;
}

void_t ServiceSetConfig::Unbind(GtpTransHandle Handle)
{
    GtpTransSetReferenceInstance(Handle, NULL);

    if (m_TransHandle != Handle)
    {
        PRINT_LOG(ERROR, "This transaction is not attached to service set configure");
        DUMP_VARIABLE(Handle);
        return;
    }

    m_TransHandle = GTP_INVALID_HANDLE;

    // Cancel delay task and recycle thread
    Application::GetSingleton().CancelDelayTask(ServiceSetConfig::OnTimeProc, this);
    Wait();
}

void_t ServiceSetConfig::OnTimeProc(void_t * Data)
{
    ASSERT_VALID_POINT(Data);
    ServiceSetConfig * This = static_cast<ServiceSetConfig *>(Data);
    This->OnTime();
}

void_t ServiceSetConfig::OnTime()
{
    ASSERT(IsRunning(), "Thread MUST be running");

    GMI_RESULT RetVal = GMI_SUCCESS;

    if (IsStopped())
    {
        GMI_RESULT ThreadRetVal = GMI_SUCCESS;

        // Recycle resouces of thread
        RetVal = Wait(&ThreadRetVal);
        if (GMI_SUCCESS == RetVal)
        {
            if (ThreadRetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to run thread");
                DUMP_VARIABLE(ThreadRetVal);
            }

            if (GTP_INVALID_HANDLE == m_TransHandle)
            {
                PRINT_LOG(WARNING, "No transaction is binding, no need to send response");
                return;
            }

            // Send error response
            RetVal = SendSimpleResponse(m_TransHandle, "SetConfig", ThreadRetVal);
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to send response");
                GtpDestroyTransHandle(m_TransHandle);
            }
        }
        else
        {
            PRINT_LOG(ALERT, "Faild to stop thread");
        }
    }
    else
    {
        // Try again in 100ms
        RetVal = Application::GetSingleton().AddDelayTask(ServiceSetConfig::OnTimeProc, this, 0, 100000);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to add delay task");

            // TODO:
        }
    }
}

GMI_RESULT ServiceSetConfig::ThreadEntry()
{
    GMI_RESULT RetVal = ConfigureService::GetInstance().SetDeviceInfo(m_DeviceInfo, m_UsingSupper);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to set device information");
        return RetVal;
    }

    RetVal = ConfigureService::GetInstance().SetNetworkInfo(m_NetworkInfo, m_UsingSupper);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to set network information");
        return RetVal;
    }

    return GMI_SUCCESS;
}

