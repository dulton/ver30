#ifndef __CONFIG_FILE_MANAGER_H
#define __CONFIG_FILE_MANAGER_H

#include "ipc_fw_v3.x_setting.h"
#include "ipc_fw_v3.x_resource.h"
#include "sys_env_types.h"
#include "gmi_media_ctrl.h"
#include "gmi_system_headers.h"


class ConfigFileManager
{
public:
    ConfigFileManager();
    ~ConfigFileManager();
    GMI_RESULT Initialize();
    GMI_RESULT Deinitialize();
    
    GMI_RESULT GetVideoEncodeStreamNum(int32_t *StreamNum);
    GMI_RESULT GetVideoEncodeSettings(int32_t StreamId, SysPkgEncStreamCombine *SysEncStreamCombine, VideoEncodeParam *VidEncParam);    
    GMI_RESULT GetVideoEncodeSettingsDefault(int32_t StreamId, SysPkgEncStreamCombine *SysEncStreamCombine, SysPkgEncodeCfg *SysEncodeCfgPtr);
    GMI_RESULT GetVideoEncodeSettingsDefault(int32_t StreamId, SysPkgEncStreamCombine *SysEncStreamCombine, VideoEncodeParam *VidEncParam);
    GMI_RESULT GetStreamCombine(SysPkgEncStreamCombine *SysEncStreamCombinePtr);
    GMI_RESULT SetStreamCombine(SysPkgEncStreamCombine *SysEncStreamCombinePtr);
    GMI_RESULT GetSysEncStreamCombineDefault(SysPkgEncStreamCombine *SysEncStreamCombinePtr);
    GMI_RESULT GetVideoEncodeSettingOptions();
    GMI_RESULT GetOsdTextNum(int32_t StreamId, int32_t *TextNum);
    GMI_RESULT GetOsdSettings(int32_t StreamId, int32_t StreamNum, VideoOSDParam *OsdParamPtr);
    GMI_RESULT SetOsdSettings(int32_t StreamId, VideoOSDParam *OsdParamPtr);
    GMI_RESULT SetVideoEncodeStreamNum(int32_t StreamNum);
    GMI_RESULT SetVideoEncodeSetting(int32_t StreamId, VideoEncodeParam * VidEncParam);
    GMI_RESULT GetImageSettings(int32_t SourceId, int32_t ChanId, ImageBaseParam *ImageBaseParamPtr);    
	GMI_RESULT GetImageSettingsDefault(int32_t SourceId, int32_t ChanId, SysPkgImaging *SysImagingPtr);
    GMI_RESULT SetImageSettings(int32_t SourceId, int32_t ChanId, ImageBaseParam *ImageBaseParamPtr);
    GMI_RESULT GetVideoSourceSettings(int32_t SourceId, SysPkgVideoSource *VideoSourcePtr);
    GMI_RESULT SetVideoSourceSettings(int32_t SourceId, SysPkgVideoSource *VideoSourcePtr);
    GMI_RESULT GetImageAdvanceSettings(int32_t SourceId, int32_t ChanId, ImageAdvanceParam *ImageAdavnceParamPtr);
    GMI_RESULT GetImageAdvanceSettingsDefault(int32_t SourceId, int32_t ChanId, SysPkgAdvancedImaging *SysAdvancedImagingPtr);
    GMI_RESULT SetImageAdvanceSettings(int32_t SourceId, int32_t ChanId, ImageAdvanceParam *ImageAdvanceParamPtr);
    GMI_RESULT GetWhiteBalanceSettings(int32_t SourceId, ImageWbParam *ImageWbParamPtr);
    GMI_RESULT SetWhiteBalanceSettings(int32_t SourceId, ImageWbParam *ImageWbParamPtr);
    GMI_RESULT SetDaynightSettings(int32_t SourceId, ImageDnParam *ImageDnParamPtr);
    GMI_RESULT GetDaynightSettings(int32_t SourceId, ImageDnParam *ImageDnParamPtr);
    GMI_RESULT GetDaynightSettingsDefault(int32_t SourceId, SysPkgDaynight *SysDaynight);
    GMI_RESULT GetVideoStreamType(int32_t StreamId, int32_t *StreamTypePtr);
    GMI_RESULT SetVideoStreamType(int32_t StreamId, int32_t StreamType);
    GMI_RESULT GetAudioEncodeSettings(AudioEncParam *SysAudioEncodeCfgPtr);
    GMI_RESULT GetAudioEncodeSettings(SysPkgAudioEncodeCfg *AudioEncodeCfgPtr);
    GMI_RESULT SetAudioEncodeSettings(SysPkgAudioEncodeCfg *AudioEncodeCfgPtr);
    
    GMI_RESULT GetHwAutoDetectInfo(SysPkgComponents *SysComponents);
    GMI_RESULT GetDeviceInfo(SysPkgDeviceInfo *SysDeviceInfoPtr);
    GMI_RESULT SetDeviceInfo(SysPkgDeviceInfo *SysDeviceInfoPtr);
    GMI_RESULT GetNtpServerInfo(SysPkgNtpServerInfo *SysNtpServerInfoPtr);
    GMI_RESULT SetNtpServerInfo(SysPkgNtpServerInfo *SysNtpServerInfoPtr);
    GMI_RESULT GetSysTimeType(SysPkgDateTimeType *SysDateTimePtr);
    GMI_RESULT SetSysTimeType(SysPkgDateTimeType *SysDateTimePtr);
    GMI_RESULT SetExternNetworkPort(SysPkgNetworkPort *SysNetworkPortPtr);
    GMI_RESULT GetExternNetworkPort(SysPkgNetworkPort *SysNetworkPortPt);
    
    GMI_RESULT GetAutoFocusMode(int32_t *FocusModePtr);
    GMI_RESULT SetAutoFocusMode(int32_t FocusMode);   
    GMI_RESULT GetPresetsInfo(SysPkgPresetInfo_Inner SysPresetsInfoInner[256]);
    GMI_RESULT SetPresetsInfo(SysPkgPresetInfo_Inner *SysPresetInfoInnerPtr);
    GMI_RESULT GetCurrentZoomPos(int32_t *ZoomPosPtr);
    GMI_RESULT SetCurrentZoomPos(int32_t ZoomPos);    
    GMI_RESULT GetPtzSpeedMap(char_t HSpeed[10][64], char_t VSpeed[10][64]);

    GMI_RESULT FactoryDefault(void);
    GMI_RESULT GetBitrates(int32_t Width, int32_t Height, int32_t *BitRateAverage, int32_t *BitRateUp, int32_t *BitRateDown);
private:	
	GMI_RESULT Lock();
	GMI_RESULT Unlock();
	
private:
    char_t m_FactorySettingFile[128];
    char_t m_DefaultSettingFile[128];
    char_t m_SettingFile[128];   
    char_t m_ResourceFile[128];
    char_t m_PresetsFile[128];
    GMI_Mutex m_Mutex;
};

#endif

