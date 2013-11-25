#include <string>

#include <tinyxml.h>

#include <tool_protocol.h>

#include <common_def.h>
#include <md5_engine.h>

#define MAC_ADDRESS_STRING_LENGTH 18

typedef enum tagRequestId
{
    eRequestSearch = 0,
    eRequestSetConfig,
    eRequestPutFile,
    eRequestGetFile,
} RequestId;

class Request
{
public:
    Request()
        : m_Interface("eth0")
        , m_RequestId(eRequestSearch)
        , m_TargetDevice(NULL)
        , m_MD5Engine()
        , m_Running(false)
        , m_TransHandle(GTP_INVALID_HANDLE)
    {
    }

    const char_t   * m_Interface;
    RequestId        m_RequestId;
    const char_t   * m_TargetDevice;
    MD5Engine        m_MD5Engine;
    boolean_t        m_Running;
    GtpTransHandle   m_TransHandle;
};

// Application running flag
static boolean_t     l_Running          = true;

// Define XML strings
static const uint8_t l_SearchRequest[]    = "<Request operation=\"Search\"/>";
static const uint8_t l_SetConfigRequest[] =
"<Request operation=\"SetConfig\">"
    "<DeviceInfo>"
        "<Name>IPCamera</Name>"
    "</DeviceInfo>"
    "<NetworkInfo>"
        "<InterfaceList num=\"1\">"
            "<Interface name=\"eth0\" enable=\"1\" dhcp=\"0\">"
                "<Address>192.168.1.45</Address>"
                "<Netmask>255.255.255.0</Netmask>"
                "<Gateway>192.168.1.1</Gateway>"
            "</Interface>"
        "</InterfaceList>"
        "<DNSList num=\"2\">"
            "<DNS>192.168.1.1</DNS>"
            "<DNS>8.8.8.8</DNS>"
        "</DNSList>"
    "</NetworkInfo>"
"</Request>";

static const uint8_t l_PutFileRequest[]   =
"<Request operation=\"PutFile\" toolid=\"product\">"
    "<FileInfo compress=\"0\" encrypted=\"0\" reserve=\"1\" reboot=\"1\">"
        "<Filename>capability_configurable.xml</Filename>"
        "<Path>/opt/config/</Path>"
        "<Md5>869672758199da86033f0adb7b627801</Md5>"
    "</FileInfo>"
"</Request>";

static const uint8_t l_PutFileACK[]       = "<ACK operation=\"PutFile\"/>";
static const uint8_t l_GetFileRequest[]   =
"<Request operation=\"GetFile\" toolid=\"product\">"
    "<FileList>"
        "<File>/opt/config/capability_auto.xml</File>"
    "</FileList>"
"</Request>";

static const uint8_t l_GetFileACK[]       = "<ACK operation=\"GetFile\"/>";

static const uint8_t l_FileContent[]      =
"<?xml version=\"1.0\"?>"
"<Capability>"
    "<Shield>None</Shield>"
    "<AlarmPort>None</AlarmPort>"
    "<IRCtrl>None</IRCtrl>"
    "<IRCut>None</IRCut>"
    "<IRAlgorithm>Type0</IRAlgorithm>"
    "<DcIries>None</DcIries>"
    "<Audio>None</Audio>"
    "<SensorMode>Normal</SensorMode>"
    "<WideDynamicRange>None</WideDynamicRange>"
    "<LowLumen>None</LowLumen>"
    "<FloorOSD>None</FloorOSD>"
    "<GB28181>Unsupported</GB28181>"
    "<MaxStreamNum>2</MaxStreamNum>"
    "<MaxPicWidth>1920</MaxPicWidth>"
    "<MaxPicHeight>1080</MaxPicHeight>"
"</Capability>";

// Help to convert binary mac address to string
inline static const char_t * MacAddr2String(uint8_t MacAddr[MAC_ADDRESS_LENGTH])
{
    static char_t MacAddrString[MAC_ADDRESS_STRING_LENGTH];
    snprintf(MacAddrString, sizeof(MacAddrString), "%02x:%02x:%02x:%02x:%02x:%02x", MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);
    return MacAddrString;
}

inline static boolean_t IsRunning()
{
    return l_Running;
}

inline static void_t StopRunning()
{
    l_Running = false;
}

inline static const char_t * Signal2String(int32_t SignalNumber)
{
    switch (SignalNumber)
    {
#define CASE_SIGNAL(sig) case sig: return #sig

        CASE_SIGNAL(SIGTERM);
        CASE_SIGNAL(SIGQUIT);
        CASE_SIGNAL(SIGINT);

#undef CASE_SIGNAL
    }

    return "Unknown";
}

static void_t SignalStop(int32_t SignalNumber)
{
    PRINT_LOG(INFO, "Application received signal %s ...", Signal2String(SignalNumber));

    if (IsRunning())
    {
        PRINT_LOG(INFO, "Try to stop process ...");
        StopRunning();
    }
    else
    {
        PRINT_LOG(WARNING, "Process is already stopping ...");
    }
}

static void_t StartSetConfig(Request * RequestInstance, GtpHandle Handle, uint8_t MacAddr[MAC_ADDRESS_LENGTH])
{
    GMI_RESULT     RetVal      = GMI_SUCCESS;
    GtpTransHandle TransHandle = GTP_INVALID_HANDLE;

    do
    {
        TransHandle = GtpCreateTransHandle(Handle, MacAddr);
        if (GTP_INVALID_HANDLE == TransHandle)
        {
            PRINT_LOG(ERROR, "Faield to create transaction");
            break;
        }

        RetVal = GtpTransSendData(TransHandle, l_SetConfigRequest, sizeof(l_SetConfigRequest), GTP_DATA_TYPE_UTF8_XML);
        if (GMI_SUCCESS != RetVal)
        {
            PRINT_LOG(ERROR, "Failed to send data");
            break;
        }

        RequestInstance->m_TransHandle = TransHandle;
        return;
    } while (0);

    if (TransHandle != GTP_INVALID_HANDLE)
    {
        GtpDestroyTransHandle(TransHandle);
    }
}

static void_t StartPutFile(Request * RequestInstance, GtpHandle Handle, uint8_t MacAddr[MAC_ADDRESS_LENGTH])
{
    GMI_RESULT     RetVal      = GMI_SUCCESS;
    GtpTransHandle TransHandle = GTP_INVALID_HANDLE;

    do
    {
        TransHandle = GtpCreateTransHandle(Handle, MacAddr);
        if (GTP_INVALID_HANDLE == TransHandle)
        {
            PRINT_LOG(ERROR, "Faield to create transaction");
            break;
        }

        RetVal = GtpTransSendData(TransHandle, l_PutFileRequest, sizeof(l_PutFileRequest), GTP_DATA_TYPE_UTF8_XML);
        if (GMI_SUCCESS != RetVal)
        {
            PRINT_LOG(ERROR, "Failed to send data");
            break;
        }

        RequestInstance->m_TransHandle = TransHandle;
        return;
    } while (0);

    if (TransHandle != GTP_INVALID_HANDLE)
    {
        GtpDestroyTransHandle(TransHandle);
    }
}

static void_t StartGetFile(Request * RequestInstance, GtpHandle Handle, uint8_t MacAddr[MAC_ADDRESS_LENGTH])
{
    GMI_RESULT     RetVal      = GMI_SUCCESS;
    GtpTransHandle TransHandle = GTP_INVALID_HANDLE;

    do
    {
        TransHandle = GtpCreateTransHandle(Handle, MacAddr);
        if (GTP_INVALID_HANDLE == TransHandle)
        {
            PRINT_LOG(ERROR, "Faield to create transaction");
            break;
        }

        RetVal = GtpTransSendData(TransHandle, l_GetFileRequest, sizeof(l_GetFileRequest), GTP_DATA_TYPE_UTF8_XML);
        if (GMI_SUCCESS != RetVal)
        {
            PRINT_LOG(ERROR, "Failed to send data");
            break;
        }

        RequestInstance->m_TransHandle = TransHandle;
        return;
    } while (0);

    if (TransHandle != GTP_INVALID_HANDLE)
    {
        GtpDestroyTransHandle(TransHandle);
    }
}

static void_t OnRecvXML(Request * RequestInstance, GtpTransHandle TransHandle, const char_t * Buffer, uint32_t BufferLength)
{
    std::string     XMLString(Buffer, BufferLength);
    TiXmlDocument   XMLDoc;
    TiXmlElement  * XMLRootElement = NULL;
    GtpHandle       Handle         = GtpTransGetGtpHandle(TransHandle);
    uint8_t         MacAddr[MAC_ADDRESS_LENGTH];

    GtpTransGetMacAddress(TransHandle, MacAddr);
    PRINT_LOG(DEBUG, "Received XML text from %s:\n%s", MacAddr2String(MacAddr), XMLString.c_str());

    XMLDoc.Parse(XMLString.c_str());
    XMLRootElement = XMLDoc.RootElement();
    if (NULL == XMLRootElement || NULL == XMLRootElement->Value())
    {
        PRINT_LOG(WARNING, "No root element in XML string");
        return;
    }

    // Get attribute 'operation' of root element
    const char_t * Operation = XMLRootElement->Attribute("operation");
    if (NULL == Operation)
    {
        PRINT_LOG(WARNING, "Failed to parse XML string, no attribute 'operation' in root element");
        return;
    }

    if (strcmp(XMLRootElement->Value(), "Response") == 0)
    {
        int32_t Result = 0;
        if (XMLRootElement->QueryIntAttribute("result", &Result) != TIXML_SUCCESS)
        {
            PRINT_LOG(WARNING, "Failed to parse XML string, no attribute 'result' in 'Response' element");
            return;
        }

        if (strcmp(Operation, "Search") == 0)
        {
            PRINT_LOG(INFO, "Device %s is online, result : 0x%08x", MacAddr2String(MacAddr), Result);

            if (RequestInstance->m_Running)
            {
                // Already running request
                return;
            }

            if (RequestInstance->m_TargetDevice == NULL)
            {
                // No target device
                return;
            }

            if (strcmp(RequestInstance->m_TargetDevice, MacAddr2String(MacAddr)) != 0)
            {
                // Not the target device
                return;
            }

            switch (RequestInstance->m_RequestId)
            {
            case eRequestSearch:
                // Nothing to do
                break;

            case eRequestSetConfig:
                StartSetConfig(RequestInstance, Handle, MacAddr);
                break;

            case eRequestPutFile:
                StartPutFile(RequestInstance, Handle, MacAddr);
                break;

            case eRequestGetFile:
                StartGetFile(RequestInstance, Handle, MacAddr);
                break;

            default:
                PRINT_LOG(ALERT, "Unknown request id");
                return;
            }

            RequestInstance->m_Running = true;
        }
        else if (strcmp(Operation, "SetConfig") == 0)
        {
            ASSERT_EQUAL(RequestInstance->m_RequestId, eRequestSetConfig);

            PRINT_LOG(INFO, "Set configure request complete, result : 0x%08x", Result);
        }
        else if (strcmp(Operation, "PutFile") == 0)
        {
            ASSERT_EQUAL(RequestInstance->m_RequestId, eRequestPutFile);

            PRINT_LOG(INFO, "Put file request complete, result : 0x%08x", Result);
        }
        else if (strcmp(Operation, "GetFile") == 0)
        {
            ASSERT_EQUAL(RequestInstance->m_RequestId, eRequestGetFile);

            if (GMI_SUCCESS == Result)
            {
                char_t MD5Value[33];
                RequestInstance->m_MD5Engine.Finalize();
                RequestInstance->m_MD5Engine.HexDigest(MD5Value, sizeof(MD5Value));

                PRINT_LOG(INFO, "Get file request complete, MD5Value = %s", MD5Value);
            }
            else
            {
                PRINT_LOG(WARNING, "Get file request failed, result : 0x%08x", Result);
            }
        }
        else
        {
            PRINT_LOG(WARNING, "Unknown response");
            DUMP_VARIABLE(Operation);
        }
    }
    else if (strcmp(XMLRootElement->Value(), "ACK") == 0)
    {
        if (strcmp(Operation, "PutFile") == 0)
        {
            ASSERT_EQUAL(RequestInstance->m_RequestId, eRequestPutFile);

            GMI_RESULT RetVal = GtpTransSendData(TransHandle, l_FileContent, sizeof(l_FileContent), GTP_DATA_TYPE_BINARY);
            if (GMI_SUCCESS != RetVal)
            {
                PRINT_LOG(ERROR, "Failed to send data");
                return;
            }

            RetVal = GtpTransSendData(TransHandle, l_PutFileACK, sizeof(l_PutFileACK), GTP_DATA_TYPE_UTF8_XML);
            if (GMI_SUCCESS != RetVal)
            {
                PRINT_LOG(ERROR, "Failed to send data");
                return;
            }
        }
        else if (strcmp(Operation, "GetFile") == 0)
        {
            ASSERT_EQUAL(RequestInstance->m_RequestId, eRequestGetFile);

            GMI_RESULT RetVal = GtpTransSendData(TransHandle, l_GetFileACK, sizeof(l_GetFileACK), GTP_DATA_TYPE_UTF8_XML);
            if (GMI_SUCCESS != RetVal)
            {
                PRINT_LOG(ERROR, "Failed to send data");
                return;
            }
        }
        else
        {
            PRINT_LOG(WARNING, "Unknown ACK");
            DUMP_VARIABLE(Operation);
        }
    }
    else if (strcmp(XMLRootElement->Value(), "Notify") == 0)
    {
        if (strcmp(Operation, "OffLine") == 0)
        {
            PRINT_LOG(INFO, "%s has notified offline", MacAddr2String(MacAddr));
        }
        else
        {
            PRINT_LOG(WARNING, "Unknown notification");
            DUMP_VARIABLE(Operation);
        }
    }
    else if (strcmp(XMLRootElement->Value(), "Request") == 0)
    {
        PRINT_LOG(VERBOSE, "Receive a request from other device");
    }
    else
    {
        PRINT_LOG(WARNING, "Unknown XML root element");
        DUMP_VARIABLE(XMLRootElement->Value());
    }
}

static void_t OnSentProc(void_t * Data, GtpTransHandle TransHandle, GMI_RESULT Errno)
{
    ASSERT_VALID_POINT(Data);
    Request * RequestInstance = reinterpret_cast<Request *>(Data);

    if (Errno != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send data");
        DUMP_VARIABLE(Errno);
        GtpDestroyTransHandle(TransHandle);
        return;
    }

    if (RequestInstance->m_TransHandle == TransHandle)
    {
        switch (RequestInstance->m_RequestId)
        {
        case eRequestSearch:
            PRINT_LOG(ALERT, "MUST NOT be search request");
            break;

        case eRequestSetConfig:
            PRINT_LOG(VERBOSE, "Succeed to send set configure request");
            break;

        case eRequestPutFile:
            PRINT_LOG(VERBOSE, "Succeed to send put file packet");
            break;

        case eRequestGetFile:
            PRINT_LOG(VERBOSE, "Succeed to send get file packet");
            break;

        default:
            PRINT_LOG(ALERT, "Unknown request id");
            return;
        }
    }
}

static void_t OnRecvProc(void_t * Data, GtpTransHandle TransHandle, const uint8_t * Buffer, uint32_t BufferLength, uint8_t Type)
{
    ASSERT_VALID_POINT(Data);
    Request * RequestInstance = reinterpret_cast<Request *>(Data);

    switch (Type)
    {
    case GTP_DATA_TYPE_UTF8_XML:
        {
            OnRecvXML(RequestInstance, TransHandle, reinterpret_cast<const char_t *>(Buffer), BufferLength);
            break;
        }

    case GTP_DATA_TYPE_BINARY:
        {
            ASSERT_EQUAL(RequestInstance->m_RequestId, eRequestGetFile);
            RequestInstance->m_MD5Engine.Update(Buffer, BufferLength);
            break;
        }

    case GTP_DATA_TYPE_USER1:
    case GTP_DATA_TYPE_USER2:
    default:
        {
            PRINT_LOG(ALERT, "Unknown data type frome server");
            break;
        }
    }
}

static void_t OnTransactionKilledProc(void_t * Data, GtpTransHandle TransHandle)
{
    ASSERT_VALID_POINT(Data);
    Request * RequestInstance = reinterpret_cast<Request *>(Data);

    if (RequestInstance->m_TransHandle == TransHandle)
    {
        RequestInstance->m_TransHandle = GTP_INVALID_HANDLE;
    }
}

static void_t StartSearch(void_t * Data)
{
    ASSERT_VALID_POINT(Data);
    GtpTransHandle TransHandle = reinterpret_cast<GtpTransHandle>(Data);
    GtpHandle      Handle      = GtpTransGetGtpHandle(TransHandle);
    GMI_RESULT     RetVal      = GMI_SUCCESS;

    RetVal = GtpTransSendData(TransHandle, l_SearchRequest, sizeof(l_SearchRequest), GTP_DATA_TYPE_UTF8_XML);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to send search request");
    }

    // Run search command again in 10 seconds
    RetVal = GtpAddDelayTask(Handle, StartSearch, Data, 10, 0);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to add delay task");
    }
}

static GMI_RESULT MainLoop(GtpHandle Handle)
{
    GMI_RESULT RetVal = GMI_SUCCESS;

    while (IsRunning())
    {
        RetVal = GtpDoEvent(Handle);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(EMERGENCY, "Failed to call GtpDoEvent()");
            DUMP_VARIABLE(Handle);
            break;
        }
    }

    return RetVal;
}

static Request * ParseArgument(int argc, const char * argv [])
{
    Request * NewRequest = new Request();
    if (NULL == NewRequest)
    {
        PRINT_LOG(ALERT, "Not enough memory");
        return NULL;
    }

    for (int i = 1; i < argc; ++ i)
    {
        const char * Parameter = argv[i] + 2;

        if ('-' == argv[i][0])
        {
            switch (argv[i][1])
            {
            case 'i':
                {
                    NewRequest->m_Interface = Parameter;
                    break;
                }

            case 'R':
                {
                    if (strcmp(Parameter, "Search") == 0)
                    {
                        NewRequest->m_RequestId = eRequestSearch;
                    }
                    else if (strcmp(Parameter, "SetConfig") == 0)
                    {
                        NewRequest->m_RequestId = eRequestSetConfig;
                    }
                    else if (strcmp(Parameter, "PutFile") == 0)
                    {
                        NewRequest->m_RequestId = eRequestPutFile;
                    }
                    else if (strcmp(Parameter, "GetFile") == 0)
                    {
                        NewRequest->m_RequestId = eRequestGetFile;
                    }
                    else
                    {
                        PRINT_LOG(ERROR, "Unknown request");
                        StopRunning();
                    }

                    break;
                }

            case 'd':
                {
                    NewRequest->m_TargetDevice = Parameter;
                    break;
                }

            default:
                PRINT_LOG(ERROR, "Unknown option");
                StopRunning();
                break;
            }
        }
        else
        {
            PRINT_LOG(ERROR, "Unknown agrument");
            StopRunning();
        }
    }

    return NewRequest;
}

int main(int argc, const char * argv [])
{
    // Initialized log module
    LOG_INITIALIZE(NULL, 0);
    LOG_DISABLE_WRITE();
    LOG_SET_DISPLAY_LEVEL(DEBUG);

    signal(SIGTERM, SignalStop);
    signal(SIGINT,  SignalStop);
    signal(SIGQUIT, SignalStop);

    GMI_RESULT          RetVal      = GMI_SUCCESS;
    PcapSessionHandle   PcapSession = PCAP_SESSION_INVALID_HANDLE;
    GtpHandle           Handle      = GTP_INVALID_HANDLE;
    GtpTransHandle      TransHandle = GTP_INVALID_HANDLE;
    Request           * NewRequest  = NULL;

    do
    {
        NewRequest = ParseArgument(argc, argv);
        if (NULL == NewRequest)
        {
            PRINT_LOG(ERROR, "Failed to parse argument");
            break;
        }

        PcapSession = PcapSessionOpen(NewRequest->m_Interface, 500);
        if (PCAP_SESSION_INVALID_HANDLE == PcapSession)
        {
            PRINT_LOG(ERROR, "Failed to open pcap session on interface %s", NewRequest->m_Interface);
            break;
        }

        Handle = GtpCreateClientHandle(PcapSession);
        if (GTP_INVALID_HANDLE == Handle)
        {
            PRINT_LOG(ERROR, "Failed to create client handle");
            break;
        }

        // Register proc functions
        GtpSetSentCallback(Handle, OnSentProc, reinterpret_cast<void_t *>(NewRequest));
        GtpSetRecvCallback(Handle, OnRecvProc, reinterpret_cast<void_t *>(NewRequest));
        GtpSetTransKilledCallback(Handle, OnTransactionKilledProc, reinterpret_cast<void_t *>(NewRequest));

        TransHandle = GtpCreateBroadcastTransHandle(Handle);
        if (GTP_INVALID_HANDLE == TransHandle)
        {
            PRINT_LOG(ERROR, "Failed to create broadcast transaction");
            break;
        }

        RetVal = GtpAddDelayTask(Handle, StartSearch, reinterpret_cast<void_t *>(TransHandle), 0, 0);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to add delay task");
            break;
        }

        RetVal = MainLoop(Handle);
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to run main loop");
            break;
        }
    } while (0); 

    if (TransHandle != GTP_INVALID_HANDLE)
    {
        GtpDestroyTransHandle(TransHandle);
    }

    if (Handle != GTP_INVALID_HANDLE)
    {
        GtpDestroyHandle(Handle);
    }

    if (PcapSession != PCAP_SESSION_INVALID_HANDLE)
    {
        PcapSessionClose(PcapSession);
    }

    if (NewRequest != NULL)
    {
        delete NewRequest;
    }

    return 0;
}
