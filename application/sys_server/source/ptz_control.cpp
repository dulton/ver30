#include "ipc_fw_v3.x_setting.h"
#include "ptz_control.h"
#include "gmi_system_headers.h"
#include "base_memory_manager.h"
#include "referrence_ptr.h"
#include "log.h"
#include "config_file_manager.h"


PtzControl::PtzControl()
    : m_Handle( NULL )
    , m_CfgFilePtr()
    , m_CfgPathPtr()
    , m_PresetPathPtr()
    , m_DefaultUartCtrlCfg()
{
}


PtzControl::~PtzControl()
{
}


GMI_RESULT PtzControl::GetSpeed(char_t *SpeedString, int32_t Speed[11])
{
    char_t *Name = SpeedString;
    char_t *NameTmp = NULL;
    uint8_t Nel = 0;
    int32_t Len = strlen(Name);
    NameTmp = Name;
    while (strsep(&NameTmp, ","))
    {
        if (NameTmp)
        {
            Nel++;
        }
    }

    if (1 == Nel)
    {
        return GMI_FAIL;
    }

    uint32_t i = 0;
    char_t *SpeedTmp = NULL;
    for (NameTmp = Name; NameTmp < (Name+Len);)
    {
        SpeedTmp = NameTmp;
        for (NameTmp += strlen(NameTmp); NameTmp < (Name+Len) && !*NameTmp; NameTmp++);
        Speed[i] = atoi(SpeedTmp);
        i++;
    }

    return GMI_SUCCESS;
}


GMI_RESULT PtzControl::LoadPtzSpeedTable()
{
    char_t HS[MAX_SPEED_RANGE / STEP][64];
    char_t VS[MAX_SPEED_RANGE / STEP][64];
    memset(HS, 0, sizeof(HS));
    memset(VS, 0, sizeof(VS));

    GMI_RESULT Result = m_ConfigFileManagerPtr->GetPtzSpeedMap(HS, VS);
    if (FAILED(Result))
    {
        SYS_ERROR("get ptz speed map table fail, Result = 0x%lx\n", Result);
        return Result;
    }


    SYS_INFO("===PTZ SPEED TABLE===\n");
    for (int32_t i = 0; i < MAX_SPEED_RANGE / STEP; i++)
    {
        SYS_INFO("HS[%d] = %s VS[%d] = %s\n", i, HS[i], i, VS[i]);
        Result = GetSpeed(HS[i], (m_HSpeed + i * STEP));
        if (FAILED(Result))
        {
            //fail, set default value
            memset(m_HSpeed, MAX_SPEED_RANGE/2, sizeof(m_HSpeed));
            break;
        }

        Result = GetSpeed(VS[i], (m_VSpeed + i * STEP));
        if (FAILED(Result))
        {
            //fail, set default value
            memset(m_VSpeed, MAX_SPEED_RANGE/2, sizeof(m_VSpeed));
            break;
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT PtzControl::Initialize( const char_t * CfgFileName, const char_t * CfgPathName, const char_t * PresetPathName, UartCtrlCfg * DefCtrlCfg )
{
    if (CfgFileName == NULL
            || CfgPathName == NULL
            || PresetPathName == NULL
            || DefCtrlCfg == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    m_CfgFilePtr = BaseMemoryManager::Instance().News<char_t>( strlen(CfgFileName) );
    if (m_CfgFilePtr.GetPtr() == NULL)
    {
        return GMI_OUT_OF_MEMORY;
    }

    m_CfgPathPtr = BaseMemoryManager::Instance().News<char_t>( strlen(CfgPathName) );
    if (m_CfgFilePtr.GetPtr() == NULL)
    {
        m_CfgFilePtr = NULL;
        return GMI_OUT_OF_MEMORY;
    }

    m_PresetPathPtr = BaseMemoryManager::Instance().News<char_t>( strlen( PresetPathName ));
    if (m_PresetPathPtr.GetPtr() == NULL)
    {
        m_CfgFilePtr = NULL;
        m_CfgPathPtr = NULL;
        return GMI_OUT_OF_MEMORY;
    }

    memcpy(m_CfgFilePtr.GetPtr(), CfgFileName, strlen(CfgFileName));
    memcpy(m_CfgPathPtr.GetPtr(), CfgPathName, strlen(CfgPathName));
    memcpy(m_PresetPathPtr.GetPtr(), PresetPathName, strlen(PresetPathName));
    memcpy(&m_DefaultUartCtrlCfg, DefCtrlCfg, sizeof(UartCtrlCfg));

    GMI_RESULT Result = GMI_SUCCESS;
    UartCtrlCfg CtrlCfgTmp;

    Result = GMI_PtzCfgRead((char_t*)CfgFileName, CfgPathName, DefCtrlCfg, &CtrlCfgTmp);
    if (FAILED(Result))
    {
        return Result;
    }

    Result = GMI_PtzCtrInit((char_t*)CfgFileName, (char_t*)PresetPathName, DefCtrlCfg, &m_Handle);
    if (FAILED(Result))
    {
        return Result;
    }

    m_ConfigFileManagerPtr = BaseMemoryManager::Instance().New<ConfigFileManager>();
    if (NULL == m_ConfigFileManagerPtr.GetPtr())
    {
        GMI_PtzCtlUninit(m_Handle);
        SYS_ERROR("m_ConfigFileManagerPtr new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_ConfigFileManagerPtr new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    Result = m_ConfigFileManagerPtr->Initialize();
    if (FAILED(Result))
    {
        GMI_PtzCtlUninit(m_Handle);
        SYS_ERROR("m_ConfigFileManagerPtr->Initialize fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = LoadPtzSpeedTable();
    if (FAILED(Result))
    {
        GMI_PtzCtlUninit(m_Handle);
        SYS_ERROR("LoadPtzSpeedTable fail, Result = 0x%lx\n", Result);
        return Result;
    }

    //stop all
    PtzCtlCmd  PtzCtlCmdTmp;
    memset(&PtzCtlCmdTmp, 0, sizeof(PtzCtlCmd));
    PtzCtlCmdTmp.s_Cmd = e_PTZ_CMD_Stop;
    Control(PTZ_CONTINUE_CONTROL_MODE, &PtzCtlCmdTmp);

    return GMI_SUCCESS;
}


GMI_RESULT PtzControl::Deinitialize()
{
    GMI_RESULT Result = m_ConfigFileManagerPtr->Deinitialize();
    if (FAILED(Result))
    {
        return Result;
    }

    Result = GMI_PtzCtlUninit(m_Handle);
    if (FAILED(Result))
    {
        return Result;
    }

    m_CfgFilePtr = NULL;
    m_CfgPathPtr = NULL;
    m_PresetPathPtr = NULL;
    return GMI_SUCCESS;
}

GMI_RESULT PtzControl::Control(int32_t CtrlMode, PtzCtlCmd *PtzCtlCmdPtr)
{
    GMI_RESULT Result = GMI_SUCCESS;

    if (PtzCtlCmdPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    switch (PtzCtlCmdPtr->s_Cmd)
    {
    case e_PTZ_CMD_Left:
    case e_PTZ_CMD_Right:
        PtzCtlCmdPtr->s_CmdParam[0] = m_HSpeed[PtzCtlCmdPtr->s_CmdParam[0]];
        break;
    case e_PTZ_CMD_Up:
    case e_PTZ_CMD_Down:
        PtzCtlCmdPtr->s_CmdParam[0] = m_VSpeed[PtzCtlCmdPtr->s_CmdParam[0]];
        break;
    case e_PTZ_CMD_LeftUp:
    case e_PTZ_CMD_LeftDown:
    case e_PTZ_CMD_RightUp:
    case e_PTZ_CMD_RightDown:
        PtzCtlCmdPtr->s_CmdParam[0] = m_HSpeed[PtzCtlCmdPtr->s_CmdParam[0]];
        PtzCtlCmdPtr->s_CmdParam[1] = m_VSpeed[PtzCtlCmdPtr->s_CmdParam[1]];
        break;
    }

    Result = GMI_PtzControl(m_Handle, CtrlMode, PtzCtlCmdPtr);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT PtzControl::ReadCfg(UartCtrlCfg * CtrlCfgPtr)
{
    GMI_RESULT Result = GMI_SUCCESS;

    if (CtrlCfgPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    Result = GMI_PtzCfgRead(m_CfgFilePtr.GetPtr(), m_CfgPathPtr.GetPtr(), &m_DefaultUartCtrlCfg, CtrlCfgPtr);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT PtzControl::WriteCfg(UartCtrlCfg* CtrlCfgPtr)
{
    GMI_RESULT Result = GMI_SUCCESS;

    if (CtrlCfgPtr == NULL)
    {
        return GMI_INVALID_PARAMETER;
    }

    Result = GMI_PtzCfgWrite(m_CfgFilePtr.GetPtr(), m_CfgPathPtr.GetPtr(), CtrlCfgPtr);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}

