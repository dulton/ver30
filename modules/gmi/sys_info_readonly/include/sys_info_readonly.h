#ifndef __SYS_INFO_READONLY_H__
#define __SYS_INFO_READONLY_H__
#include "gmi_system_headers.h"
#include "ipc_fw_v3.x_setting.h"

//at first, call this api when use sys_info_readonly module
GMI_RESULT SysInfoReadInitialize();
//at last , call this api when finish sys_info_readonly module
GMI_RESULT SysInfoReadDeinitialize();
//open
GMI_RESULT SysInfoOpen(const char_t* FileName, FD_HANDLE *Handle);
//close
GMI_RESULT SysInfoClose(FD_HANDLE Handle);
//read
GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const char_t* Default , char_t *Context);
GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const float_t Default , float_t *Context);
GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord,const  int32_t Default , int32_t *Context);
GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const uint32_t Default, uint32_t *Context);
GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const uint16_t Default , uint16_t *Context);
GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const int16_t Default , int16_t *Context);
GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const int8_t Default , int8_t *Context);
GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const uint8_t Default , uint8_t *Context);




//read cpu name
GMI_RESULT SysInfoReadCpuName(char_t Cpu[32]);
//read sensor name
GMI_RESULT SysInfoReadSensorName(char_t Sensor[32]);
//read ircut name
GMI_RESULT SysInfoReadIRcutName(char_t IRcut[32]);
//read shield name
GMI_RESULT SysInfoReadShieldName(char_t Shield[32]);
//read max resolution
GMI_RESULT SysInfoReadMaxRes(int32_t *Width, int32_t *Height);
//read max stream num
GMI_RESULT SysInfoReadMaxStreamNum(int32_t *StreamNum);
//read software version
GMI_RESULT SysInfoReadSoftwareVer(char_t Ver[64]);
//read hardware version
GMI_RESULT SysInfoReadHardwareVer(char_t Ver[64]);
//read alarm state, true-enable, false-disable
GMI_RESULT SysInfoReadAlarmState(boolean_t *Enabled);
//read language
GMI_RESULT SysInfoReadLanguage(char_t Language[32]);
//read GB28181 state, true-enable, false-disable
GMI_RESULT SysInfoReadGB28181State(boolean_t *Enabled);

#endif
