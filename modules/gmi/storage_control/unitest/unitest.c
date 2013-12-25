#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <dirent.h>

#include "storage_manager.h"

int main(int argc, char *argv[])
{
	int iRet = -1;
	StorageFormatIn StorageFormatParam;
	memset(&StorageFormatParam, 0, sizeof(StorageFormatParam));
	StorageFormatParam.s_StorageDevice = TYPE_STORAGE_SD;
	StorageFormatParam.s_RecFileSize = 512;

	if(argc < 2)
	{
		printf("*************help*************\n");
		printf("./sdTest 2 0 0 0     query segment record of main db file \n");
		printf("./sdTest 2 0 1 0     query file record of main db file \n");
		printf("./sdTest 2 0 0 1     query segment record of backup db file \n");
		printf("./sdTest 2 0 1 1     query file record of backup db file \n");
		printf("\n");
		return LOCAL_RET_OK;
	}

	printf("main.\n");
	if(strcmp(argv[1], "0") == 0)
	{
		iRet = InitPartion(StorageFormatParam.s_RecFileSize);
	}
	else if(strcmp(argv[1], "4") == 0)
	{	
		iRet = GMI_StorageDeviceFormat(&StorageFormatParam);
	}
	
	if (iRet == LOCAL_RET_ERR)
	{
		printf("operate error\n");
		return LOCAL_RET_ERR;
	}

	return LOCAL_RET_OK;
}


GMI_RESULT SystemServiceManager::SvrPtzControl(SysPkgPtzCtrl *PtzCtrl )
{
    SYS_INFO("%s in..........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s in..........\n", 
__func__);
    uint32_t Time1 = TimeStamp();

	#ifdef TEST_STORAGE
	RecordScheduleConfigIn RecordScheduleConfig;
	RecordFileQueryIn RecordFileQueryPtr;
	uint32_t CurQueryPosNo = 0;
	uint32_t QueryResArraySize = MAX_NUM_QUERY_RECORD;
	RecordFileQueryResOut **RecordFileQueryResPtr = NULL;
	uint32_t QueryResTotalNum = 0;
	uint32_t  QueryResCurNum = 0;
	int32_t i_k = 0;
	static RecordDownReplayQueryIn RecordDownReplayQuery;
    RecordDownReplayQueryResOut **RecordDownReplayQueryResPtr;
	uint32_t QueryResArraySize2 = 1;
	#endif

    if (m_SupportPtz)
    {
        GMI_RESULT     Result;
        static int32_t LastPtzCmd;
        SysPkgPtzCtrl  PtzCtrlTmp;
        PtzCtlCmd      PT_CtlCmd;

        if (PtzCtrl == NULL)
        {
            return GMI_INVALID_PARAMETER;
        }

        memcpy(&PtzCtrlTmp, PtzCtrl, sizeof(SysPkgPtzCtrl));
        memset(&PT_CtlCmd, 0, sizeof(PtzCtlCmd));

        // SYS_INFO("Time %u, Cmd %d, Param[0] %d, Param[1] %d, Param[2] %d, 
Param[3] %d\n",
        //          Time1, PtzCtrlTmp.s_PtzCmd, PtzCtrlTmp.s_Param[0], 
PtzCtrlTmp.s_Param[1], PtzCtrlTmp.s_Param[2], PtzCtrlTmp.s_Param[3]);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Time %u, Cmd %d, 
Param[0] %d, Param[1] %d, Param[2] %d, Param[3] %d\n", \
                  Time1, PtzCtrlTmp.s_PtzCmd, PtzCtrlTmp.s_Param[0], 
PtzCtrlTmp.s_Param[1], PtzCtrlTmp.s_Param[2], PtzCtrlTmp.s_Param[3]);

        if (SYS_PTZCMD_LEFT        == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_RIGHT     == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_UP        == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_DOWN      == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_LEFTUP    == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_LEFTDOWN  == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_RIGHTUP   == PtzCtrlTmp.s_PtzCmd
                || SYS_PTZCMD_RIGHTDOWN == PtzCtrlTmp.s_PtzCmd)
        {
            switch (PtzCtrlTmp.s_PtzCmd)
            {
            case SYS_PTZCMD_LEFT:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_Left;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
				
				#ifdef TEST_STORAGE
				
				RecordFileQueryPtr.s_RecTrigMode = 0x1;
				RecordFileQueryPtr.s_RecQueryType = 0;
				RecordFileQueryPtr.s_RecQueryTime[0] = 1386421269;
				RecordFileQueryPtr.s_RecQueryTime[1] = 1388451169;
				RecordFileQueryResPtr = (RecordFileQueryResOut **)malloc(sizeof(
RecordFileQueryResOut *)*MAX_NUM_QUERY_RECORD);
				if(NULL == RecordFileQueryResPtr)
				{
					SYS_ERROR("RecordFileQueryResPtr malloc error.\n");
					break;
				}
				for(i_k=0;i_k<MAX_NUM_QUERY_RECORD; i_k++)
				{
					RecordFileQueryResPtr[i_k] = (RecordFileQueryResOut *)malloc(sizeof(
RecordFileQueryResOut));
					if(NULL == RecordFileQueryResPtr[i_k])
					{
						SYS_ERROR("RecordFileQueryResPtr[%d] malloc error.\n", i_k);
						break;
					}
					memset(RecordFileQueryResPtr[i_k], 0, sizeof(RecordFileQueryResOut));
				}

				if(i_k < MAX_NUM_QUERY_RECORD)
				{
					
break;
				}

			QueryLoop:
				for(i_k=0;i_k<MAX_NUM_QUERY_RECORD; i_k++)
				{
					memset(RecordFileQueryResPtr[i_k], 0, sizeof(RecordFileQueryResOut));
				}
			
				if(FAILED(GMI_RecordFileQuery(&RecordFileQueryPtr, &CurQueryPosNo, 
	                                 RecordFileQueryResPtr, QueryResArraySize,
	                                 &QueryResTotalNum, &QueryResCurNum)))
	            {
					
					SYS_ERROR("###GMI_RecordFileQuery fail, Result = 0x%lx\n", Result);
				    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "#####
GMI_RecordFileQuery, Result = 0x%lx\n", Result);
				}
				else
				{
					SYS_ERROR("###GMI_RecordFileQuery OK\n");
					for(i_k=0;i_k<QueryResCurNum; i_k++)
					{
						SYS_ERROR("RecFile %d:[%s]  [%d]  [%d-%d]\n", i_k, RecordFileQueryResPtr
[i_k]->s_RecFileName, 
							RecordFileQueryResPtr[i_k]->s_RecFileSize, RecordFileQueryResPtr[i_k]->
s_RecFileTime[0], RecordFileQueryResPtr[i_k]->s_RecFileTime[1]);
					}

					
					if(QueryResCurNum < QueryResTotalNum)
					{
						SYS_ERROR("###GMI_RecordFileQuery %d < %d\n", QueryResCurNum, 
QueryResTotalNum);
						goto QueryLoop;
					}

					
					RecordDownReplayQuery.s_RecQueryType = 0;
					memset(RecordDownReplayQuery.RecDownReplayInfo.s_RecFileName, 0, sizeof(
RecordDownReplayQuery.RecDownReplayInfo.s_RecFileName));
					memcpy(RecordDownReplayQuery.RecDownReplayInfo.s_RecFileName, 
RecordFileQueryResPtr[0]->s_RecFileName, strlen(RecordFileQueryResPtr[0]->
s_RecFileName));
					SYS_ERROR("RecordFileQueryResPtr[0]->s_RecFileName=%s, 
RecordDownReplayQuery.RecDownReplayInfo.s_RecFileName=%s.\n", 
						RecordFileQueryResPtr[0]->s_RecFileName, RecordDownReplayQuery.
RecDownReplayInfo.s_RecFileName);
				}

				
				for(i_k=0;i_k<MAX_NUM_QUERY_RECORD; i_k++)
				{
					if(NULL != RecordFileQueryResPtr[i_k])
					{
						free(RecordFileQueryResPtr[i_k]);
						RecordFileQueryResPtr[i_k] = NULL;
					}
				}
				if(NULL != RecordFileQueryResPtr)
				{
					free(RecordFileQueryResPtr);
					RecordFileQueryResPtr = NULL;
				}
				#endif
                break;
            case SYS_PTZCMD_RIGHT:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_Right;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
				#ifdef TEST_STORAGE
				RecordDownReplayQueryResPtr = (RecordDownReplayQueryResOut**)malloc(sizeof
(RecordDownReplayQueryResOut *)*QueryResArraySize2);
				if(NULL == RecordDownReplayQueryResPtr)
				{
					SYS_ERROR("RecordDownReplayQueryResPtr malloc error.\n");
					break;
				}
				for(i_k=0;i_k<QueryResArraySize2; i_k++)
				{
					RecordDownReplayQueryResPtr[i_k] = (RecordDownReplayQueryResOut *)malloc(
sizeof(RecordDownReplayQueryResOut));
					if(NULL == RecordDownReplayQueryResPtr[i_k])
					{
						SYS_ERROR("RecordDownReplayQueryResPtr[%d] malloc error.\n", i_k);
						break;
					}
					memset(RecordDownReplayQueryResPtr[i_k], 0, sizeof(
RecordDownReplayQueryResOut));
				}
				SYS_ERROR("RecordDownReplayQuery s_RecFileName=%s.\n", 
RecordDownReplayQuery.RecDownReplayInfo.s_RecFileName);
				if(FAILED(GMI_RecordDownReplayQuery(&RecordDownReplayQuery, 
RecordDownReplayQueryResPtr, QueryResArraySize2)))
				{
					SYS_ERROR("###GMI_RecordDownReplayQuery fail, Result = 0x%lx\n", Result);
				}
				else
				{
					SYS_ERROR("*********GMI_RecordDownReplayQuery*******\n");
					for(i_k=0;i_k<QueryResArraySize2; i_k++)
					{
						SYS_ERROR("s_RecQueryType=%d[0-down,1-replay]\n", 
RecordDownReplayQueryResPtr[i_k]->s_RecQueryType);
						SYS_ERROR("s_StreamType=%d[0-ps,1-es]\n", RecordDownReplayQueryResPtr[
i_k]->s_StreamType);
						SYS_ERROR("s_EncodeType=%d[0-none,1-h264,2-mjpeg]\n", 
RecordDownReplayQueryResPtr[i_k]->s_EncodeType);
						SYS_ERROR("s_AudioType=%d[0-none,1-G711a,2-G711u,3-G726]\n", 
RecordDownReplayQueryResPtr[i_k]->s_AudioType);
						SYS_ERROR("s_VideoFrame=%d\n", RecordDownReplayQueryResPtr[i_k]->
s_VideoFrame);
						SYS_ERROR("s_AudioFrame=%d\n", RecordDownReplayQueryResPtr[i_k]->
s_AudioFrame);
						SYS_ERROR("s_VideoWide=%d\n", RecordDownReplayQueryResPtr[i_k]->
s_VideoWide);
						SYS_ERROR("s_VideoHeight=%d\n", RecordDownReplayQueryResPtr[i_k]->
s_VideoHeight);
						SYS_ERROR("s_RecFileName=%s\n", RecordDownReplayQueryResPtr[i_k]->
s_RecFileName);
						SYS_ERROR("s_RecFileOffset=%d\n", RecordDownReplayQueryResPtr[i_k]->
s_RecFileOffset);
						SYS_ERROR("s_RecFileSize=%d\n", RecordDownReplayQueryResPtr[i_k]->
s_RecFileSize);
						SYS_ERROR("s_RecFileReplayNum=%d\n", RecordDownReplayQueryResPtr[i_k]->
s_RecFileReplayNum);
						SYS_ERROR("s_RecFileCurNo=%d\n", RecordDownReplayQueryResPtr[i_k]->
s_RecFileCurNo);
						SYS_ERROR("s_CurFileStartTime=%d\n", RecordDownReplayQueryResPtr[i_k]->
s_CurFileStartTime);
						SYS_ERROR("s_CurFileEndTime=%d\n", RecordDownReplayQueryResPtr[i_k]->
s_CurFileEndTime);
						SYS_ERROR("\n");
					}
					SYS_ERROR("***************************************\n");
				}
				for(i_k=0;i_k<QueryResArraySize2; i_k++)
				{
					if(NULL != RecordDownReplayQueryResPtr[i_k])
					{
						free(RecordDownReplayQueryResPtr[i_k]);
						RecordDownReplayQueryResPtr[i_k] = NULL;
					}
				}
				if(NULL != RecordDownReplayQueryResPtr)
				{
					free(RecordDownReplayQueryResPtr);
					RecordDownReplayQueryResPtr = NULL;
				}
				
				#endif
				break;
            case SYS_PTZCMD_UP:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_Up;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
				#ifdef TEST_STORAGE
				StorageUninitIn StorageUninitParam;
				memset(&StorageUninitParam, 0, sizeof(StorageUninitParam));
				if(FAILED(GMI_StorageDeviceUninit(&StorageUninitParam)))
				{
					
					SYS_ERROR("###GMI_StorageDeviceUninit fail, Result = 0x%lx\n", Result);
				    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "#####
GMI_StorageDeviceUninit, Result = 0x%lx\n", Result);
				}
				#endif
                break;
            case SYS_PTZCMD_DOWN:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_Down;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
				
				#ifdef TEST_STORAGE
				memset(&RecordScheduleConfig, 0, sizeof(RecordScheduleConfig));
				RecordScheduleConfig.s_Enable = 0;
				if(FAILED(GMI_RecordScheduleConfig(&RecordScheduleConfig)))
				{
					
					SYS_ERROR("###GMI_RecordScheduleConfig111 fail, Result = 0x%lx\n", Result
);
				    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "#####
GMI_RecordScheduleConfig111, Result = 0x%lx\n", Result);
				}
				#endif
                break;
            case SYS_PTZCMD_LEFTUP:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_LeftUp;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
                PT_CtlCmd.s_CmdParam[1] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
				#ifdef TEST_STORAGE
				StorageFormatIn StorageFormatParam;
				memset(&StorageFormatParam, 0, sizeof(StorageFormatParam));
				if(FAILED(GMI_StorageDeviceFormat(&StorageFormatParam)))
				{
					
					SYS_ERROR("###GMI_StorageDeviceFormat fail, Result = 0x%lx\n", Result);
				    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "#####
GMI_StorageDeviceFormat, Result = 0x%lx\n", Result);
				}
				#endif
				
                break;
            case SYS_PTZCMD_LEFTDOWN:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_LeftDown;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
                PT_CtlCmd.s_CmdParam[1] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
				#ifdef TEST_STORAGE
				StorageInitIn StorageInitParam;
				memset(&StorageInitParam, 0, sizeof(StorageInitParam));
				if(FAILED(GMI_StorageDeviceInit(&StorageInitParam)))
				{
					
					SYS_ERROR("###GMI_StorageDeviceInit fail, Result = 0x%lx\n", Result);
				    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "#####
GMI_StorageDeviceInit, Result = 0x%lx\n", Result);
				}
				#endif
                break;
            case SYS_PTZCMD_RIGHTUP:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_RightUp;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
                PT_CtlCmd.s_CmdParam[1] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
				#ifdef TEST_STORAGE
				RecordParamConfigIn RecordParamConfig;
				memset(&RecordParamConfig, 0, sizeof(RecordParamConfig));
				if(FAILED(GMI_RecordParamConfig(&RecordParamConfig)))
				{
					
					SYS_ERROR("###GMI_RecordParamConfig fail, Result = 0x%lx\n", Result);
				    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "#####
GMI_RecordParamConfig, Result = 0x%lx\n", Result);
				}
				#endif
                break;
            case SYS_PTZCMD_RIGHTDOWN:
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_RightDown;
                PT_CtlCmd.s_CmdParam[0] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
                PT_CtlCmd.s_CmdParam[1] = FloatToInt((float_t)PtzCtrlTmp.
s_Param[0]*100/255);
				#ifdef TEST_STORAGE
				memset(&RecordScheduleConfig, 0, sizeof(RecordScheduleConfig));
				RecordScheduleConfig.s_Enable = 1;
				RecordScheduleConfig.s_RecEndTime[0][0] = (24<<16);
				RecordScheduleConfig.s_RecEndTime[1][0] = (24<<16);
				RecordScheduleConfig.s_RecEndTime[2][0] = (24<<16);
				RecordScheduleConfig.s_RecEndTime[3][0] = (24<<16);
				RecordScheduleConfig.s_RecEndTime[4][0] = (24<<16);
				RecordScheduleConfig.s_RecEndTime[5][0] = (24<<16);
				RecordScheduleConfig.s_RecEndTime[6][0] = (24<<16);
				if(FAILED(GMI_RecordScheduleConfig(&RecordScheduleConfig)))
				{
					
					SYS_ERROR("###GMI_RecordScheduleConfig000 fail, Result = 0x%lx\n", Result
);
				    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "#####
GMI_RecordScheduleConfig000, Result = 0x%lx\n", Result);
				}
				#endif
                break;
            }

            Result = m_StreamCenterClientPtr->PauseAutoFocus(m_AutoFocusHandle
, true);
            if (FAILED(Result))
            {
                SYS_ERROR("PauseAutoFocus fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
PauseAutoFocus fail, Result = 0x%lx\n", Result);
                return Result;
            }

            Result = m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &
PT_CtlCmd);
            if (FAILED(Result))
            {
                SYS_ERROR("PTZ Control fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PTZ 
Control fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_PTZCMD_ZOOM_TELE == PtzCtrlTmp.s_PtzCmd
                 || SYS_PTZCMD_ZOOM_WIDE == PtzCtrlTmp.s_PtzCmd)
        {
            ZoomCmd    Z_CtlCmd;

            memset(&Z_CtlCmd, 0, sizeof(ZoomCmd));
            Z_CtlCmd.s_ZoomMode = ((SYS_PTZCMD_ZOOM_TELE == PtzCtrlTmp.
s_PtzCmd) ? ZOOM_MODE_IN : ZOOM_MODE_OUT);
            Result = m_StreamCenterClientPtr->StartZoom(m_AutoFocusHandle, 
m_ZoomHandle, Z_CtlCmd.s_ZoomMode);
            if (FAILED(Result))
            {
                SYS_ERROR("StartZoom fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
StartZoom fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_PTZCMD_GMI_MANUAL_FOCUS == PtzCtrlTmp.s_PtzCmd)
        {
            Result = m_StreamCenterClientPtr->AutoFocusGlobalScan(
m_AutoFocusHandle);
            if (FAILED(Result))
            {
                SYS_ERROR("AutoFocusGlobalScan fail, Result = 0x%lx\n", Result
);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
AutoFocusGlobalScan fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_PTZCMD_FOCUS_FAR == PtzCtrlTmp.s_PtzCmd
                 || SYS_PTZCMD_FOCUS_NEAR == PtzCtrlTmp.s_PtzCmd)
        {
            if (AF_MODE_MANUAL == m_FocusMode)
            {
                int32_t Mode = SYS_PTZCMD_FOCUS_FAR == PtzCtrlTmp.s_PtzCmd ? 
AF_DIR_MODE_OUT : AF_DIR_MODE_IN;

                Result = m_StreamCenterClientPtr->ControlAutoFocus(
m_AutoFocusHandle, Mode);
                if (FAILED(Result))
                {
                    SYS_ERROR("ControlAutoFocus fail, Result = 0x%lx\n", 
Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
ControlAutoFocus fail, Result = 0x%lx\n", Result);
                    return Result;
                }
            }
            else
            {
                SYS_INFO("not support focus near or far on focusmode %d\n", 
m_FocusMode);
            }
        }
        else if (SYS_PTZCMD_SETPRESET == PtzCtrlTmp.s_PtzCmd)
        {
            PT_CtlCmd.s_Cmd = e_PTZ_CMD_SetPreset;
            PT_CtlCmd.s_CmdParam[0] = PtzCtrlTmp.s_Param[0];

            if (1 > PT_CtlCmd.s_CmdParam[0]
                    || 256 < PT_CtlCmd.s_CmdParam[0] )
            {
                return GMI_INVALID_PARAMETER;
            }

            Result = m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &
PT_CtlCmd);
            if (FAILED(Result))
            {
                SYS_ERROR("PTZ Control fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PTZ 
Control fail, Result = 0x%lx\n", Result);
                return Result;
            }

            int32_t ZoomPos;
            Result = m_StreamCenterClientPtr->GetZoomPosition(m_ZoomHandle, &
ZoomPos);
            if (FAILED(Result))
            {
                SYS_ERROR("GetZoomPosition fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
GetZoomPosition fail, Result = 0x%lx\n", Result);
                return Result;
            }

            uint32_t Id = PT_CtlCmd.s_CmdParam[0];
            sprintf((m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Name, "Preset%d", 
Id);
            (m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Index        = Id;
            (m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Setted       = true;
            (m_PresetsInfo_InnerPtr.GetPtr())[Id].s_ZoomPosition = ZoomPos;
            Result = m_ConfigFileManagerPtr->SetPresetsInfo(&((
m_PresetsInfo_InnerPtr.GetPtr())[Id]));
            if (FAILED(Result))
            {
                (m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Setted   = false;
                SYS_ERROR("m_ConfigFileManagerPtr SetPresetsInfo fail, Result 
= 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
m_ConfigFileManagerPtr SetPresetsInfo fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_PTZCMD_GOTOPRESET == PtzCtrlTmp.s_PtzCmd)
        {
            PT_CtlCmd.s_Cmd = e_PTZ_CMD_GotoPreset;
            PT_CtlCmd.s_CmdParam[0] = PtzCtrlTmp.s_Param[0];
            if (1 > PT_CtlCmd.s_CmdParam[0]
                    || 256 < PT_CtlCmd.s_CmdParam[0] )
            {
                return GMI_INVALID_PARAMETER;
            }

            uint32_t Id = PT_CtlCmd.s_CmdParam[0];
            int32_t ZoomPos = (m_PresetsInfo_InnerPtr.GetPtr())[Id].
s_ZoomPosition;

            if ((m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Setted)
            {        
                Result = m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &
PT_CtlCmd);
                if (FAILED(Result))
                {
                    SYS_ERROR("PTZ Control fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
PTZ Control fail, Result = 0x%lx\n", Result);
                    return Result;
                }

                Result = m_StreamCenterClientPtr->SetZoomPosition(m_ZoomHandle
, ZoomPos);
                if (FAILED(Result))
                {
                    SYS_ERROR("SetZoomPosition fail, Result = 0x%lx\n", Result
);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
SetZoomPosition fail, Result = 0x%lx\n", Result);
                    return Result;
                }               
            }
        }
        else if (SYS_PTZCMD_CLEARPRESET == PtzCtrlTmp.s_PtzCmd)
        {
            PtzCtlCmd  PT_CtlCmd;

            memset(&PT_CtlCmd, 0, sizeof(PtzCtlCmd));

            PT_CtlCmd.s_Cmd = e_PTZ_CMD_RmPreset;
            PT_CtlCmd.s_CmdParam[0] = PtzCtrlTmp.s_Param[0];
            if (1 > PT_CtlCmd.s_CmdParam[0]
                    || 256 < PT_CtlCmd.s_CmdParam[0] )
            {
                return GMI_INVALID_PARAMETER;
            }

            Result = m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &
PT_CtlCmd);
            if (FAILED(Result))
            {
                SYS_ERROR("PTZ Control fail, Result = 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "PTZ 
Control fail, Result = 0x%lx\n", Result);
                return Result;
            }

            uint32_t Id = PT_CtlCmd.s_CmdParam[0];
            (m_PresetsInfo_InnerPtr.GetPtr())[Id].s_Setted = false;
            Result = m_ConfigFileManagerPtr->SetPresetsInfo(&((
m_PresetsInfo_InnerPtr.GetPtr())[Id]));
            if (FAILED(Result))
            {
                SYS_ERROR("m_ConfigFileManagerPtr SetPresetsInfo fail, Result 
= 0x%lx\n", Result);
                DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
m_ConfigFileManagerPtr SetPresetsInfo fail, Result = 0x%lx\n", Result);
                return Result;
            }
        }
        else if (SYS_PTZCMD_STOP == PtzCtrlTmp.s_PtzCmd)
        {
            if (SYS_PTZCMD_LEFT        == LastPtzCmd
                    || SYS_PTZCMD_RIGHT     == LastPtzCmd
                    || SYS_PTZCMD_UP        == LastPtzCmd
                    || SYS_PTZCMD_DOWN      == LastPtzCmd
                    || SYS_PTZCMD_LEFTUP    == LastPtzCmd
                    || SYS_PTZCMD_LEFTDOWN  == LastPtzCmd
                    || SYS_PTZCMD_RIGHTUP   == LastPtzCmd
                    || SYS_PTZCMD_RIGHTDOWN == LastPtzCmd)
            {
                PT_CtlCmd.s_Cmd = e_PTZ_CMD_Stop;
                Result = m_PtzControlPtr->Control(PTZ_CONTINUE_CONTROL_MODE, &
PT_CtlCmd);
                if (FAILED(Result))
                {
                    SYS_ERROR("PTZ Control fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
PTZ Control fail, Result = 0x%lx\n", Result);
                    return Result;
                }

                Result = m_StreamCenterClientPtr->PauseAutoFocus(
m_AutoFocusHandle, false);
                if (FAILED(Result))
                {
                    SYS_ERROR("PauseAutoFocus fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
PauseAutoFocus fail, Result = 0x%lx\n", Result);
                    return Result;
                }
            }
            else if (SYS_PTZCMD_ZOOM_TELE == LastPtzCmd
                     || SYS_PTZCMD_ZOOM_WIDE == LastPtzCmd)
            {
                Result = m_StreamCenterClientPtr->StopZoom(m_AutoFocusHandle, 
m_ZoomHandle, &m_ZoomPos);
                if (FAILED(Result))
                {
                    SYS_ERROR("StopZoom fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
StopZoom fail, Result = 0x%lx\n", Result);
                    return Result;
                }

                //signal recordzoompos thread to save current zoom position. 
11/7/2013 guoqiang.lu
                m_RecordZoomNotify.Signal();
            }
            else
            {
                if (AF_MODE_MANUAL == m_FocusMode)//on manual focus mode , 
donot need resume autofocus task
                {
                    Result = m_StreamCenterClientPtr->ControlAutoFocus(
m_AutoFocusHandle, AF_DIR_MODE_STOP);
                    if (FAILED(Result))
                    {
                        SYS_ERROR("ControlAutoFocus fail, Result = 0x%lx\n", 
Result);
                        DEBUG_LOG(g_DefaultLogClient, 
e_DebugLogLevel_Exception, "ControlAutoFocus fail, Result = 0x%lx\n", Result);
                        return Result;
                    }
                }

                Result = m_StreamCenterClientPtr->PauseAutoFocus(
m_AutoFocusHandle, false);
                if (FAILED(Result))
                {
                    SYS_ERROR("PauseAutoFocus fail, Result = 0x%lx\n", Result);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "
PauseAutoFocus fail, Result = 0x%lx\n", Result);
                    return Result;
                }
            }
        }
        else
        {
            SYS_ERROR("not support ptz cmd %d\n", PtzCtrlTmp.s_PtzCmd);
            return GMI_NOT_SUPPORT;
        }
        LastPtzCmd = PtzCtrlTmp.s_PtzCmd;
    }

    SYS_INFO("%s waste Time %u, normal out..........\n", __func__, TimeStamp(
) - Time1);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s waste Time %u, 
normal out..........\n", __func__, TimeStamp() - Time1);
    return GMI_SUCCESS;
}

