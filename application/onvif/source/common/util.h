#ifndef __UTIL_H__
#define __UTIL_H__

#define MAX_NICS (4)

typedef struct tag_nic_info
{
    char ifname[20];
    char mac[20];
    char ip[20];
    char mask[20];
    char gateway[20];
} nic_info_t;


int get_active_nic_info(nic_info_t nic_info[MAX_NICS]);


int get_slave_ifcnamelist(char *ifcnamelist, int len);


int get_gateway(char *gateway, const char *ifname);


int get_hostip_by_ip(const char *ip, char *hostip);




#endif
