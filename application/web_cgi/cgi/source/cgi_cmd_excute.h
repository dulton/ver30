
#ifndef __CGI_CMD_EXCUTE_H__
#define __CGI_CMD_EXCUTE_H__

#define    MAX_XML_BUFFER_LENTH       16384
#define    MAX_CHAR_BUFFER_LENTH       4096
#define    MAX_BUFFER_LENTH       255
#define    MIN_BUFFER_LENTH        32
#define    CMD_BUFFER_LENTH       128


#include "gmi_system_headers.h"
//SessionId Check
GMI_RESULT CgiCheckSessionId(const char_t *FncCmd);
//system LogIn and LogOut
GMI_RESULT CgiLogIn(const char_t *FncCmd);
GMI_RESULT CgiLogOut(const char_t *FncCmd);
//system IP information setting and getting
GMI_RESULT CgiGetIpInfo(const char_t *FncCmd);
GMI_RESULT CgiSetIpInfo(const char_t *FncCmd);
//system network port information setting and getting
GMI_RESULT CgiGetNetworkPort(const char_t *FncCmd);
GMI_RESULT CgiSetNetworkPort(const char_t *FncCmd);
//system device information
GMI_RESULT CgiGetSysDeviceInfo(const char_t *FncCmd);
GMI_RESULT CgiSetSysDeviceInfo(const char_t *FncCmd);
//system NTP server information setting and getting
GMI_RESULT CgiGetNtpServerInfo(const char_t *FncCmd);
GMI_RESULT CgiSetNtpServerInfo(const char_t *FncCmd);
//system Ftp server infomation setting and getting
GMI_RESULT CgiGetFtpServerInfo(const char_t *FncCmd);
GMI_RESULT CgiSetFtpServerInfo(const char_t *FncCmd);
//system time setting and getting
GMI_RESULT CgiGetTime(const char_t *FncCmd);
GMI_RESULT CgiSetTime(const char_t *FncCmd);
//System Ctrl Commad type
GMI_RESULT CgiSystemRootCmd(const char_t *FncCmd);
GMI_RESULT CgiSystemDefaultCmd(const char_t *FncCmd);
GMI_RESULT CgiSystemSimpleDefaultCmd(const char_t *FncCmd);

//System User contrl
GMI_RESULT CgiGetUsers(const char_t *FncCmd);
GMI_RESULT CgiAddUsers(const char_t *FncCmd);
GMI_RESULT CgiModifyUsers(const char_t *FncCmd);
GMI_RESULT CgiSetUsers(const char_t *FncCmd);
GMI_RESULT CgiDelUsers(const char_t *FncCmd);

//Get Encode string Number
GMI_RESULT CgiGetEncodeStreamNum(const char_t * FncCmd);

//System Encode config
GMI_RESULT CgiGetEncodeCfg(const char_t *FncCmd);
GMI_RESULT CgiSetEncodeCfg(const char_t *FncCmd);

//System Image config
GMI_RESULT CgiGetImageCfg(const char_t *FncCmd);
GMI_RESULT CgiSetImageCfg(const char_t *FncCmd);

GMI_RESULT CgiFactoryDefaultStreamCombine(const char_t *FncCmd);
GMI_RESULT CgiFactoryDefaultVideoEncode(const char_t *FncCmd);


//AdvancedImage
GMI_RESULT CgiGetAdvancedImaging(const char_t *FncCmd);
GMI_RESULT CgiSetAdvancedImaging(const char_t *FncCmd);

//Set Image to default mode
GMI_RESULT CgiSetImagingDefaultMode(const char_t *FncCmd);

//System xml config
GMI_RESULT CgiGetCapabilities(const char_t *FncCmd);
GMI_RESULT CgiGetWorkState(const char_t *FncCmd);

//System Af Mode
GMI_RESULT CgiGetAutoFocusMode(const char_t *FncCmd);
GMI_RESULT CgiSetAutoFocusMode(const char_t *FncCmd);

GMI_RESULT CgiFocusGlobalScan(const char_t *FncCmd);
GMI_RESULT CgiGetSystemAutoFocusIsValid(const char_t *FncCmd);

//Osd Config
GMI_RESULT CgiGetShowInfo(const char_t *FncCmd);
GMI_RESULT CgiSetShowInfo(const char_t *FncCmd);

//WhiteBalance Mode Setting and Getting
GMI_RESULT CgiGetWhiteBalanceMode(const char_t *FncCmd);
GMI_RESULT CgiSetWhiteBalanceMode(const char_t *FncCmd);

//WhiteBalance Mode Setting and Getting
GMI_RESULT CgiGetDayNightMode(const char_t *FncCmd);
GMI_RESULT CgiSetDayNightMode(const char_t *FncCmd);

//Check IPaddr is Exist
GMI_RESULT CgiCheckIPExist(const char_t *FncCmd);

//Video Source Mirror
GMI_RESULT CgiSetVideoSourceMirror(const char_t *FncCmd);
GMI_RESULT CgiGetVideoSourceMirror(const char_t *FncCmd);

//Video Encode Mirror
GMI_RESULT CgiGetVideoEncodeMirror(const char_t *FncCmd);
GMI_RESULT CgiSetVideoEncodeMirror(const char_t *FncCmd);

//Video Encode Stream Combine
GMI_RESULT CgiGetVideoEncStreamCombine(const char_t *FncCmd);
GMI_RESULT CgiSetVideoEncStreamCombine(const char_t *FncCmd);

//audio config
GMI_RESULT CgiGetAudioEncCfg(const char_t *FncCmd);
GMI_RESULT CgiSetAudioEncCfg(const char_t *FncCmd);

GMI_RESULT CgiGetUpdateCfg(const char_t *FncCmd);

GMI_RESULT CgiSystemRebootCmd(const char_t *FncCmd);
GMI_RESULT CgiFactoryDefault(const char_t *FncCmd);
GMI_RESULT CgiFactoryDefaultAll(const char_t *FncCmd);

GMI_RESULT CgiSysSearchPtzPresetInfo(const char_t *FncCmd);

GMI_RESULT CgiSysGetDeviceStartedTime(const char_t *FncCmd);

//manufacture config tool api
GMI_RESULT CgiConfigToolGetDeviceInfo(const char_t *FncCmd);
GMI_RESULT CgiConfigToolSetDeviceInfo(const char_t *FncCmd);
GMI_RESULT CgiConfigToolSetIrCutStatus(const char_t *FncCmd);
GMI_RESULT CgiConfigToolGetRtcTime(const char_t *FncCmd);
GMI_RESULT CgiConfigToolTestWatchdog(const char_t *FncCmd);
GMI_RESULT CgiConfigToolOpenDcIris(const char_t *FncCmd);
GMI_RESULT CgiConfigToolCloseDcIris(const char_t *FncCmd);
GMI_RESULT CgiConfigToolGetMac(const char_t *FncCmd);
GMI_RESULT CgiConfigToolSetMac(const char_t *FncCmd);
GMI_RESULT CgiConfigToolTestAFConfigFile(const char_t *FncCmd);
GMI_RESULT CgiConfigToolIrCutOpen(const char_t *FncCmd);
GMI_RESULT CgiConfigToolIrCutClose(const char_t *FncCmd);
GMI_RESULT CgiSysGetLogInfo(const char_t *FncCmd);

#endif
