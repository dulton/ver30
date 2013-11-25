#ifndef __CONFIGURE_MANAGER_H__
#define __CONFIGURE_MANAGER_H__

#include <string>
#include <vector>
#include <map>

#include "common_def.h"

// Define network interface information
class InterfaceInfo
{
public:
    // Default constructor
    InterfaceInfo() {}

    // Constructor with interface name
    InterfaceInfo(const char * Name)
        : m_Name(Name)
        , m_Enabled(true)
        , m_DHCPStatus(false)
    {
    }

    // Copy constructor
    InterfaceInfo(const InterfaceInfo & Ins)
        : m_Name(Ins.m_Name)
        , m_IpAddress(Ins.m_IpAddress)
        , m_Netmask(Ins.m_Netmask)
        , m_Gateway(Ins.m_Gateway)
        , m_MacAddress(Ins.m_MacAddress)
        , m_Enabled(Ins.m_Enabled)
        , m_DHCPStatus(Ins.m_DHCPStatus)
    {
    }

    // Set attribute of this interface
    inline void_t SetName(const std::string & Name) { m_Name = Name; }
    inline void_t SetIpAddress(const std::string & IpAddress) { m_IpAddress = IpAddress; }
    inline void_t SetNetmask(const std::string & Netmask) { m_Netmask = Netmask; }
    inline void_t SetGateway(const std::string & Gateway) { m_Gateway = Gateway; }
    inline void_t SetMacAddress(const std::string & MacAddress) { m_MacAddress = MacAddress; }
    inline void_t SetEnabled(boolean_t Enabled) { m_Enabled = Enabled; }
    inline void_t SetDHCPStatus(boolean_t DHCPStatus) { m_DHCPStatus = DHCPStatus; }

    // Get attribute of this interface
    inline const std::string & GetName() const { return m_Name; }
    inline const std::string & GetIpAddress() const { return m_IpAddress; }
    inline const std::string & GetNetmask() const { return m_Netmask; }
    inline const std::string & GetGateway() const { return m_Gateway; }
    inline const std::string & GetMacAddress() const { return m_MacAddress; }
    inline boolean_t IsEnabled() const { return m_Enabled; }
    inline boolean_t IsDHCP() const { return m_DHCPStatus; }

private:
    // Attribute of network interface
    std::string m_Name;
    std::string m_IpAddress;
    std::string m_Netmask;
    std::string m_Gateway;
    std::string m_MacAddress;
    boolean_t   m_Enabled;
    boolean_t   m_DHCPStatus;
};

// Define network information
class NetworkInfo
{
public:
    // Initialize interface list
    inline void_t ClearInterfaceList() { m_InterfaceList.clear(); }
    // Initialize dns server list
    inline void_t ClearDNSServerList() { m_DNSServerList.clear(); }

    // Add one interface information into network information
    inline void_t AddInterface(const InterfaceInfo & Iface) { m_InterfaceList.push_back(Iface); }
    // Add one dns server into network information
    inline void_t AddDNSServer(const std::string & DNSServer) { m_DNSServerList.push_back(DNSServer); }

    // Get count of interfaces
    inline uint32_t GetInterfaceCount() const { return m_InterfaceList.size(); }
    // Get count of dns servers
    inline uint32_t GetDNSServerCount() const { return m_DNSServerList.size(); }

    // Get one interface information
    inline const InterfaceInfo & GetInterface(uint32_t Index) const
    {
        ASSERT_LESS(Index, m_InterfaceList.size());
        return m_InterfaceList[Index];
    }

    // Get one dns server
    inline const std::string & GetDNSServer(uint32_t Index) const
    {
        ASSERT_LESS(Index, m_DNSServerList.size());
        return m_DNSServerList[Index];
    }

private:
    std::vector<InterfaceInfo> m_InterfaceList;
    std::vector<std::string>   m_DNSServerList;
};

// Define device information
class DeviceInfo
{
public:
    // Set attribute of this instance
    inline void_t SetName(const std::string & Name) { m_Name = Name; }
    inline void_t SetSerialNumber(const std::string & SerialNumber) { m_SerialNumber = SerialNumber; }
    inline void_t SetHardwareVersion(const std::string & HardwareVersion) { m_HardwareVersion = HardwareVersion; }
    inline void_t SetFirmwareVersion(const std::string & FirmwareVersion) { m_FirmwareVersion = FirmwareVersion; }
    inline void_t SetLastBootTime(const std::string & LastBootTime) { m_LastBootTime = LastBootTime; }

    // Get attribute of this instance
    inline const std::string & GetName() const { return m_Name; }
    inline const std::string & GetSerialNumber() const { return m_SerialNumber; }
    inline const std::string & GetHardwareVersion() const { return m_HardwareVersion; }
    inline const std::string & GetFirmwareVersion() const { return m_FirmwareVersion; }
    inline const std::string & GetLastBootTime() const { return m_LastBootTime; }

private:
    // Attribute of device information
    std::string m_Name;
    std::string m_SerialNumber;
    std::string m_HardwareVersion;
    std::string m_FirmwareVersion;
    std::string m_LastBootTime;
};

// Define service ports
typedef std::map<std::string, uint16_t> ServicePorts;

// Define configure service
class ConfigureService : public Instance<ConfigureService>
{
friend class Instance<ConfigureService>;

public:
    inline boolean_t Initialized() const { return m_Initialized; }

    GMI_RESULT Initialize();
    GMI_RESULT Uninitialize();

    // Get information
    GMI_RESULT GetNetworkInfo(NetworkInfo & NetInfo) const;
    GMI_RESULT GetDeviceInfo(DeviceInfo & DevInfo);
    GMI_RESULT GetServicePorts(ServicePorts & Ports);

    // Set information
    GMI_RESULT SetNetworkInfo(const NetworkInfo & NetInfo, boolean_t UsingSupper = false);
    GMI_RESULT SetDeviceInfo(const DeviceInfo & DevInfo, boolean_t UsingSupper = false);

    // Import configure files
    GMI_RESULT ImportFile(const char_t * File, const char_t * Target, boolean_t Reserved, uint32_t Encrypt);

    // Get resources
    uint16_t GetAuthLocalPort();
    uint8_t GetAuthLocalModuleId();

    uint16_t GetDaemonRemotePort();
    uint16_t GetDaemonLocalPort();
    uint16_t GetDaemonRebootPort();
    int32_t GetDaemonLocalModuleId();

private:
    ConfigureService();
    ~ConfigureService();

    inline boolean_t Logined() const { return m_Logined; }

    GMI_RESULT Login();
    GMI_RESULT Logout();

    // Help to get interface information
    GMI_RESULT GetInterfaceInfo(int SockFd, InterfaceInfo & IfaceInfo) const;
    GMI_RESULT GetInterfaceGateway(InterfaceInfo & IfaceInfo) const;
    GMI_RESULT GetInterfaceDHCPStatus(InterfaceInfo & IfaceInfo) const;

    // Status of configure service
    boolean_t         m_Initialized;
    boolean_t         m_Logined;

    // User information
    uint32_t          m_AuthValue;
    uint16_t          m_SessionId;
    uint8_t           m_UserFlag;
};

#endif // __CONFIGURE_MANAGER_H__

