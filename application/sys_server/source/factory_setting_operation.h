#ifndef __FACTORY_SETTING_OPERATION_H__
#define __FACTORY_SETTING_OPERATION_H__

#include "sys_env_types.h"
#include "ipc_fw_v3.x_setting.h"
#include "gmi_system_headers.h"

class FactorySettingOperation
{
public:
    FactorySettingOperation();
    ~FactorySettingOperation();
    GMI_RESULT Initialize();
    GMI_RESULT Deinitialize();      
    GMI_RESULT GetVideoMaxCapability(int32_t *MaxStreamNum, int32_t *MaxPicWidth, int32_t *MaxPicHeight);
    GMI_RESULT GetVideoParams(int32_t StreamNum, int32_t CombineNo, int32_t Res[16][4]);//Res[][0]:width,Res[][1]: height,Res[][2]: frames
    GMI_RESULT GetImageParams(int32_t Imgs[32]);//Imgs[0]:bright,Imgs[1]:contrast,Imgs[2]:saturation,Imgs[3]:hue,Imgs[4]:sharpness, Imgs[5]:ae target ratio
    GMI_RESULT GetPtzSpeed(char_t HSpeed[10][64], char_t VSpeed[10][64]);
    GMI_RESULT GetIrCutModeDnThr(int8_t *IrcutMode, int8_t *DayToNightThr, int8_t *NightToDayThr);
    GMI_RESULT GetDeviceInfo(SysPkgDeviceInfo *SysDeviceInfoPtr);
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
	GMI_RESULT Lock();
	GMI_RESULT Unlock();
private:
    GMI_Mutex  m_Mutex;
};


#endif

