#ifndef __UTILITLY_H__
#define __UTILITLY_H__
#include <assert.h>
#include <search.h>
#include "gmi_system_headers.h"
#include "sys_env_types.h"

GMI_RESULT InitCgi(char_t *Query);

char_t *GetCgi(char_t *Name);

GMI_RESULT NET_GetIpInfo(char_t *EthName, char_t *IP);

GMI_RESULT NET_GetMacInfo(char_t *EthName, char_t *Mac);

GMI_RESULT GMI_StrRpl(char_t* DstOut, char_t* SrcIn, const char_t* SrcRpl, const char_t* DstRpl);

GMI_RESULT GMI_SysCheckSessionId(uint16_t SessionId);

GMI_RESULT GMI_CheckAutoFlags(boolean_t *AutoFlags);

GMI_RESULT CheckIpExist(const char_t* InData);

GMI_RESULT GMI_GetUpdatePort(int32_t  *UpgradePort);

GMI_RESULT FactoryDefault(void);

GMI_RESULT GMI_WriteNetDefaultCfg(void);

#define WEB_GET_VAR(var) (GetCgi(var))

#endif
