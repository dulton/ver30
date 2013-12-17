#ifndef __SYS_CLIENT_API_H__
#define __SYS_CLIENT_API_H__

#include "sys_env_types.h"
#include "auth_center_api.h"
#include "gmi_system_headers.h"

GMI_RESULT SysInitialize(uint16_t LocalAuthRudpPort);
GMI_RESULT SysInitializeExt(struct timeval *TimeoutPtr, uint16_t TryCount);
GMI_RESULT SysDeinitialize();

/*
*in:SysLogInfoSearch
*out:SysLogInfoInt, SysLogInfo
*/
GMI_RESULT SysGetLogInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgLogInfoSearch *SysLogInfoSearch, SysPkgLogInfoInt *SysLogInfoInt, SysPkgLogInfo *SysLogInfo);

/*************************************************
  *ModuleId:
                "auth_center_api.h" :
                 #define ID_MOUDLE_REST_SDK          0x1   //sdk server reset linknum and sessionId
                 #define ID_MOUDLE_REST_ONVIF        0x2  //onvif server
                 #define ID_MOUDLE_REST_GB             0x3   //GB
                 #define ID_MOUDLE_REST_WEB           0x4   //WEB
  *------------------------------------------------------*/
GMI_RESULT SysAuthLogin(char_t  UserName[32],
                        char_t    UserPasswd[32],
                        uint16_t  InSessionId,
                        uint8_t   ModuleId,
                        uint16_t *OutSessionIdPtr,
                        uint8_t  *UserFlagPtr,
                        uint32_t *AuthvaluePtr
                       );
GMI_RESULT SysAuthLogout(uint16_t SessionId);
/*************************************************
  *SessionId: after AuthLogin() , get "OutSessionId"
  *Valid:true, SessionId is valid; Valid:false, SessionId is invalid.
  *------------------------------------------------------*/
GMI_RESULT SysCheckSessionId(uint16_t SessionId, boolean_t *Valid);

GMI_RESULT SysGetDeviceIP(uint16_t SessionId, uint32_t AuthValue, SysPkgIpInfo *SysPkgIpInfoPtr);
GMI_RESULT SysSetDeviceIP(uint16_t SessionId, uint32_t AuthValue, SysPkgIpInfo *SysPkgIpInfoPtr);

GMI_RESULT SysGetDeviceInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgDeviceInfo *SysPkgDeviceInfoPtr);
GMI_RESULT SysSetDeviceInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgDeviceInfo *SysPkgDeviceInfoPtr);

GMI_RESULT FactorySimpleDefaultAll(uint16_t SessionId, uint32_t AuthValue);
GMI_RESULT FactoryDefaultAll(uint16_t SessionId, uint32_t AuthValue);
GMI_RESULT FactoryDefaultImaging(uint16_t SessionId, uint32_t AuthValue);
GMI_RESULT FactoryDefaultStreamCombine(uint16_t SessionId, uint32_t AuthValue);
GMI_RESULT FactoryDefaultVideoEncode(uint16_t SessionId, uint32_t AuthValue);
GMI_RESULT SysReboot(uint16_t SessionId, uint32_t AuthValue);
GMI_RESULT SysExecuteImportFile(uint16_t SessionId, uint32_t AuthValue, SysPkgConfigFileInfo *SysConfigFileInfoPtr);

GMI_RESULT SysGetTime(uint16_t SessionId, uint32_t AuthValue, SysPkgDateTimeType *SysDateTimeTypePtr, SysPkgSysTime *SysTimePtr, SysPkgTimeZone *SysTimezonePtr, SysPkgNtpServerInfo *SysNtpServerInfoPtr);
GMI_RESULT SysSetTime(uint16_t SessionId, uint32_t AuthValue, SysPkgDateTimeType *SysDateTimeTypePtr, SysPkgSysTime *SysTimePtr, SysPkgTimeZone *SysTimezonePtr, SysPkgNtpServerInfo *SysNtpServerInfoPtr);

GMI_RESULT SysGetTimezone(uint16_t SessionId, uint32_t AuthValue, SysPkgTimeZone *SysTimezonePtr);
GMI_RESULT SysGetNtpServer(uint16_t SessionId, uint32_t AuthValue, SysPkgNtpServerInfo *SysNtpServerInfoPtr);

GMI_RESULT SysGetUsers(uint16_t SessionId, uint32_t AuthValue, SysPkgUserInfo *SysUserInfoPtr, uint32_t UserCnt, uint32_t *RealUserCnt);
GMI_RESULT SysSetUsers(uint16_t SessionId, uint32_t AuthValue, SysPkgUserInfo *SysUserInfoPtr);
GMI_RESULT SysDelUsers(uint16_t SessionId, uint32_t AuthValue, SysPkgUserInfo *SysUserInfoPtr);
/**********************************************************************
   *in  :  SysUserInfoPtr userinfo
   *return: GMI_INVALID_OPERATION means userinfo not exist.
   *-------------------------------------------------------------------------------*/
GMI_RESULT SysModifyUsers(uint16_t SessionId, uint32_t AuthValue, SysPkgUserInfo *SysUserInfoPtr);
/**********************************************************************
    *in  :  SysUserInfoPtr userinfo
    *return: GMI_INVALID_OPERATION means userinfo have already exist.
    *-------------------------------------------------------------------------------*/
GMI_RESULT SysAddUsers(uint16_t SessionId, uint32_t AuthValue, SysPkgUserInfo *SysUserInfoPtr);


GMI_RESULT SysGetNetworkPort(uint16_t SessionId, uint32_t AuthValue, SysPkgNetworkPort *SysNetworkPortPtr);
GMI_RESULT SysSetNetworkPort(uint16_t SessionId, uint32_t AuthValue, SysPkgNetworkPort *SysNetworkPortPtr);

GMI_RESULT SysGetCapabilities(uint16_t SessionId, uint32_t AuthValue, char_t* Message, int32_t MessageLength, SysPkgXml *SysCapabilities);
GMI_RESULT SysGetWorkState(uint16_t SessionId, uint32_t AuthValue, char_t* Message, int32_t MessageLength, SysPkgXml *SysWorkState);

GMI_RESULT SysGetEncodeStreamNum(uint16_t SessionId, uint32_t AuthValue, uint32_t *StreamNum);
GMI_RESULT SysGetEncodeCfg(uint16_t SessionId, uint32_t AuthValue, SysPkgEncodeCfg *SysEncodeCfgPtr, uint32_t ReqEncodeCfgNum, uint32_t *RspEncodeCfgNum);
GMI_RESULT SysSetEncodeCfg(uint16_t SessionId, uint32_t AuthValue, int32_t StreamId, SysPkgEncodeCfg *SysEncodeCfgPtr);
GMI_RESULT SysGetVideoEncStreamCombine(uint16_t SessionId, uint32_t AuthValue, SysPkgEncStreamCombine *SysEncStreamCombinePtr);
GMI_RESULT SysSetVideoEncStreamCombine(uint16_t SessionId, uint32_t AuthValue, SysPkgEncStreamCombine *SysEncStreamCombinePtr);
/**********************************************************************
   *in  :  Mirror : default-0(nomal), 1-horizontal flip, 2-vertical flip, 3-horizontal&vertical flip
   *        StreamId: 0,1,2,3
   *-------------------------------------------------------------------------------*/
GMI_RESULT SysGetVideoEncodeMirror(uint16_t SessionId, uint32_t AuthValue, int32_t StreamId, int32_t *Mirror);
GMI_RESULT SysSetVideoEncodeMirror(uint16_t SessionId, uint32_t AuthValue, int32_t StreamId, int32_t Mirror);

GMI_RESULT SysGetVideoSource(uint16_t SessionId, uint32_t AuthValue, SysPkgVideoSource *SysVideoSource);
/**********************************************************************
   *in  :  Mirror : default-0(nomal), 1-horizontal flip, 2-vertical flip, 3-horizontal&vertical flip
   *-------------------------------------------------------------------------------*/
GMI_RESULT SysGetVideoSourceMirror(uint16_t SessionId, uint32_t AuthValue, int32_t *MirrorPtr);
GMI_RESULT SysSetVideoSourceMirror(uint16_t SessionId, uint32_t AuthValue, int32_t Mirror);
/**********************************************************************
   *in  :  Mirror : default-0(nomal), 1-horizontal flip, 2-vertical flip, 3-horizontal&vertical flip
   *-------------------------------------------------------------------------------*/
GMI_RESULT SysGetVideoSourceMirror(uint16_t SessionId, uint32_t AuthValue, int32_t *MirrorPtr);
GMI_RESULT SysSetVideoSourceMirror(uint16_t SessionId, uint32_t AuthValue, int32_t Mirror);

GMI_RESULT SysGetImaging(uint16_t SessionId, uint32_t AuthValue, SysPkgImaging *SysImagingPtr);
GMI_RESULT SysSetImaging(uint16_t SessionId, uint32_t AuthValue, SysPkgImaging *SysImagingPtr);
GMI_RESULT SysGetWhiteBalance(uint16_t SessionId, uint32_t AuthValue, SysPkgWhiteBalance *SysWhiteBalancePtr);
GMI_RESULT SysSetWhiteBalance(uint16_t SessionId, uint32_t AuthValue, SysPkgWhiteBalance *SysWhiteBalancePtr);
GMI_RESULT SysGetDaynight(uint16_t SessionId, uint32_t AuthValue, SysPkgDaynight *SysDaynightPtr);
GMI_RESULT SysSetDaynight(uint16_t SessionId, uint32_t AuthValue, SysPkgDaynight *SysDaynightPtr);

GMI_RESULT SysForceIdr(uint16_t SessionId, uint32_t AuthValue, int32_t StreamId);

GMI_RESULT SysGetAdvancedImaging(uint16_t SessionId, uint32_t AuthValue, SysPkgAdvancedImaging *SysAdvancedImagingPtr);
GMI_RESULT SysSetAdvancedImaging(uint16_t SessionId, uint32_t AuthValue, SysPkgAdvancedImaging *SysAdvancedImagingPtr);

GMI_RESULT SysPtzControl(uint16_t SessionId, uint32_t AuthValue, SysPkgPtzCtrl *SysPkgPtzCtrlPtr);
GMI_RESULT SysGetAutoFocusCfg(uint16_t SessionId, uint32_t AuthValue, SysPkgAutoFocus *SysAutoFocusPtr);
GMI_RESULT SysSetAutoFocusCfg(uint16_t SessionId, uint32_t AuthValue, SysPkgAutoFocus *SysAutoFocusPtr);
GMI_RESULT SysFocusGlobalScan(uint16_t SessionId, uint32_t AuthValue);
GMI_RESULT SysSearchPtzPresetInfo(uint16_t SessionId, uint32_t AuthValue, uint32_t PresetIndex, boolean_t *Setted,  char_t PresetName[128]);
GMI_RESULT SysSetPtzPresetInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgPtzPresetInfo *SysPtzPresetInfoPtr);
GMI_RESULT SysGetMaxPresetNum(uint16_t SessionId, uint32_t AuthValue, int32_t *MaxNumPtr);
GMI_RESULT SysGetPtzPresetInfo(uint16_t SessionId, uint32_t AuthValue, int32_t MaxNum, SysPkgPtzPresetInfo *SysPtzPresetInfoPtr, int32_t *RspNum);

GMI_RESULT SysGetShowInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgShowCfg *SysShowInfoPtr, uint32_t ReqShowInfoNum, uint32_t *RspShowInfoNum);
GMI_RESULT SysSetShowInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgShowCfg *SysShowInfoPtr);

GMI_RESULT SysGetDeviceStartedTime(SysPkgSysTime *DeviceStartedTimePtr);

//audio start & stop
GMI_RESULT SysStartAudioDecode(uint16_t SessionId, uint32_t AuthValue, SysPkgAudioDecParam *SysAudioDecParamPtr);
GMI_RESULT SysStopAudioDecode(uint16_t SessionId, uint32_t AuthValue, int32_t AudioId);
//audio config,
//EncodeType:1--G.711A,2--G.711U,3--G.726, at present, just support G.711A
//CapVolume, PlayVolume:10%,20%,30%,40%,50%,60%...100%, default 100%.
//date:10/12/2013
GMI_RESULT SysGetAudioEncCfg(uint16_t SessionId, uint32_t AuthValue, uint32_t *AudioId, uint8_t *EncodeType, uint16_t *CapVolume, uint16_t *PlayVolume);
GMI_RESULT SysSetAudioEncCfg(uint16_t SessionId, uint32_t AuthValue, uint32_t AudioId, uint8_t EncodeType, uint16_t CapVolume, uint16_t PlayVolume);

GMI_RESULT SysGetHostName(char_t HostName[128]);
GMI_RESULT SysSetHostName(char_t HostName[128]);

//Local api factory simple default and factory all default
GMI_RESULT FactorySimpleDefaultAllLocal(uint16_t SessionId, uint32_t AuthValue);
GMI_RESULT FactoryDefaultAllLocal(uint16_t SessionId, uint32_t AuthValue);

// Local set System Mac
GMI_RESULT GMI_ConfigToolSetMac(uint16_t SessionId, uint32_t AuthValue,const char_t *MacAddrs);

// Detect systm Af config file is exists
//if FileFlags is 1 ,config file exists. else if FileFlags is 0 ,config file not exists
GMI_RESULT GMI_ConfigToolAfConfigDetect(uint16_t SessionId, uint32_t AuthValue,int32_t *FileFlags);

//Use 
GMI_RESULT GMI_ConfigToolWatchDogTest(uint16_t SessionId, uint32_t AuthValue);

//Use for DcIris Device test
GMI_RESULT GMI_ConfigToolOpenDcIris(uint16_t SessionId, uint32_t AuthValue);
GMI_RESULT GMI_ConfigToolCloseDcIris(uint16_t SessionId, uint32_t AuthValue);

//Use for IRCUT Device test
GMI_RESULT GMI_ConfigToolIrCultOpen(uint16_t SessionId, uint32_t AuthValue);
GMI_RESULT GMI_ConfigToolIrCultClose(uint16_t SessionId, uint32_t AuthValue);

GMI_RESULT  GMI_SetSystemNetworkMac(const char_t *NetMac);


#endif

