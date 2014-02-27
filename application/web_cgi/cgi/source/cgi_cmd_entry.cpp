#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgi_cmd_entry.h"
#include "cgi_cmd_excute.h"
#include "log.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

typedef GMI_RESULT (*CgiFncHandle) (const char_t *FncCmd);

typedef struct
{
    const char_t *s_CgiFncCmd;
    CgiFncHandle  s_CgiFncHandle;
} CgiFncHandler;

static CgiFncHandler WebFncTable[] =
{
    {"CheckSessionId",CgiCheckSessionId},
    {"LogIn",  CgiLogIn},
    {"LogOut", CgiLogOut},
    {"GetIpInfo", CgiGetIpInfo},
    {"SetIpInfo", CgiSetIpInfo},
    {"GetSysDeviceInfo", CgiGetSysDeviceInfo},
    {"SetSysDeviceInfo", CgiSetSysDeviceInfo},
    {"GetNetworkPort", CgiGetNetworkPort},
    {"SetNetworkPort", CgiSetNetworkPort},
    {"GetNtpServerInfo", CgiGetNtpServerInfo},
    {"SetNtpServerInfo", CgiSetNtpServerInfo},
    {"GetFtpServerInfo", CgiGetFtpServerInfo},
    {"SetFtpServerInfo", CgiSetFtpServerInfo},
    {"GetTime", CgiGetTime},
    {"SetTime", CgiSetTime},
    {"SystemRootCmd", CgiSystemRootCmd},
    {"SystemDefaultCmd",CgiSystemDefaultCmd},
    {"SystemSimpleDefaultCmd",CgiSystemSimpleDefaultCmd},
    {"GetUsers", CgiGetUsers},
    {"AddUsers", CgiAddUsers},
    {"ModifyUsers", CgiModifyUsers},
    {"SetUsers", CgiSetUsers},
    {"DelUsers", CgiDelUsers},
    {"GetEncodeStreamNum",CgiGetEncodeStreamNum},
    {"GetEncodeCfg", CgiGetEncodeCfg},
    {"SetEncodeCfg", CgiSetEncodeCfg},
    {"FactoryDefaultStreamCombine",CgiFactoryDefaultStreamCombine},
    {"FactoryDefaultVideoEncode",CgiFactoryDefaultVideoEncode},
    {"GetImageCfg", CgiGetImageCfg},
    {"SetImageCfg", CgiSetImageCfg},
    {"GetAdvancedImaging", CgiGetAdvancedImaging},
    {"SetAdvancedImaging", CgiSetAdvancedImaging},
    {"SetImagingDefaultMode", CgiSetImagingDefaultMode},
    {"GetCapabilities", CgiGetCapabilities},
    {"GetWorkState", CgiGetWorkState},
    {"GetAutoFocusMode", CgiGetAutoFocusMode},
    {"SetAutoFocusMode", CgiSetAutoFocusMode},
    {"FocusGlobalScan", CgiFocusGlobalScan},
    {"GetSystemAfFlags", CgiGetSystemAutoFocusIsValid},
    {"GetShowInfo", CgiGetShowInfo},
    {"SetShowInfo", CgiSetShowInfo},
    {"GetWhiteBalanceMode", CgiGetWhiteBalanceMode},
    {"SetWhiteBalanceMode", CgiSetWhiteBalanceMode},
    {"GetDayNightMode", CgiGetDayNightMode},
    {"SetDayNightMode", CgiSetDayNightMode},
    {"CheckIPExist", CgiCheckIPExist},
    {"GetVideoSourceMirror",CgiGetVideoSourceMirror},
    {"SetVideoSourceMirror",CgiSetVideoSourceMirror},
    {"GetVideoEncodeMirror",CgiGetVideoEncodeMirror},
    {"SetVideoEncodeMirror",CgiSetVideoEncodeMirror},
    {"GetVideoEncStreamCombine",CgiGetVideoEncStreamCombine},
    {"SetVideoEncStreamCombine",CgiSetVideoEncStreamCombine},
    {"GetAudioEncCfg",CgiGetAudioEncCfg},
    {"SetAudioEncCfg",CgiSetAudioEncCfg},
    {"GetUpdateCfg",CgiGetUpdateCfg},
    {"SystemRebootCmd",CgiSystemRebootCmd},
    {"FactoryDefault",CgiFactoryDefault},
    {"FactoryDefaultAll",CgiFactoryDefaultAll},
    {"SysSearchPtzPresetInfo",CgiSysSearchPtzPresetInfo},
    {"SysGetDeviceStartedTime",CgiSysGetDeviceStartedTime},
    {"ConfigToolGetDeviceInfo",CgiConfigToolGetDeviceInfo},
    {"ConfigToolSetDeviceInfo",CgiConfigToolSetDeviceInfo},
    {"ConfigToolSetIrCutStatus",CgiConfigToolSetIrCutStatus},
    {"ConfigToolGetRtcTime",CgiConfigToolGetRtcTime},
    {"ConfigToolTestWatchdog",CgiConfigToolTestWatchdog},
    {"ConfigToolOpenDcIris",CgiConfigToolOpenDcIris},
    {"ConfigToolCloseDcIris", CgiConfigToolCloseDcIris},
    {"ConfigToolTestAFConfigFile",CgiConfigToolTestAFConfigFile},
    {"ConfigToolGetMac",CgiConfigToolGetMac},
    {"ConfigToolSetMac",CgiConfigToolSetMac},
    {"ConfigToolIrCutOpen",CgiConfigToolIrCutOpen},
    {"ConfigToolIrCutClose",CgiConfigToolIrCutClose},
    {"ConfigToolGetSystemInfo",CgiConfigToolGetSystemInfo},
    {"SysGetLogInfo",CgiSysGetLogInfo},
    {"SysStop3A",CgiSysStop3A},
    {"SysCmdTest",CgiSysCmdTest},
    {"SysGetAlarmConfig",CgiSysGetAlarmConfig},
    {"SysSetAlarmInConfig",CgiSysSetAlarmInConfig},
    {"SysSetAlarmOutConfig",CgiSysSetAlarmOutConfig},
    {"SysSetAlarmPIRConfig",CgiSysSetAlarmPIRConfig},
    {"SysGetAlmScheduleTime",CgiSysGetAlmScheduleTime},
    {"SysSetAlmScheduleTime",CgiSysSetAlmScheduleTime},
    {}
};

GMI_RESULT CgiCmdProcess(const char_t *FncCmd)
{
    uint32_t Cnt;

    if (NULL == FncCmd)
    {
        return GMI_INVALID_PARAMETER;
    }

    for (Cnt = 0; Cnt < ARRAY_SIZE(WebFncTable); Cnt++)
    {
        if (0 == strcasecmp(FncCmd, WebFncTable[Cnt].s_CgiFncCmd))
        {
            CGI_ERROR("CgiCmdProcess  s_CgiFncCmd = %s  Cnt = %d  MaxCnt = %d\n", WebFncTable[Cnt].s_CgiFncCmd, Cnt, ARRAY_SIZE(WebFncTable));
            break;
        }
    }

    if (Cnt >= ARRAY_SIZE(WebFncTable))
    {
        return GMI_FAIL;
    }

    return (*(WebFncTable[Cnt].s_CgiFncHandle))(FncCmd);
}

