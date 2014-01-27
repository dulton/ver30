#include <sys_client.h>
#include <sys_env_types.h>
#include <ipc_fw_v3.x_resource.h>
#include <ipc_fw_v3.x_setting.h>
#include <gmi_daemon_heartbeat_api.h>

#include "configure_service.h"

// Define length of buffers
#define MAC_ADDRESS_LENGTH        6
#define IP_ADDRESS_STRING_LENGTH  16
#define MAC_ADDRESS_STRING_LENGTH 18
#define MAX_STRING_LENGTH         128
#define MAX_INTERFACE_NAME_LENGTH IFNAMSIZ

// Define system configure files
#define DNS_FILE_PATH             "/etc/resolv.conf"
#define ROUTE_FILE_PATH           "/proc/net/route"
#define INTERFACE_FILE_PATH       "/etc/network/interfaces"

// Define interface list
static const char_t * l_InterfaceList[] = { "eth0" };

// Help to initialize structure of interface request
inline static void_t InitInterfaceRequest(struct ifreq & IfReq, const char_t * Name)
{
    memset(&IfReq, 0, sizeof(IfReq));
    strncpy(IfReq.ifr_name, Name, sizeof(IfReq.ifr_name) - 1);
}

// Help to convert binary mac address to string
inline static const char_t * MacAddr2String(uint8_t MacAddr[MAC_ADDRESS_LENGTH])
{
    static char_t MacAddrString[MAC_ADDRESS_STRING_LENGTH];
    snprintf(MacAddrString, sizeof(MacAddrString), "%02x:%02x:%02x:%02x:%02x:%02x",
        MacAddr[0], MacAddr[1], MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);

    return MacAddrString;
}

// Help to convert structure of system time to string
inline static const char_t * SysTime2String(SysPkgSysTime & SysTime)
{
    static char_t TimeString[MAX_STRING_LENGTH];
    snprintf(TimeString, sizeof(TimeString), "%04d-%02d-%02dT%02d:%02d:%02d",
    SysTime.s_Year, SysTime.s_Month, SysTime.s_Day, SysTime.s_Hour, SysTime.s_Minute, SysTime.s_Second);

    return TimeString;
}

ConfigureService::ConfigureService()
    : m_Initialized(false)
    , m_Logined(false)
    , m_AuthValue(0)
    , m_SessionId(0)
    , m_UserFlag(0)
{
}

ConfigureService::~ConfigureService()
{
    Uninitialize();
}

GMI_RESULT ConfigureService::Initialize()
{
    if (Initialized())
    {
        return GMI_ALREADY_OPERATED;
    }

    GMI_RESULT RetVal = GMI_SUCCESS;

    do
    {
        // Initialize sys client
        RetVal = SysInitialize(GetAuthLocalPort());
        if (RetVal != GMI_SUCCESS)
        {
            PRINT_LOG(ERROR, "Failed to initialize sys client");
            break;
        }

        m_Initialized = true;

        return GMI_SUCCESS;
    } while (0);

    return RetVal;
}

GMI_RESULT ConfigureService::Uninitialize()
{
    if (!Initialized())
    {
        return GMI_INVALID_OPERATION;
    }

    GMI_RESULT RetVal = Logout();
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to logout supper user");
    }

    RetVal = SysDeinitialize();
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to deinitialize");
    }

    m_Initialized = false;
    return RetVal;
}

GMI_RESULT ConfigureService::GetNetworkInfo(NetworkInfo & NetInfo) const
{
    if (!Initialized())
    {
        PRINT_LOG(ERROR, "Configure service is not initialized yet");
        return GMI_INVALID_OPERATION;
    }

    // Clear network information first
    NetInfo.ClearInterfaceList();
    NetInfo.ClearDNSServerList();

    GMI_RESULT      RetVal  = GMI_SUCCESS;
    int             SockFd  = -1;
    FILE          * FilePtr = NULL;
    static char_t   Buf[MAX_STRING_LENGTH];
    static char_t   IpAddr[IP_ADDRESS_STRING_LENGTH];

    do
    {
        // Create socket
        SockFd = socket(AF_INET, SOCK_STREAM, 0);
        if (SockFd < 0)
        {
            PRINT_LOG(ERROR, "Failed to create socket, errno = %d", errno);
            RetVal = GMI_FAIL;
            break;
        }

        // TODO: use system calls to get interface list
        for (uint32_t i = 0; i < COUNT_OF(l_InterfaceList); ++ i)
        {
            InterfaceInfo IfaceInfo(l_InterfaceList[i]);
            if (GMI_SUCCESS == GetInterfaceInfo(SockFd, IfaceInfo))
            {
                NetInfo.AddInterface(IfaceInfo);
            }
        }

        // Open dns server configure file
        FilePtr = fopen(DNS_FILE_PATH, "r");
        if (FilePtr == NULL)
        {
            PRINT_LOG(WARNING, "Failed to open '%s'", DNS_FILE_PATH);
            break;
        }

        // Get dns server list
        while (fgets(Buf, sizeof(Buf), FilePtr) != NULL)
        {
            if (sscanf(Buf, "nameserver %16s", IpAddr) != 1)
            {
                continue;
            }

            NetInfo.AddDNSServer(IpAddr);
        }
    } while (0);

    if (SockFd >= 0)
    {
        close(SockFd);
    }

    if (FilePtr != NULL)
    {
        fclose(FilePtr);
    }

    return RetVal;
}

GMI_RESULT ConfigureService::GetDeviceInfo(DeviceInfo & DevInfo)
{
    if (!Initialized())
    {
        PRINT_LOG(ERROR, "Configure service is not initialized yet");
        return GMI_INVALID_OPERATION;
    }

    GMI_RESULT RetVal = Login();
    if (RetVal != GMI_SUCCESS)
    {
        return RetVal;
    }

    SysPkgDeviceInfo PkgDevInfo;
    memset(&PkgDevInfo, 0x00, sizeof(PkgDevInfo));

    RetVal = SysGetDeviceInfo(m_SessionId, m_AuthValue, &PkgDevInfo);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to get device information");
        return RetVal;
    }

    DevInfo.SetName(PkgDevInfo.s_DeviceName);
    DevInfo.SetSerialNumber(PkgDevInfo.s_DeviceSerialNum);
    DevInfo.SetHardwareVersion(PkgDevInfo.s_DeviceHwVer);
    DevInfo.SetFirmwareVersion(PkgDevInfo.s_DeviceFwVer);

    SysPkgSysTime BootTime;
    RetVal = SysGetDeviceStartedTime(&BootTime);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to get device boot time");
        return RetVal;
    }

    DevInfo.SetLastBootTime(SysTime2String(BootTime));

    return GMI_SUCCESS;
}

GMI_RESULT ConfigureService::GetServicePorts(ServicePorts & Ports)
{
    if (!Initialized())
    {
        PRINT_LOG(ERROR, "Configure service is not initialized yet");
        return GMI_INVALID_OPERATION;
    }

    GMI_RESULT RetVal = Login();
    if (RetVal != GMI_SUCCESS)
    {
        return RetVal;
    }

    SysPkgNetworkPort SysNetworkPorts;
    memset(&SysNetworkPorts, 0x00, sizeof(SysNetworkPorts));

    RetVal = SysGetNetworkPort(m_SessionId, m_AuthValue, &SysNetworkPorts);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to get ports for all the services");
        return RetVal;
    }

    Ports.clear();
    Ports["HTTP"] = static_cast<uint16_t>(SysNetworkPorts.s_HTTP_Port);
    Ports["RTSP"] = static_cast<uint16_t>(SysNetworkPorts.s_RTSP_Port);
    Ports["SDK"] = static_cast<uint16_t>(SysNetworkPorts.s_SDK_Port);
    Ports["Upgrade"] = static_cast<uint16_t>(SysNetworkPorts.s_Upgrade_Port);

    return GMI_SUCCESS;
}

GMI_RESULT ConfigureService::SetNetworkInfo(const NetworkInfo & NetInfo, boolean_t UsingSupper)
{
    if (!Initialized())
    {
        PRINT_LOG(ERROR, "Configure service is not initialized yet");
        return GMI_INVALID_OPERATION;
    }

    GMI_RESULT RetVal = Login();
    if (RetVal != GMI_SUCCESS)
    {
        return RetVal;
    }

    SysPkgIpInfo PkgIpInfo;
    uint32_t     i = 0;

    memset(&PkgIpInfo, 0x00, sizeof(PkgIpInfo));

    // Configure device information
    RetVal = SysGetDeviceIP(m_SessionId, m_AuthValue, &PkgIpInfo);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to set network information");
        return RetVal;
    }

    PRINT_LOG(VERBOSE, "Get interface name: %s", PkgIpInfo.s_InterfName);

    for (i = 0; i < NetInfo.GetInterfaceCount(); ++ i)
    {
        const InterfaceInfo & Iface = NetInfo.GetInterface(i);
        if (Iface.GetName() != PkgIpInfo.s_InterfName)
        {
            continue;
        }

        snprintf(PkgIpInfo.s_IpAddress, sizeof(PkgIpInfo.s_IpAddress), "%s", Iface.GetIpAddress().c_str());
        PRINT_LOG(VERBOSE, "Set ip address: %s", PkgIpInfo.s_IpAddress);

        snprintf(PkgIpInfo.s_SubNetMask, sizeof(PkgIpInfo.s_SubNetMask), "%s", Iface.GetNetmask().c_str());
        PRINT_LOG(VERBOSE, "Set netmask: %s", PkgIpInfo.s_SubNetMask);

        snprintf(PkgIpInfo.s_GateWay, sizeof(PkgIpInfo.s_GateWay), "%s", Iface.GetGateway().c_str());
        PRINT_LOG(VERBOSE, "Set gateway: %s", PkgIpInfo.s_GateWay);

        if (UsingSupper)
        {
            snprintf(PkgIpInfo.s_HwAddress, sizeof(PkgIpInfo.s_HwAddress), "%s", Iface.GetMacAddress().c_str());
            PRINT_LOG(VERBOSE, "Set mac address: %s", PkgIpInfo.s_HwAddress);
        }

        PkgIpInfo.s_Dhcp = Iface.IsDHCP() ? 1 : 0;
        PRINT_LOG(VERBOSE, "Using dhcp: %s", PkgIpInfo.s_Dhcp ? "true" : "false");
        break;
    }

    char_t   * Ptr  = PkgIpInfo.s_Dns;
    uint32_t   Left = sizeof(PkgIpInfo.s_Dns);
    memset(Ptr, 0x00, Left);
    for (i = 0; i < NetInfo.GetDNSServerCount(); ++ i)
    {
        if (NetInfo.GetDNSServer(i).length() == 0)
        {
            continue;
        }

        if (NetInfo.GetDNSServer(i).length() + 1 > Left)
        {
            break;
        }

        snprintf(Ptr, Left, "%s ", NetInfo.GetDNSServer(i).c_str());

        Ptr += NetInfo.GetDNSServer(i).length() + 1;
        Left -= NetInfo.GetDNSServer(i).length() + 1;
    }
    
    PRINT_LOG(VERBOSE, "Set DNS list: %s", PkgIpInfo.s_Dns);

    // Configure device information
    RetVal = SysSetDeviceIP(m_SessionId, m_AuthValue, &PkgIpInfo);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to set network information");
        return RetVal;
    }

    return GMI_SUCCESS;
}

GMI_RESULT ConfigureService::SetDeviceInfo(const DeviceInfo & DevInfo, boolean_t UsingSupper)
{
    if (!Initialized())
    {
        PRINT_LOG(ERROR, "Configure service is not initialized yet");
        return GMI_INVALID_OPERATION;
    }

    GMI_RESULT RetVal = Login();
    if (RetVal != GMI_SUCCESS)
    {
        return RetVal;
    }

    SysPkgDeviceInfo PkgDevInfo;
    memset(&PkgDevInfo, 0x00, sizeof(PkgDevInfo));

    RetVal = SysGetDeviceInfo(m_SessionId, m_AuthValue, &PkgDevInfo);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to get device information");
        return RetVal;
    }

    snprintf(PkgDevInfo.s_DeviceName, sizeof(PkgDevInfo.s_DeviceName), "%s", DevInfo.GetName().c_str());
    PRINT_LOG(VERBOSE, "Set device name: %s", PkgDevInfo.s_DeviceName);

    if (UsingSupper)
    {
        snprintf(PkgDevInfo.s_DeviceSerialNum, sizeof(PkgDevInfo.s_DeviceSerialNum), "%s", DevInfo.GetSerialNumber().c_str());
        PRINT_LOG(VERBOSE, "Set device serial number: %s", PkgDevInfo.s_DeviceSerialNum);
    }

    // Configure device information
    RetVal = SysSetDeviceInfo(m_SessionId, m_AuthValue, &PkgDevInfo);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to set device information");
        return RetVal;
    }

    return GMI_SUCCESS;
}

GMI_RESULT ConfigureService::GetInterfaceInfo(int SockFd, InterfaceInfo & IfaceInfo) const
{
    ASSERT_GREATER_EQUAL(SockFd, 0);

    struct ifreq IfReq;

    // Initialize the struct, and fill the interface name
    InitInterfaceRequest(IfReq, IfaceInfo.GetName().c_str());

    // Call ioctl to get interface ip address
    if (ioctl(SockFd, SIOCGIFADDR, &IfReq) < 0)
    {
        PRINT_LOG(ERROR, "Failed to call ioctl, errno = %d", errno);
        return GMI_FAIL;
    }

    IfaceInfo.SetIpAddress(inet_ntoa(((struct sockaddr_in *)&IfReq.ifr_netmask)->sin_addr));

    // Initialize the struct, and fill the interface name
    InitInterfaceRequest(IfReq, IfaceInfo.GetName().c_str());

    // Call ioctl to get interface netmask
    if (ioctl(SockFd, SIOCGIFNETMASK, &IfReq) < 0)
    {
        PRINT_LOG(ERROR, "Failed to call ioctl, errno = %d", errno);
        return GMI_FAIL;
    }

    IfaceInfo.SetNetmask(inet_ntoa(((struct sockaddr_in *)&IfReq.ifr_netmask)->sin_addr));

    // Initialize the struct, and fill the interface name
    InitInterfaceRequest(IfReq, IfaceInfo.GetName().c_str());

    // Call ioctl to get interface mac address
    if (ioctl(SockFd, SIOCGIFHWADDR, &IfReq) < 0)
    {
        PRINT_LOG(ERROR, "Failed to call ioctl, errno = %d", errno);
        return GMI_FAIL;
    }

    IfaceInfo.SetMacAddress(MacAddr2String((uint8_t *)IfReq.ifr_hwaddr.sa_data));

    // Ignore the return value
    GetInterfaceGateway(IfaceInfo);
    GetInterfaceDHCPStatus(IfaceInfo);

    return GMI_SUCCESS;
}

GMI_RESULT ConfigureService::GetInterfaceGateway(InterfaceInfo & IfaceInfo) const
{
    FILE           * FilePtr = NULL;
    char_t           Buf[MAX_STRING_LENGTH];
    char_t           Iface[MAX_INTERFACE_NAME_LENGTH];
    uint32_t         DstAddr = INADDR_ANY;
    uint32_t         Gateway = INADDR_ANY;
    struct in_addr   InAddr;

    // Set default value
    IfaceInfo.SetGateway(std::string());

    FilePtr = fopen(ROUTE_FILE_PATH, "r");
    if (FilePtr == NULL)
    {
        PRINT_LOG(ERROR, "Failed to open '%s'", ROUTE_FILE_PATH);
        return GMI_FAIL;
    }

    // Skip title line
    if (fgets(Buf, sizeof(Buf), FilePtr) == NULL)
    {
        PRINT_LOG(ERROR, "Failed to get the title string");
        // Close file before we return
        fclose(FilePtr);
        return GMI_FAIL;
    }

    while (fgets(Buf, sizeof(Buf), FilePtr) != NULL)
    {
        if (sscanf(Buf, "%s\t%X\t%X", Iface, &DstAddr, &Gateway) != 3)
        {
            continue;
        }

        if (DstAddr != INADDR_ANY || IfaceInfo.GetName() != Iface)
        {
            continue;
        }

        // Find the gateway of this interface
        InAddr.s_addr = Gateway;
        IfaceInfo.SetGateway(inet_ntoa(InAddr));

        // Close file before we return
        fclose(FilePtr);
        return GMI_SUCCESS;
    }

    PRINT_LOG(WARNING, "Failed to find gateway for interface %s", IfaceInfo.GetName().c_str());

    // Close file before we return
    fclose(FilePtr);
    return GMI_FAIL;
}

GMI_RESULT ConfigureService::GetInterfaceDHCPStatus(InterfaceInfo & IfaceInfo) const
{
    FILE   * FilePtr = NULL;
    char_t   Buf[MAX_STRING_LENGTH];
    char_t   Iface[MAX_INTERFACE_NAME_LENGTH];
    char_t   Value[MAX_INTERFACE_NAME_LENGTH];

    // Set default value
    IfaceInfo.SetDHCPStatus(false);

    FilePtr = fopen(INTERFACE_FILE_PATH, "r");
    if (FilePtr == NULL)
    {
        PRINT_LOG(ERROR, "Failed to open '%s'", INTERFACE_FILE_PATH);
        return GMI_FAIL;
    }

    while (fgets(Buf, sizeof(Buf), FilePtr) != NULL)
    {
        if (sscanf(Buf, "iface %s inet %s", Iface, Value) != 2)
        {
            continue;
        }

        if (IfaceInfo.GetName() != Iface)
        {
            continue;
        }

        // Value should be 'dhcp' or 'static'
        IfaceInfo.SetDHCPStatus(0 == strcmp(Value, "dhcp"));

        // Close file before we return
        fclose(FilePtr);
        return GMI_SUCCESS;
    }

    PRINT_LOG(WARNING, "Failed to find dhcp status for interface %s", IfaceInfo.GetName().c_str());

    // Close file before we return
    fclose(FilePtr);
    return GMI_FAIL;
}

GMI_RESULT ConfigureService::ImportFile(const char_t * File, const char_t * Target, boolean_t Reserved, uint32_t Encrypt)
{
    if (NULL == File || NULL == Target)
    {
        PRINT_LOG(ERROR, "Invalid parameter");
        return GMI_INVALID_PARAMETER;
    }

    if (!Initialized())
    {
        PRINT_LOG(ERROR, "Configure service is not initialized yet");
        return GMI_INVALID_OPERATION;
    }

    SysPkgConfigFileInfo ImportInfo;
    memset(&ImportInfo, 0x00, sizeof(ImportInfo));

    // Fill the data structure
    strncpy(ImportInfo.s_FileTmpPath, File, sizeof(ImportInfo.s_FileTmpPath) - 1);
    strncpy(ImportInfo.s_FileTargetPath, Target, sizeof(ImportInfo.s_FileTargetPath) - 1);
    ImportInfo.s_Persit = Reserved ? 1 : 0;
    ImportInfo.s_Encrypt = Encrypt;

    DUMP_VARIABLE(ImportInfo.s_FileTmpPath);
    DUMP_VARIABLE(ImportInfo.s_FileTargetPath);
    DUMP_VARIABLE(ImportInfo.s_Persit);
    DUMP_VARIABLE(ImportInfo.s_Encrypt);

    GMI_RESULT RetVal = SysExecuteImportFile(m_SessionId, m_AuthValue, &ImportInfo);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(ERROR, "Failed to import file");
        return RetVal;
    }

    return GMI_SUCCESS;
}

GMI_RESULT ConfigureService::Login()
{
    if (Logined())
    {
        return GMI_SUCCESS;
    }

    char_t SupperUsername[] = GMI_SUPER_USER_NAME;
    char_t SupperPassword[] = GMI_SUPER_USER_PASSWD;

    GMI_RESULT RetVal = SysAuthLogin(SupperUsername, SupperPassword, 0, GetAuthLocalModuleId(), &m_SessionId, &m_UserFlag, &m_AuthValue);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to login supper user");
        return RetVal;
    }

    PRINT_LOG(VERBOSE, "Succeeded to login supper user");
    DUMP_VARIABLE(m_SessionId);
    DUMP_VARIABLE(m_UserFlag);
    DUMP_VARIABLE(m_AuthValue);

    m_Logined = true;

    return GMI_SUCCESS;
}

GMI_RESULT ConfigureService::Logout()
{
    if (!Logined())
    {
        return GMI_SUCCESS;
    }

    GMI_RESULT RetVal = SysAuthLogout(m_SessionId);
    if (RetVal != GMI_SUCCESS)
    {
        PRINT_LOG(WARNING, "Failed to logout supper user");
        return RetVal;
    }

    PRINT_LOG(VERBOSE, "Succeeded to logout supper user");

    m_SessionId = 0;
    m_UserFlag = 0;
    m_AuthValue = 0;
    m_Logined = false;

    return GMI_SUCCESS;
}

uint16_t ConfigureService::GetAuthLocalPort()
{
    return GMI_CONFIG_TOOL_AUTH_PORT;
}

uint8_t ConfigureService::GetAuthLocalModuleId()
{
    return ID_MOUDLE_REST_CONFIG;
}

uint16_t ConfigureService::GetDaemonRemotePort()
{
    return GMI_DAEMON_HEARDBEAT_SERVER;
}

uint16_t ConfigureService::GetDaemonLocalPort()
{
    return GMI_DAEMON_HEARTBEAT_CONFIG_TOOL;
}

uint16_t ConfigureService::GetDaemonServerPort()
{
    return GMI_DAEMON_HEARTBEAT_STATUS_QUERY;
}

int32_t ConfigureService::GetDaemonLocalModuleId()
{
    return CONFIG_TOOL_SERVER_ID;
}

