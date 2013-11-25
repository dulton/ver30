#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
/*#include <linux/fb.h>*/
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <net/if.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "util.h"

#define BONDING_DIR ("/proc/net/bonding")
#define SLAVE_IFC_MARK ("Slave Interface: ")
//#define BONDING_DIR (".")

int get_slave_ifcnamelist(char *ifcnamelist, int len)
{
    struct dirent **namelist;
    int n = 0;
    FILE *fp = NULL;
//	char ifcnamelist[512] = {0};
//	int  len = sizeof(ifcnamelist) - 1;
    n = scandir(BONDING_DIR, &namelist, 0, alphasort);
    if (n < 0)
    {
        return -1;
    }
    else
    {
        while (n--)
        {
            //ignore . and .. directory
            if (namelist[n]->d_name[0] != '.')
            {
                char fullpath[256];
                struct stat st;
                memset(&st, 0, sizeof(struct stat));
                memset(fullpath, 0, sizeof(fullpath));
                snprintf(fullpath, sizeof(fullpath) - 1, "%s/%s", BONDING_DIR, namelist[n]->d_name);
                //get file stat info
                if (-1 == stat(fullpath, &st))
                {
                    continue;
                }

                //ignore dir
                if (S_IFDIR & st.st_mode)
                {
                    continue;
                }

                //get file get slave ifcname
                fp = fopen(fullpath, "rb");
                if (NULL == fp)
                {
                    continue;
                }
                else
                {
                    //parse file
                    char buf[256];
                    memset(buf, 0, sizeof(buf));
                    while (NULL != fgets(buf, sizeof(buf) - 1, fp))
                    {
                        //check slave ifc mark
                        char *p = strstr(buf, SLAVE_IFC_MARK);
                        if (NULL != p)
                        {
                            char ifcname[20];
                            char *tmp = NULL;
                            memset(ifcname, 0, sizeof(ifcname));
                            p += strlen(SLAVE_IFC_MARK);
                            //ignore 0x0d
                            if (NULL != (tmp = strrchr(p, '\n')))
                            {
                                *tmp = '\0';
                            }
                            strncpy(ifcname, p, sizeof(ifcname) - 1);
                            if (len > 0)
                            {
                                //copy to ifcnamelist
                                tmp = strncat(ifcnamelist, ifcname, len);
                                len -= strlen(tmp);
                            }
                        }
                    }//endof while (NULL != fgets(buf, sizeof(buf) - 1, fp))
                }

                //close file
                if (NULL != fp)
                {
                    fclose(fp);
                    fp = NULL;
                }
                printf("filename=%s,fullpath=%s.............\n", namelist[n]->d_name, fullpath);
            }//endof if (namelist[n]->d_name[0] != '.')
            free(namelist[n]);
        }//endof while (n--)
        free(namelist);
    }//endof else

    printf("ifcnamelist=%s.............\n", ifcnamelist);
    return 0;
}

//get nic info
int get_active_nic_info(nic_info_t nic_info[MAX_NICS])
{
    int skfd = -1, if_cnt = 0, if_idx = 0;
    int idex = 0;
    struct ifreq if_buf[MAX_NICS + 4];
    struct ifconf ifc;
//	struct list_head *ip_info_list = NULL;
//	struct sockaddr_in data;
    char ifcnamelist[512] = {0};
    int  len = sizeof(ifcnamelist) - 1;

    //get slave ifcname list
    get_slave_ifcnamelist(ifcnamelist, len);

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("Get NIC info, Create socket error.......\n");
        return -1;
    }

    ifc.ifc_len = sizeof(if_buf);
    ifc.ifc_buf = (char *)if_buf;
    if (!ioctl(skfd, SIOCGIFCONF, (char *)&ifc))
    {
        if_cnt = (ifc.ifc_len) / (sizeof(struct ifreq));
        //if_cnt = min(if_cnt, MAX_NICS);
        //printf("if_cnt=%d.........\n", if_cnt);

        for (if_idx = 0; if_idx < if_cnt; ++if_idx)
        {
            /* ignore local loopback interface */
            if (strcmp("lo", if_buf[if_idx].ifr_name) == 0)
            {
                continue;
            }

            //ignore slve ifcname when in bonding
            if (0 != strlen(ifcnamelist))
            {
                if (NULL != strstr(ifcnamelist, if_buf[if_idx].ifr_name))
                {
                    continue;
                }
            }

            /* get interface flags */
            if (!(ioctl(skfd, SIOCGIFFLAGS, (char *)(&if_buf[if_idx]))))
            {
                char ip[20] = {0};
                char mask[20] = {0};
                char ifname[20] = {0};
                struct sockaddr_in *addr = (struct sockaddr_in *)&if_buf[if_idx].ifr_addr;
                memcpy(ifname, if_buf[if_idx].ifr_name, sizeof(ifname) - 1);
                //printf("ifname=%s.....\n", ifname);
                inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
                //printf("ip=%s.................\n", ip);
                memcpy(nic_info[idex].ifname, ifname, sizeof(nic_info[idex].ifname) - 1);
                memcpy(nic_info[idex].ip, ip, sizeof(nic_info[idex].ip) - 1);

                /*get mask*/
                if (!(ioctl(skfd, SIOCGIFNETMASK, (char *)(&if_buf[if_idx]))))
                {
                    inet_ntop(AF_INET, &addr->sin_addr, mask, sizeof(mask));
                    //printf("mask=%s.................\n", mask);
                    memcpy(nic_info[idex].mask, mask, sizeof(nic_info[idex].mask) - 1);
                }

                /*get mac*/
                if (!(ioctl(skfd, SIOCGIFHWADDR, (char *)(&if_buf[if_idx]))))
                {
                    char temp[20];
                    memset(temp, 0, sizeof(temp));
                    memcpy(temp, (char *)(&(if_buf[if_idx].ifr_hwaddr.sa_data)), sizeof(temp) - 1);
                    //printf("mac=%02x:%02x:%02x:%02x:%02x:%02x..........\n", (unsigned char)temp[0],(unsigned char)temp[1], (unsigned char)temp[2],
                    //		(unsigned char)temp[3], (unsigned char)temp[4], (unsigned char)temp[5]);
                    memcpy(nic_info[idex].mac, temp, sizeof(nic_info[idex].mac) - 1);
                }

                {
                    /*get gateway*/
                    char gateway[20];
                    memset(gateway, 0, sizeof(gateway));
                    strcpy(gateway, "0.0.0.0");
                    get_gateway(gateway, ifname);
                    //printf("gateway=%s............\n", gateway);
                    memcpy(nic_info[idex].gateway, gateway, sizeof(nic_info[idex].gateway) - 1);
                }

                if (++idex >= MAX_NICS)
                {
                    break;
                }
            }

        }	/* end of for (if_idx = 0; if_idx < if_cnt; ++if_idx) */
    }

    close(skfd);
    skfd = -1;

    return 0;
}


int get_hostip_by_ip(const char *ip, char *hostip)
{
    int ret = -1;
    nic_info_t nic_info[MAX_NICS];
    unsigned int i = 0;
    unsigned long host_ip = 0;
    unsigned long host_mask = 0;
    unsigned long client_ip = 0;

    if ((NULL == ip) || (0 == strlen(ip)) || (NULL == hostip))
    {
        goto errExit;
    }

    *hostip = '\0';
    memset(nic_info, 0, sizeof(nic_info));

    ret = get_active_nic_info(nic_info);
    if (0 != ret)
    {
        printf("get active nic info fail..........................\n");
        goto errExit;
    }

    client_ip = inet_addr(ip);
    for (i = 0; i < MAX_NICS && strlen(nic_info[i].ip); ++i)
    {
        host_ip = inet_addr(nic_info[i].ip);
        host_mask = inet_addr(nic_info[i].mask);

        if ((host_mask & host_ip) == (host_mask & client_ip))
        {
            strcpy(hostip, nic_info[i].ip);
            break;
        }
    }//endof for loop


    if (0 == strlen(hostip))
    {
        strcpy(hostip, nic_info[0].ip);
    }

    //set success flag
    ret = 0;

errExit:

    return ret;
}


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <asm/types.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <stdlib.h>

#define BUFSIZE (4096)
struct route_info
{
    u_int dstAddr;
    u_int srcAddr;
    u_int gateWay;
    char ifName[IF_NAMESIZE];
};

static int readNlSock(int sockFd, char *bufPtr, int seqNum, int pId)
{
    struct nlmsghdr *nlHdr = NULL;
    int readLen = 0;
    int msgLen = 0;

    do
    {
        if ((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0)
        {
            perror("SOCK READ: ");
            return -1;
        }

        nlHdr = (struct nlmsghdr *)bufPtr;

        if ((NLMSG_OK(nlHdr, (unsigned int)readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR))
        {
            perror("Error in recieved packet");
            return -1;
        }

        /* Check if the its the last message */
        if (nlHdr->nlmsg_type == NLMSG_DONE)
        {
            break;
        }
        else
        {
            /* Else move the pointer to buffer appropriately */
            bufPtr += readLen;
            msgLen += readLen;
        }

        /* Check if its a multi part message */
        if ((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0)
        {
            /* return if its not */
            break;
        }
    }
    while ((nlHdr->nlmsg_seq != (unsigned int)seqNum) || (nlHdr->nlmsg_pid != (unsigned int)pId));
    return msgLen;
}


static void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo, char *gateway, const char *ifname)
{
    struct rtmsg *rtMsg = NULL;
    struct rtattr *rtAttr = NULL;
    int rtLen = 0;
    //2007-12-10
    struct in_addr dst;
    struct in_addr gate;

    rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

    // If the route is not for AF_INET or does not belong to main routing table
    //then return.
    if ((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
    {
        return;
    }

    /* get the rtattr field */
    rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
    rtLen = RTM_PAYLOAD(nlHdr);

    while (RTA_OK(rtAttr,rtLen))
    {
        switch (rtAttr->rta_type)
        {
        case RTA_OIF:
            if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
            break;

        case RTA_GATEWAY:
            rtInfo->gateWay = *(u_int *)RTA_DATA(rtAttr);
            break;

        case RTA_PREFSRC:
            rtInfo->srcAddr = *(u_int *)RTA_DATA(rtAttr);
            break;

        case RTA_DST:
            rtInfo->dstAddr = *(u_int *)RTA_DATA(rtAttr);
            break;

        default:
            break;
        }

        //next
        rtAttr = RTA_NEXT(rtAttr,rtLen);
    }//endof while loop

    //2007-12-10
    dst.s_addr = rtInfo->dstAddr;
    if (strstr((char *)inet_ntoa(dst), "0.0.0.0") && !strcmp(rtInfo->ifName, ifname))
    {
        gate.s_addr = rtInfo->gateWay;
        sprintf(gateway, (char *)inet_ntoa(gate));
    }

}


int get_gateway(char *gateway, const char *ifname)
{
    struct nlmsghdr *nlMsg = NULL;
    //struct rtmsg *rtMsg = NULL;
    struct route_info *rtInfo = NULL;
    char msgBuf[BUFSIZE] = {0};

    int sock = -1;
    int len = 0;
    int msgSeq = 0;
    int ret = -1;


    if ((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
    {
        printf("Socket Creation:\n");
        goto errorExit;
    }

    /* Initialize the buffer */
    memset(msgBuf, 0, BUFSIZE);

    /* point the header and the msg structure pointers into the buffer */
    nlMsg = (struct nlmsghdr *)msgBuf;
    //rtMsg = (struct rtmsg *)NLMSG_DATA(nlMsg);

    /* Fill in the nlmsg header*/
    nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)); // Length of message.
    nlMsg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .

    nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
    nlMsg->nlmsg_seq = msgSeq++; // Sequence of the message packet.
    nlMsg->nlmsg_pid = getpid(); // PID of process sending the request.

    /* Send the request */
    if (send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0)
    {
        printf("Write To Socket Failed...\n");
        goto errorExit;
    }

    /* Read the response */
    if ((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0)
    {
        printf("Read From Socket Failed...\n");
        goto errorExit;
    }

    /* Parse and print the response */
    rtInfo = (struct route_info *)malloc(sizeof(struct route_info));
    if (NULL == rtInfo)
    {
        goto errorExit;
    }

    while (NLMSG_OK(nlMsg,(unsigned int)len))
    {
        memset(rtInfo, 0, sizeof(struct route_info));
        parseRoutes(nlMsg, rtInfo, gateway, ifname);

        //next
        nlMsg = NLMSG_NEXT(nlMsg,len);
    }//endof while loop

    //set successful flag
    ret = 0;

errorExit:

    if (NULL != rtInfo)
    {
        free(rtInfo);
        rtInfo = NULL;
    }

    if (-1 != sock)
    {
        close(sock);
        sock = -1;
    }

    return ret;
}

