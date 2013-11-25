#ifndef __PTZ_CONTROL_H__
#define __PTZ_CONTROL_H__

#include "gmi_brdwrapperdef.h"
#include "gmi_brdwrapper.h"
#include "base_memory_manager.h"
#include "referrence_ptr.h"
#include "config_file_manager.h"

#define MAX_SPEED_RANGE  (100)
#define STEP (10)

class PtzControl
{
public:
    PtzControl();
    ~PtzControl();
    GMI_RESULT Initialize( const char_t* CfgFileName, const char_t* CfgPathName, const char_t* PresetPathName, UartCtrlCfg* DefCtrlCfg );
    GMI_RESULT Deinitialize();
    GMI_RESULT Control( int32_t CtrlMode, PtzCtlCmd *PtzCtlCmdPtr );
    GMI_RESULT ReadCfg( UartCtrlCfg* CtrlCfgPtr );
    GMI_RESULT WriteCfg( UartCtrlCfg* CtrlCfgPtr );
private:
    GMI_RESULT LoadPtzSpeedTable();
    GMI_RESULT GetSpeed(char_t *SpeedString, int32_t Speed[11]);
private:
    FD_HANDLE m_Handle;
    ReferrencePtr<char_t,  DefaultObjectsDeleter>  m_CfgFilePtr;
    ReferrencePtr<char_t,  DefaultObjectsDeleter>  m_CfgPathPtr;
    ReferrencePtr<char_t,  DefaultObjectsDeleter>  m_PresetPathPtr;
    UartCtrlCfg m_DefaultUartCtrlCfg;
    int32_t m_HSpeed[MAX_SPEED_RANGE]; //speed range 0~100; 0-9,10-19,...90-100
    int32_t m_VSpeed[MAX_SPEED_RANGE]; //range 0~100;
    ReferrencePtr<ConfigFileManager>   m_ConfigFileManagerPtr;
};

#endif

