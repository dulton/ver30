#ifndef __NET_MANAGER_H__
#define __NET_MANAGER_H__

#include "gmi_netconfig_api.h"

GMI_RESULT CheckIPv4String(char_t *Ip);
GMI_RESULT CheckIPv4MaskString(char_t *Mask);
GMI_RESULT CheckIPv4Config(char_t *IpString, char_t *MaskString, char_t *GatewayString);
GMI_RESULT CheckMacString(char_t *MacString);
GMI_RESULT NetReadMacChar(char_t EthName[32], char_t Mac[6]);
GMI_RESULT NetReadMac( long_t NetId, char_t *MacPtr );
GMI_RESULT NetWriteMac( long_t NetId, char_t *MacPtr );
GMI_RESULT NetReadIP( long_t NetId, IpInfo *IpInfoPtr );
GMI_RESULT NetWriteIP( IpInfo *IpInfoPtr );
GMI_RESULT NetActivate( long_t NetId );
GMI_RESULT NetFacotryDefault();

#endif


