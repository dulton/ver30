#include "gmi_brdwrapper.h"

#if !defined( __linux__ )

GMI_RESULT GMI_SetSysZone(char_t *Zone)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_GetBrdTime( struct tm* ptTime)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_SetBrdTime( struct tm* ptTime )
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_SysOpenWdGuard(ulong_t dwNoticeTimeout)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_SysCloseWdGuard(void)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_SysNoticeWdGuard(void)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_SysWdGuardIsOpened(int32_t *Status)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_BrdHwReset(void)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_BrdLedStatusSet(uint8_t byLedID, uint8_t byState)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_BrdQueryLedState(TBrdLedState *ptBrdLedState)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_BrdGetAnalogInputValue(uint8_t byPort, int32_t* volts)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_BrdGetAlarmInput(int32_t mode, uint8_t byPort, uint8_t* pbyState)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_BrdSetAlarmOutput(int32_t mode,uint8_t byPort, uint8_t byState)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_BrdGetEthLinkStat(uint8_t byEthId, uint8_t *Link)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_BrdGetEthNegStat(uint8_t byEthId, uint8_t *AutoNeg, uint8_t *Duplex, ulong_t *Speed)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_BrdSetEthNego(uint8_t byEthId, uint8_t Duplex, ulong_t Speed)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_PtzCtrInit(char_t *Filename, char_t *ItemPath, UartCtrlCfg *CtrlCfg,  FD_HANDLE  *Handle)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_PtzCtlUninit(FD_HANDLE  Handle)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_PtzControl(FD_HANDLE  Handle, int32_t CtlMod, PtzCtlCmd *CtlCmd)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_PtzCfgRead( char_t * FileName, const char_t *ItemPath, UartCtrlCfg *DefaultCtrlCfg, UartCtrlCfg *CtrlCfg)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_PtzCfgWrite( char_t * FileName, const char_t *ItemPath, UartCtrlCfg *CtrlCfg)
{
    return GMI_SUCCESS;
}

#endif
