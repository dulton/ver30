#ifndef __GMI_PTZ_CONTROL_API_H__
#define __GMI_PTZ_CONTROL_API_H__

#include "gmi_system_headers.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

#define PTZ_DEVICE_PATH         "/dev/ttyS1"

//Ptz Control Mode
#define  PTZ_CONTINUE_CONTROL_MODE     0
#define  PTZ_DISCONTINUE_CONTROL_MODE     1

//Ptz Control  Protocol
#define  PTZ_PROTOCOL_D_PROTOCOL        1
#define  PTZ_PROTOCOL_P_PROTOCOL         2
#define  PTZ_PROTOCOL_AIP_STAR_PTZ      3


// ctr cmd:
// 0-up, 1-down, 2-up stop, 3-down stop, 
// 4-left, 5-right, 6-left stop, 7-right stop
// 8-ZoomTele, 9-ZoomWide, 10-ZoomTele stop, 11-ZoomWide stop
// 12-FocusNear, 13-FocusFar, 14-FocusNear stop, 15-FocusFar stop
// 16-IrisOpen, 17-IrisClose, 18-IrisOpen stop, 19-IrisClose stop
// 20-Stop all
typedef enum{
    e_PTZ_CMD_Up=0,
    e_PTZ_CMD_Down,
    e_PTZ_CMD_UpStop,
    e_PTZ_CMD_DownStop,
    e_PTZ_CMD_Left,
    e_PTZ_CMD_Right,
    e_PTZ_CMD_LeftStop,
    e_PTZ_CMD_RightStop,
    e_PTZ_CMD_ZoomTele,
    e_PTZ_CMD_ZoomWide,
    e_PTZ_CMD_ZoomTeleStop,	// 10
    e_PTZ_CMD_ZoomWideStop,
    e_PTZ_CMD_FocusNear,
    e_PTZ_CMD_FocusFar,
    e_PTZ_CMD_FocusNearStop,
    e_PTZ_CMD_FocusFarStop,
    e_PTZ_CMD_IrisOpen,
    e_PTZ_CMD_IrisClose,
    e_PTZ_CMD_IrisOpenStop,
    e_PTZ_CMD_IrisCloseStop,
    e_PTZ_CMD_SetSpeed,	// 20
    
    e_PTZ_CMD_SetPreset,
    e_PTZ_CMD_RmPreset,
    e_PTZ_CMD_GotoPreset,
    
    e_PTZ_CMD_Stop,
    
    e_PTZ_CMD_LeftUp, //25
    e_PTZ_CMD_LeftDown,
    e_PTZ_CMD_RightUp,
    e_PTZ_CMD_RightDown,
    
    e_PTZ_CMD_LeftLimit, // 29 this is compare with define in UART_EXT_CMD_LEFT_LIMIT
    e_PTZ_CMD_RightLimit,
    e_PTZ_CMD_UpLimit,
    e_PTZ_CMD_DownLimit,
    e_PTZ_CMD_ScanStart,
    e_PTZ_CMD_ScanStop,
    e_PTZ_CMD_AuxpntOpen,
    e_PTZ_CMD_AuxpntClose, // 36 this is compare with UART_EXT_CMD_AUXPNT_CLOSE
    e_PTZ_CMD_FocusNearClick,
    e_PTZ_CMD_FocusFarClick,
    e_PTZ_CMD_H_360,
    
    e_PTZ_CMD_Invalid = 999,
}PtzCmd;


//Device UART Cfg
typedef struct _UartCtrlCfg
{
    uint32_t s_Bauderate;     //bauderate
    uint32_t s_Databits;        //date bit
    uint32_t s_Parity;           //parity
    uint32_t s_Stopbits;       //stop bit
    uint32_t s_FlowControl;   //flow Control
    uint32_t s_Protocol;         //ptz Protocal
    uint32_t s_Address;        //Ptz Address
} UartCtrlCfg;

//Ptz Control Speed 
typedef struct _PtzCtlCmd
{
    int32_t s_Cmd;
    int32_t s_CmdParam[8];  //Max Ctl Speed 100, MIn Ctl Speed 0
    char_t  s_PresetName[128]; 
}PtzCtlCmd;

//Auto focus start
GMI_RESULT GMI_AutoFocusStart(void);

//Auto focus stop
GMI_RESULT GMI_AutoFocusStop(void);

//auto and manual charage
GMI_RESULT GMI_AutoFocusPause(boolean_t Pause);

//PTZ config read
GMI_RESULT GMI_PtzCfgRead(char_t * FileName, const char_t *ItemPath, UartCtrlCfg *DefaultCtrlCfg, UartCtrlCfg *CtrlCfg);

//PTZ config write
GMI_RESULT GMI_PtzCfgWrite(char_t * FileName, const char_t *ItemPath, UartCtrlCfg *CtrlCfg);

//Ptz control
GMI_RESULT GMI_PtzControl(FD_HANDLE  Handle, int32_t CtlMod, PtzCtlCmd *CtlCmd);

//Ptz control init
GMI_RESULT GMI_PtzCtrInit(char_t *Filename, char_t *ItemPath, UartCtrlCfg *CtrlCfg, FD_HANDLE  *Handle);

//Ptz Control uninit
GMI_RESULT GMI_PtzCtlUninit(FD_HANDLE  Handle);


//#ifdef __cplusplus
//}
//#endif

#endif


