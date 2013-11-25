#include "service_search.h"
#include "service_dispatch.h"
#include "application.h"
#include "configure_service.h"

// Create service
static ServiceSearch l_ServiceSearchInstance;

template <>
ServiceSearch * Singleton<ServiceSearch>::ms_InstancePtr = NULL;

static inline GMI_RESULT FillNetworkInfo(TiXmlElement * Parent, const NetworkInfo & NetInfo)
{
    TiXmlElement * XmlNetworkInfo   = NULL;
    TiXmlElement * XmlInterfaceList = NULL;
    TiXmlElement * XmlDNSList       = NULL;
    TiXmlElement * XmlElement       = NULL;
    TiXmlText    * XmlText          = NULL;
    uint32_t       i                = 0;
    GMI_RESULT     RetVal           = GMI_SUCCESS;

    do
    {
        // Create network root element
        NEW_XML_ELEMENT(XmlNetworkInfo, "NetworkInfo", Parent, RetVal);

        // Create interface list element
        NEW_XML_ELEMENT(XmlInterfaceList, "InterfaceList", XmlNetworkInfo, RetVal);

        // Set interface number
        XmlInterfaceList->SetAttribute("num", NetInfo.GetInterfaceCount());

        for (i = 0; i < NetInfo.GetInterfaceCount(); ++ i)
        {
            const InterfaceInfo & Iface        = NetInfo.GetInterface(i);
            TiXmlElement        * XmlInterface = NULL;

            // Create interface element
            NEW_XML_ELEMENT(XmlInterface, "Interface", XmlInterfaceList, RetVal);
            XmlInterface->SetAttribute("name", Iface.GetName().c_str());
            XmlInterface->SetAttribute("enable", Iface.IsEnabled() ? 1 : 0);
            XmlInterface->SetAttribute("dhcp", Iface.IsDHCP() ? 1 : 0);

            NEW_XML_ELEMENT(XmlElement, "Address", XmlInterface, RetVal);
            NEW_XML_TEXT(XmlText, Iface.GetIpAddress().c_str(), XmlElement, RetVal);

            NEW_XML_ELEMENT(XmlElement, "Netmask", XmlInterface, RetVal);
            NEW_XML_TEXT(XmlText, Iface.GetNetmask().c_str(), XmlElement, RetVal);

            NEW_XML_ELEMENT(XmlElement, "Gateway", XmlInterface, RetVal);
            NEW_XML_TEXT(XmlText, Iface.GetGateway().c_str(), XmlElement, RetVal);

            NEW_XML_ELEMENT(XmlElement, "MAC", XmlInterface, RetVal);
            NEW_XML_TEXT(XmlText, Iface.GetMacAddress().c_str(), XmlElement, RetVal);
        }

        // Create DNS list element
        NEW_XML_ELEMENT(XmlDNSList, "DNSList", XmlNetworkInfo, RetVal);

        // Set DNS number
        XmlDNSList->SetAttribute("num", NetInfo.GetDNSServerCount());

        for (i = 0; i < NetInfo.GetDNSServerCount(); ++ i)
        {
            // Create DNS element
            NEW_XML_ELEMENT(XmlElement, "DNS", XmlDNSList, RetVal);
            NEW_XML_TEXT(XmlText, NetInfo.GetDNSServer(i).c_str(), XmlElement, RetVal);
        }

    } while (0);

    return RetVal;
}

static inline GMI_RESULT FillDeviceInfo(TiXmlElement * Parent, const DeviceInfo & DevInfo)
{
    TiXmlElement * XmlDevInfo = NULL;
    TiXmlElement * XmlElement = NULL;
    TiXmlText    * XmlText    = NULL;
    GMI_RESULT     RetVal     = GMI_SUCCESS;

    do
    {
        // Create device root element
        NEW_XML_ELEMENT(XmlDevInfo, "DeviceInfo", Parent, RetVal);

        // Get device name
        NEW_XML_ELEMENT(XmlElement, "Name", XmlDevInfo, RetVal);
        NEW_XML_TEXT(XmlText, DevInfo.GetName().c_str(), XmlElement, RetVal);

        // Get serial number
        NEW_XML_ELEMENT(XmlElement, "SN", XmlDevInfo, RetVal);
        NEW_XML_TEXT(XmlText, DevInfo.GetSerialNumber().c_str(), XmlElement, RetVal);

        // Get hardware verion
        NEW_XML_ELEMENT(XmlElement, "HwVersion", XmlDevInfo, RetVal);
        NEW_XML_TEXT(XmlText, DevInfo.GetHardwareVersion().c_str(), XmlElement, RetVal);

        // Get firmware version
        NEW_XML_ELEMENT(XmlElement, "FwVersion", XmlDevInfo, RetVal);
        NEW_XML_TEXT(XmlText, DevInfo.GetFirmwareVersion().c_str(), XmlElement, RetVal);

        // Get last boot time version
        NEW_XML_ELEMENT(XmlElement, "LastBootTime", XmlDevInfo, RetVal);
        NEW_XML_TEXT(XmlText, DevInfo.GetLastBootTime().c_str(), XmlElement, RetVal);

    } while (0);

    return RetVal;
}

static inline GMI_RESULT FillServicePorts(TiXmlElement * Parent, const ServicePorts & Ports)
{
    TiXmlElement * XmlServiceList = NULL;
    TiXmlElement * XmlElement     = NULL;
    GMI_RESULT     RetVal         = GMI_SUCCESS;

    ServicePorts::const_iterator it;

    do
    {
        // Create services root element
        NEW_XML_ELEMENT(XmlServiceList, "ServiceList", Parent, RetVal);

        for (it = Ports.begin(); it != Ports.end(); ++ it)
        {
             NEW_XML_ELEMENT(XmlElement, "Service", XmlServiceList, RetVal);
             XmlElement->SetAttribute("name", it->first.c_str());
             XmlElement->SetAttribute("port", it->second);
        }

    } while (0);

    return RetVal;
}

ServiceSearch::ServiceSearch()
    : m_TransList()
    , m_NeedUpdate(true)
    , m_ResponseString()
{
    ServiceDispatch::GetInstance().Register(this, this);
}

ServiceSearch::~ServiceSearch()
{
    ServiceDispatch::GetInstance().Unregister(this);

    // Recycle resources of thread, if thread is still running
    if (IsRunning())
    {
        Wait();
    }
}

void_t ServiceSearch::Execute(GtpTransHandle Handle, IParser * Parser)
{
    // Ignore parser, because parser is itself

    GMI_RESULT RetVal = GMI_SUCCESS;

    // Bind transaction to this service
    Bind(Handle);

    // Check if response XML need update
    if (!NeedUpdate())
    {
        // Send response
        RetVal = GtpTransSendData(Handle, reinterpret_cast<const uint8_t *>(m_ResponseString.c_str()), m_ResponseString.length(), GTP_DATA_TYPE_UTF8_XML);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to send response");
            GtpDestroyTransHandle(Handle);
        }
        return;
    }

    // Check if response XML is updating
    if (IsRunning())
    {
        // Nothing to do, just wait
        return;
    }

    do
    {
        RetVal = Application::GetSingleton().AddDelayTask(ServiceSearch::OnTimeProc, this, 0, 100000);
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

        return;
    } while (0); 

    // Cancel task
    Application::GetSingleton().CancelDelayTask(ServiceSearch::OnTimeProc, this);

    // Send error response
    RetVal = SendSimpleResponse(Handle, "Search", RetVal);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send response");
        GtpDestroyTransHandle(Handle);
    }
}

const char_t * ServiceSearch::Name() const
{
    return "Search";
}

boolean_t ServiceSearch::ParseBinary(const uint8_t * Buffer, uint32_t BufferLength)
{
    // We could not parse binary data
    return false;
}

boolean_t ServiceSearch::ParseXML(TiXmlDocument & XmlDoc)
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
    if (NULL == Operation || strcmp(Operation, "Search") != 0)
    {
        PRINT_LOG(VERBOSE, "Not a search request");
        return false;
    }

    PRINT_LOG(VERBOSE, "It is a search request");
    return true;
}

void_t ServiceSearch::Bind(GtpTransHandle Handle)
{
    GtpTransSetReferenceInstance(Handle, this);

    m_TransList.push_back(Handle);
}

void_t ServiceSearch::Unbind(GtpTransHandle Handle)
{
    GtpTransSetReferenceInstance(Handle, NULL);

    std::vector<GtpTransHandle>::iterator it;
    for (it = m_TransList.begin(); it != m_TransList.end(); ++ it)
    {
        if (*it == Handle)
        {
            m_TransList.erase(it);
            return;
        }
    }

    PRINT_LOG(ERROR, "This transaction is not attached to service search");
    DUMP_VARIABLE(Handle);
}

void_t ServiceSearch::OnTimeProc(void_t * Data)
{
    ASSERT_VALID_POINT(Data);
    ServiceSearch * This = static_cast<ServiceSearch *>(Data);
    This->OnTime();
}

void_t ServiceSearch::OnTime()
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

            PRINT_LOG(VERBOSE, "Sending response:\n %s", m_ResponseString.c_str());

            std::vector<GtpTransHandle>::iterator it = m_TransList.begin();

            // Send response to all transations in list
            while (it != m_TransList.end())
            {
                GtpTransHandle Handle = *(it ++);

                // Send response
                RetVal = GtpTransSendData(Handle, reinterpret_cast<const uint8_t *>(m_ResponseString.c_str()), m_ResponseString.length(), GTP_DATA_TYPE_UTF8_XML);
                if (RetVal != GMI_SUCCESS)
                {
                    PRINT_LOG(ERROR, "Failed to send response");
                    GtpDestroyTransHandle(Handle);
                }
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
        RetVal = Application::GetSingleton().AddDelayTask(ServiceSearch::OnTimeProc, this, 0, 100000);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to add delay task");

            // TODO:
        }
    }
}

GMI_RESULT ServiceSearch::ThreadEntry()
{
    NetworkInfo     NetInfo;
    DeviceInfo      DevInfo;
    ServicePorts    Ports;
    TiXmlDocument   XmlDoc;
    TiXmlPrinter    XmlPrinter;
    TiXmlElement  * XmlRootElement = NULL;
    GMI_RESULT      RetVal         = GMI_SUCCESS;

    do
    {
        // Create root element of response XML
        NEW_XML_ELEMENT(XmlRootElement, "Response", &XmlDoc, RetVal);
        XmlRootElement->SetAttribute("operation", "Search");
        XmlRootElement->SetAttribute("result", RetVal2Int(GMI_SUCCESS));

        // Get network information
        RetVal = ConfigureService::GetInstance().GetNetworkInfo(NetInfo);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to get network information");
            break;
        }

        // Convert network information to XML format
        RetVal = FillNetworkInfo(XmlRootElement, NetInfo);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to fill network information");
            break;
        }

        // We will ignore the return value below
        do
        {
            // Get device information
            RetVal = ConfigureService::GetInstance().GetDeviceInfo(DevInfo);
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(WARNING, "Failed to get device information");
                break;
            }

            // Convert device information to XML format
            RetVal = FillDeviceInfo(XmlRootElement, DevInfo);
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to fill device information");
                break;
            }

            // Get port number of all services
            RetVal = ConfigureService::GetInstance().GetServicePorts(Ports);
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(WARNING, "Failed to get service ports");
                break;
            }

            // Convert them to XML format
            RetVal = FillServicePorts(XmlRootElement, Ports);
            if (RetVal != GMI_SUCCESS)
            {
                PRINT_LOG(ERROR, "Failed to fill service ports");
                break;
            }

        } while (0);

        // Convert xml document tree to string
        if (!XmlDoc.Accept(&XmlPrinter))
        {
            PRINT_LOG(ERROR, "Failed to generate response XML string");
            RetVal = GMI_FAIL;
            break;
        }

        // If every thing is OK, then remove the update flag
        if (RetVal == GMI_SUCCESS)
        {
            m_NeedUpdate = false;
        }

        m_ResponseString.assign(XmlPrinter.CStr(), XmlPrinter.Size());
        return GMI_SUCCESS;
    } while (0); 

    // Prepare error response XML
    char_t  ErrResponse[128];
    snprintf(ErrResponse,  sizeof(ErrResponse), XML_SIMPLE_RESPONSE, "Search", RetVal2Int(RetVal));
    m_ResponseString = ErrResponse;

    return RetVal;
}

