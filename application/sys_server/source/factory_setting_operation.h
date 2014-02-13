#ifndef __FACTORY_SETTING_OPERATION_H__
#define __FACTORY_SETTING_OPERATION_H__

#include "sys_env_types.h"
#include "ipc_fw_v3.x_setting.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_system_headers.h"
#include "file_lock.h"
#include "gmi_media_ctrl.h"


class FactorySettingOperation
{
public:
    FactorySettingOperation();
    ~FactorySettingOperation();
    GMI_RESULT Initialize();
    GMI_RESULT Deinitialize();   
    //get max stream num, max encode width, max encode height
    GMI_RESULT GetVideoMaxCapability(int32_t *MaxStreamNum, int32_t *MaxPicWidth, int32_t *MaxPicHeight);
    //get resolution and frame rate accroding to stream cominbe no and stream num currently
    GMI_RESULT GetVideoParams(int32_t StreamNum, int32_t CombineNo, int32_t Res[16][4]);//Res[][0]:width,Res[][1]: height,Res[][2]: frames
    //get image params
    GMI_RESULT GetImageParams(int32_t Imgs[32]);//Imgs[0]:bright,Imgs[1]:contrast,Imgs[2]:saturation,Imgs[3]:hue,Imgs[4]:sharpness, Imgs[5]:ae target ratio
    //get ptz speed map table
    GMI_RESULT GetPtzSpeed(char_t HSpeed[10][64], char_t VSpeed[10][64]);
    //get ircut mode, daytonight threshold, nighttoday threshold
    GMI_RESULT GetIrCutModeDnThr(int8_t *IrcutMode, int8_t *DayToNightThr, int8_t *NightToDayThr);
    //get ircut
    GMI_RESULT GetIrcut(GeneralParam_Ircut *Ircut);
    //get device information
    GMI_RESULT GetDeviceInfo(SysPkgDeviceInfo *SysDeviceInfoPtr);
    //excute import file
    GMI_RESULT ExcuteImportFile(SysPkgConfigFileInfo *SysConfigFilePtr);
private:
	GMI_RESULT FileIsExist(const char_t *File, boolean_t *Exist);
	GMI_RESULT GetHwAutoDetectInfo(SysPkgComponents *SysComponents);
	GMI_RESULT CheckVideoReslution(uint16_t Width, uint16_t Height);
	GMI_RESULT GetMediaParam(int32_t *MaxStreamNumPtr, int32_t *MaxPicWidthPtr, int32_t *MaxPicHeightPtr);
	GMI_RESULT WriteMediaParamToCapSw(int32_t MaxStreamNum, int32_t MaxPicWidth, int32_t MaxPicHeight);
	GMI_RESULT GetStreamCombine(int32_t MaxPicHeight, char_t **Combine,  int32_t *CombineLen);
	GMI_RESULT WriteStreamCombineToCapSw(char_t *Combine,  int32_t CombineLen);
	GMI_RESULT GenerateSoftwareCapability(); 
	GMI_RESULT GetSensorName(char_t SensorName[64]);
	GMI_RESULT GetMaxResByHwInfo(int32_t *MaxWidthPtr, int32_t *MaxHeightPtr);
	GMI_RESULT CheckCapabilitySwFile(const char_t *FilePath);
	GMI_RESULT GenerateSoftwareCapabilityByHwInfo(void);
	GMI_RESULT Lock();
	GMI_RESULT Unlock();
private:
    GMI_Mutex  m_Mutex;
    CSemaphoreMutex m_FactorySettingFileLock;    
    CSemaphoreMutex m_CapabilitySwFileLock;
    CSemaphoreMutex m_CapabilityAutoFileLock;
};


#endif

