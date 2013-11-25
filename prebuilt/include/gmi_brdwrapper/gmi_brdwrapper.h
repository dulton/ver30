#ifndef _GMI_BRD_WRAPPER_H_
#define _GMI_BRD_WRAPPER_H_

#include "gmi_system_headers.h"
#include "gmi_brdwrapperdef.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    uint8_t s_byLedSysRun;
    uint8_t s_byLedAppRun;
    uint8_t s_byLedHD;   //Soft  cannot detect  ,return LED  off
}
#if defined( __linux__ )
__attribute__ ((packed))
#endif
TBrdGMIPC2000LedDesc;

//LED information struct
typedef struct
{
    uint8_t s_dwLedNum;
    union
    {
        TBrdGMIPC2000LedDesc s_tBrdGMIPC2000Led;
        ulong_t s_byLedNo[BRD_LED_NUM_MAX];
    } nlunion;
} TBrdLedState;

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

//#######################     RTC API  START  #####################################
/*==============================================================================
name				:	GMI_SetSysZone
function			:  Setting system zone info , nwo Support zone Shanghai and UTC.
algorithm implementation	:	no
global variable		:	no
parameter declaration	:     Zone : string .example ,"UTC"  or "Shanghai"

return				:    GMI_SUCCESS:success
                              GMI_FAIL: fail
******************************************************************************/
GMI_RESULT GMI_SetSysZone(char_t *Zone);

/*==============================================================================
name				:	GMI_GetBrdTime
function			:  Get brd hardware Rtc 
algorithm implementation	:	no
global variable		:	no
parameter declaration	:     ptTime : struct tm. 

return				:    GMI_SUCCESS:success
                              GMI_FAIL: fail
******************************************************************************/
GMI_RESULT GMI_GetBrdTime( struct tm* ptTime);

/*==============================================================================
name				:  GMI_SetBrdTime
function			:  Setting board Rtc time
algorithm implementation	:	no
global variable		:	no
parameter declaration	:     ptTime : struct tm. 

return				:    GMI_SUCCESS:success
                              GMI_FAIL: fail
******************************************************************************/
GMI_RESULT GMI_SetBrdTime( struct tm* ptTime );


//#######################     WARTCH  API  START  #####################################

/*==============================================================
name				:	GMI_SysOpenWdGuard
function			:	Open watchdog Guard function
algorithm implementation	:	no
global variable			:	no
parameter declaration		:    dwNoticeTimeout :   Notify system of guard time.if Application time out feed watchdog,
                                         System will be reboot. if set NoticeTimeout 0, System will be feed Watchdog itself.
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
******************************************************************************/
GMI_RESULT GMI_SysOpenWdGuard(ulong_t dwNoticeTimeout);

/*==============================================================
name				:	GMI_SysCloseWdGuard
function			:	Close watchdog Guard function
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
******************************************************************************/
GMI_RESULT GMI_SysCloseWdGuard(void);

/*==============================================================
name				:	GMI_SysNoticeWdGuard
function			:	Application Notice System Watchdog Guard Running. if Open system guard ,Application
                            must be running this interface ,or so system will be reset
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	6/3/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
GMI_RESULT GMI_SysNoticeWdGuard(void);

/*==============================================================
name				:	GMI_SysWdGuardIsOpened
function			:  Get System WdGuad state
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     Status : 0 ,System wachdog is not open
                                             1 ,System wachdog is open
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
******************************************************************************/
GMI_RESULT GMI_SysWdGuardIsOpened(int32_t *Status);

/*==============================================================
name				:	GMI_BrdHwReset
function			:  HardWare reboot System
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
******************************************************************************/
GMI_RESULT  GMI_BrdHwReset(void);

/*==============================================================
name				:	GMI_SysHwWatchDogDisable
function			:  HardWare Hw Watchdog Disable
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
******************************************************************************/
GMI_RESULT GMI_SysHwWatchDogDisable(void);

/*==============================================================
name				:	GMI_SysHwWatchDogEnable
function			:  HardWare Hw Watchdog Enable
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
******************************************************************************/
GMI_RESULT GMI_SysHwWatchDogEnable(void);


//#######################     LED CONTROL  API  START  #####################################

/*==============================================================
name				:	GMI_BrdLedStatusSet
function			:  Set LED state
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     byLedID : the LED ID ,see (brdwrapperdef.h macro define)
                                              byState : LED state.
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
******************************************************************************/
GMI_RESULT GMI_BrdLedStatusSet(uint8_t byLedID, uint8_t byState);

/*==============================================================
name				:	BrdQueryLedState
function			:  Query LED state
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     ptBrdLedState : struct storage led state struct
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
******************************************************************************/
GMI_RESULT GMI_BrdQueryLedState(TBrdLedState *ptBrdLedState);


//#######################     ADC   API  START  #####################################

/*==============================================================
name				:	GMI_BrdGetAnalogInputValue
function			:  Get Analog input Value
lgorithm implementation	:	no
global variable			:	no
parameter declaration		:     byPort : Analog Input port
                                               volts: Analog Input volts
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT GMI_BrdGetAnalogInputValue(uint8_t byPort, int32_t* volts);


//#######################     ALARM   API  START  #####################################

/*==============================================================
name				:	GMI_BrdGetAlarmInput
function			:	Get Alarm Input State
algorithm implementation	:	no
global variable			:	no
parameter declaration		:    byPort :  Alarm in Port
                                             pbyState: 0 low volte. 1 hight volte
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT GMI_BrdGetAlarmInput(int32_t mode, uint8_t byPort, uint8_t* pbyState);

/*==============================================================
name				:	GMI_BrdSetAlarmOutput
function			:	Set Alarm Output State
algorithm implementation	:	no
global variable			:	no
parameter declaration		:    byPort :  Alarm Output Port
                                             byState: Alarm OutPut state
return				:    FAIL:  GMI_FAIL
                              SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT GMI_BrdSetAlarmOutput(int32_t mode,uint8_t byPort, uint8_t byState);


//#######################     NetWork   API  START  #####################################


/*==============================================================
name				:	GMI_BrdGetEthLinkStat
function			:  Get Eth interface Link state
lgorithm implementation	:	no
global variable			:	no
parameter declaration		:     byEthId  ,Ethernet port
                                               Link  ,Pointer to Ethernet Port state. 0 Link down  1 Link up
return				:    FAIL:  GMI_FAIL
                                     SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT  GMI_BrdGetEthLinkStat(uint8_t byEthId, uint8_t *Link);

/*==============================================================
name				:	GMI_BrdGetEthNegStat
function			:  Get Eth interface Link state
lgorithm implementation	:	no
global variable			:	no
parameter declaration		:     byEthId  ,Ethernet port
                                               AutoNeg . Ethernet state , 1 auto mode  0 force mode
                                               Duplex.  1  full duplex   0  half duplex
                                               Speed  , Ethernet rate  10/100/1000
return				:    FAIL:  GMI_FAIL
                                     SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT  GMI_BrdGetEthNegStat(uint8_t byEthId, uint8_t *AutoNeg, uint8_t *Duplex, ulong_t *Speed);

/*==============================================================
name				:	GMI_BrdSetEthNego
function			:  Set Eth interface Link state
lgorithm implementation	:	no
global variable			:	no
parameter declaration		:     byEthId  ,Ethernet port
                                               AutoNeg . Ethernet state , 1 auto neg  0 force mode
                                               Duplex.  1  full duplex   0  half duplex
                                               Speed  , Ethernet rate  10/100/1000
return				:    FAIL:  GMI_FAIL
                                     SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT  GMI_BrdSetEthNego(uint8_t byEthId, uint8_t Duplex, ulong_t Speed);


//#######################     PTZ CONTROL   API  START  #####################################

/*==============================================================
name				:	GMI_PtzCtrInit
function			:  Ptz Control init .
lgorithm implementation	:	no
global variable			:	no
parameter declaration		:     Filename  ,open file name
                                    ItemPath  . Xml config path 
                                    CtrlCfg   .  Uart config struct
                                    FD_HANDLE .
return				:    FAIL:  GMI_FAIL
                                     SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT GMI_PtzCtrInit(char_t *Filename, char_t *ItemPath, UartCtrlCfg *CtrlCfg,  FD_HANDLE  *Handle);

/*==============================================================
name				:	GMI_PtzCtlUninit
function			:  Ptz control uninit, free Handle resource
lgorithm implementation	:	no
global variable			:	no
parameter declaration		:     Handle  , 
return				:    FAIL:  GMI_FAIL
                                     SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT GMI_PtzCtlUninit(FD_HANDLE  Handle);

/*==============================================================
name				:	GMI_PtzControl
function			:  Ptz Control api
lgorithm implementation	:	no
global variable			:	no
parameter declaration		:     Handle . Ptz control Handle
                                    CtlMod . Ptz control mode for example ,
                                                    PTZ_CONTINUE_CONTROL_MODE
                                                    PTZ_DISCONTINUE_CONTROL_MODE  
                                   CtlCmd . Ptz control Cmd struct
                                          
return				:    FAIL:  GMI_FAIL
                                     SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT GMI_PtzControl(FD_HANDLE  Handle, int32_t CtlMod, PtzCtlCmd *CtlCmd);

/*==============================================================
name				:	GMI_PtzCfgRead
function			:  Read Ptz config file ,if file not exist, create config file
lgorithm implementation	:	no
global variable			:	no
parameter declaration		:      Filename  ,open file name
                                    ItemPath  . Xml config path 
							DefaultCtrlCfg. Uart default config struct
                                    CtrlCfg   .  Uart config struct
                   
return				:    FAIL:  GMI_FAIL
                                     SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT GMI_PtzCfgRead( char_t * FileName, const char_t *ItemPath, UartCtrlCfg *DefaultCtrlCfg, UartCtrlCfg *CtrlCfg);

/*==============================================================
name				:	GMI_PtzCfgWrite
function			:  Write Ptz config file
lgorithm implementation	:	no
global variable			:	no
parameter declaration		:      Filename  ,open file name
                                    ItemPath  . Xml config path 
                                    CtrlCfg   .  Uart config struct
                                    
return				:    FAIL:  GMI_FAIL
                                     SUCCESS : GMI_SUCCESS
================================================================*/
GMI_RESULT GMI_PtzCfgWrite( char_t * FileName, const char_t *ItemPath, UartCtrlCfg *CtrlCfg);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_BRDWRAPPER_H_*/


