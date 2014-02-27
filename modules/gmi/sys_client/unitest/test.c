#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sys_client.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_system_headers.h"


int main()
{
    int i = 1;
    GMI_RESULT Result;
#define    MAX_XML_BUFFER_LENTH       2048

    while (i--)
    {
    

        Result = SysInitialize(65535);
        if (FAILED(Result))
        {
        	printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
        	goto errExit;
        }		
		
		#if 0
		SysPkgLogInfoSearch SysLogInfoSearch;
		SysPkgLogInfoInt    SysLogInfoInt;
		SysPkgLogInfo       *SysLogInfoPtr;
		SysLogInfoSearch.s_SelectMode = 0;
		SysLogInfoSearch.s_MajorType = 0;
		SysLogInfoSearch.s_MinorType = 0;
		strcpy(SysLogInfoSearch.s_StartTime, "2014-2-26 00:00:00");
		strcpy(SysLogInfoSearch.s_StopTime, "2014-2-27 00:00:00");
		SysLogInfoSearch.s_Offset = 0;
		SysLogInfoSearch.s_MaxNum = 20;
		SysLogInfoPtr = (SysPkgLogInfo*)malloc(SysLogInfoSearch.s_MaxNum*sizeof(SysPkgLogInfo));
		Result = SysGetLogInfo(1234, 0, &SysLogInfoSearch, &SysLogInfoInt, SysLogInfoPtr);
		if (FAILED(Result))
		{
			printf("SysGetLogInfo fail\n");
			goto errExit;
		}
		printf("%lld %s %s %s\n", SysLogInfoPtr->s_LogId, SysLogInfoPtr->s_LogTime, SysLogInfoPtr->s_UserName, SysLogInfoPtr->s_LogData);
		#endif
		
		SysPkgAlarmEventConfig SysAlarmEventConfig;
		Result = SysGetAlarmConfig(0, 0, SYS_DETECTOR_ID_PIR, 0, &SysAlarmEventConfig, sizeof(SysPkgAlarmEventConfig));
		if (FAILED(Result))
		{
		    printf("SysGetAlarmConfig %d fail\n", SYS_DETECTOR_ID_PIR);
		    goto errExit;
		}
		
		SysAlarmEventConfig.s_AlarmId = SYS_DETECTOR_ID_PIR;
		SysAlarmEventConfig.s_EnableFlag   = 0;	
	    SysAlarmEventConfig.s_CheckTime    = 100;	
	    SysAlarmEventConfig.s_LinkAlarmStrategy = 1;
	    SysAlarmEventConfig.s_LinkAlarmExtInfo.s_OperateSeqNum = 1;
		Result = SysSetAlarmConfig(0, 0, SYS_DETECTOR_ID_PIR, 0, &SysAlarmEventConfig, sizeof(SysPkgAlarmEventConfig));
		if (FAILED(Result))
		{
		    printf("SysSetAlarmConfig %d fail\n", SYS_DETECTOR_ID_PIR);
		    goto errExit;
		}
		
		Result = SysGetAlarmConfig(0, 0, SYS_DETECTOR_ID_PIR, 0, &SysAlarmEventConfig, sizeof(SysPkgAlarmEventConfig));
		if (FAILED(Result))
		{
		    printf("SysGetAlarmConfig %d fail\n", SYS_DETECTOR_ID_PIR);
		    goto errExit;
		}
		printf("%d %d %d %d\n", SysAlarmEventConfig.s_EnableFlag,  SysAlarmEventConfig.s_CheckTime,\
		    SysAlarmEventConfig.s_LinkAlarmStrategy, SysAlarmEventConfig.s_LinkAlarmExtInfo.s_OperateSeqNum);
		goto errExit;
		
		SysPkgAlarmInConfig SysAlarmInConfig;
		Result = SysGetAlarmConfig(0, 0, SYS_DETECTOR_ID_ALARM_INPUT, 0, &SysAlarmInConfig, sizeof(SysPkgAlarmInConfig));
		if (FAILED(Result))
		{
		    printf("SysGetAlarmConfig %d fail\n", SYS_DETECTOR_ID_ALARM_INPUT);
		    goto errExit;
		}
		
		SysAlarmInConfig.s_EnableFlag   = 1;
	    SysAlarmInConfig.s_InputNumber  = 0;
	    SysAlarmInConfig.s_CheckTime    = 50;
	    SysAlarmInConfig.s_NormalStatus = 1;
	    SysAlarmInConfig.s_LinkAlarmStrategy = 1;
	    SysAlarmInConfig.s_LinkAlarmExtInfo.s_OperateSeqNum = 1;
		Result = SysSetAlarmConfig(0, 0, SYS_DETECTOR_ID_ALARM_INPUT, 0, &SysAlarmInConfig, sizeof(SysPkgAlarmInConfig));
		if (FAILED(Result))
		{
		    printf("SysSetAlarmConfig %d fail\n", SYS_DETECTOR_ID_ALARM_INPUT);
		    goto errExit;
		}
		
		Result = SysGetAlarmConfig(0, 0, SYS_DETECTOR_ID_ALARM_INPUT, 0, &SysAlarmInConfig, sizeof(SysPkgAlarmInConfig));
		if (FAILED(Result))
		{
		    printf("SysGetAlarmConfig %d fail\n", SYS_DETECTOR_ID_ALARM_INPUT);
		    goto errExit;
		}
		printf("%d %d %d %d %d %d\n", SysAlarmInConfig.s_EnableFlag, SysAlarmInConfig.s_InputNumber, SysAlarmInConfig.s_CheckTime,\
		    SysAlarmInConfig.s_NormalStatus,  SysAlarmInConfig.s_LinkAlarmStrategy, SysAlarmInConfig.s_LinkAlarmExtInfo.s_OperateSeqNum);
		
		SysPkgAlarmScheduleTime SysAlarmSchTime;
		SysAlarmSchTime.s_ScheduleId = 1;
		SysAlarmSchTime.s_Index = 0;
		SysAlarmSchTime.s_ScheduleTime[0][0].s_StartTime = 0;
		SysAlarmSchTime.s_ScheduleTime[0][0].s_EndTime = 60*10;
		Result = SysSetAlmScheduleTime(0, 0, 1, 0, &SysAlarmSchTime);
		if (FAILED(Result))
		{
		    printf("SysSetAlmScheduleTime fail\n");
		    goto errExit;
		}
		
		memset(&SysAlarmSchTime, 0, sizeof(SysPkgAlarmScheduleTime));		
		Result = SysGetAlmScheduleTime(0, 0, 1, 0, &SysAlarmSchTime);
		if (FAILED(Result))
		{
		    printf("SysSetAlmScheduleTime fail\n");
		    goto errExit;
		}
		printf("%d,%d,%d,%d\n", SysAlarmSchTime.s_ScheduleId, SysAlarmSchTime.s_Index, SysAlarmSchTime.s_ScheduleTime[0][0].s_StartTime, SysAlarmSchTime.s_ScheduleTime[0][0].s_EndTime);
		goto errExit;
		
		Result = SysStop3A(1234, 0);
		if (FAILED(Result))
		{
			printf("stop 3A fail\n");
			goto errExit;
		}
		printf("stop 3a OK\n");
		
		
		goto errExit;
		
		// printf("==========>Test SysGetNetworkPort\n");
        // SysPkgNetworkPort SysNetworkPort;

        // Result = SysGetNetworkPort(1234, 0, &SysNetworkPort);
        // if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
        // printf("SysNetworkPort %d\n", SysNetworkPort.s_Upgrade_Port);        

        // SysPkgEncStreamCombine SysEncStreamCombine;
		// Result = SysGetVideoEncStreamCombine(1234, 0, &SysEncStreamCombine);
		// if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
        // printf("s_EnableStreamNum %d, s_StreamCombineNo %d\n", SysEncStreamCombine.s_EnableStreamNum, SysEncStreamCombine.s_StreamCombineNo);
        // Result = SysSetVideoEncStreamCombine(1234, 0, &SysEncStreamCombine);
		// if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
        // goto errExit;
        // printf("=========>Test SysStartAudioDecode");
        // SysPkgAudioDecParam AudioDecodeParam;
        // AudioDecodeParam.s_Codec        = 1;
        // AudioDecodeParam.s_SampleFreq   = 8000;
        // AudioDecodeParam.s_BitWidth     = 16;
        // AudioDecodeParam.s_ChannelNum   = 1;
        // AudioDecodeParam.s_FrameRate    = 50;
        // AudioDecodeParam.s_BitRate      = 64;
        // AudioDecodeParam.s_Volume       = 50;
        // AudioDecodeParam.s_AecFlag      = 0;
        // AudioDecodeParam.s_AecDelayTime = 0;
        
        // Result = SysStartAudioDecode(1234, 0, &AudioDecodeParam);
        // if (FAILED(Result))
		// {
			// printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
		// }
		
		// sleep(5);
		// Result = SysStopAudioDecode(1234, 0, 1);
        // if (FAILED(Result))
		// {
			// printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
		// }	
		// goto errExit;

		// SysPkgDateTimeType SysDateTimeType;
		// SysPkgSysTime SysTime;
		// SysPkgTimeZone SysTimezone;		
		// SysPkgNtpServerInfo SysNtpServerInfo;
        // Result = SysGetTime(1234, 0, &SysDateTimeType, &SysTime, &SysTimezone, &SysNtpServerInfo);
        // if (FAILED(Result))
		// {
			// printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
		// }
		// SysDateTimeType.s_Type = 1;
		// SysDateTimeType.s_NtpInterval = 5;
		// Result = SysSetTime(1234, 0, &SysDateTimeType, &SysTime, &SysTimezone, &SysNtpServerInfo);
        // if (FAILED(Result))
		// {
			// printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
		// }
		

		// SysPkgXml  SysCapabilitesXml;		
		// int32_t MessageLength = 4096*1024;
		// char_t *Message = (char_t*)malloc(MessageLength);
		// Result = SysGetCapabilities(1234, 0, Message, MessageLength, &SysCapabilitesXml);
		// if (FAILED(Result))
		// {
			// printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
		// }
		// printf("Message %s\n", Message);

		// printf("=========>Test FactorySimpleDefaultAll\n");
        // Result = FactorySimpleDefaultAll(1234, 0);
        // if (FAILED(Result))
		// {
			// printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
		// }
        // goto errExit;

        // printf("=========>Test SysGetVideoSourceMirror\n");
		// int32_t Mirror;
		// Result = SysGetVideoSourceMirror(1234, 0, &Mirror);
		// if (FAILED(Result))
		// {
			// printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
		// }
		// printf("Mirror = %d\n", Mirror);
		
		// printf("=========>Test SysSetVideoSourceMirror");
		// Result = SysSetVideoSourceMirror(1234, 0, 1);
		// if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }        
        

        // printf("=========>Test SysGetDaynight\n");
        // SysPkgDaynight SysDaynight;
        // Result = SysGetDaynight(1234, 0, &SysDaynight);
        // if (FAILED(Result))
        // {
        	// printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
		// printf("SysDaynight.s_Mode %dn", SysDaynight.s_Mode);
        // SysDaynight.s_Mode = 1;
        // Result = SysSetDaynight(1234, 0, &SysDaynight);
        // if (FAILED(Result))
        // {
        	// printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }

        // printf("=========>Test FactoryDefaultImaging");
        // Result = FactoryDefaultImaging(1234, 0);
        // if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
           // goto errExit;
        // }
		
		// /*
		// Result = SysReboot(1234, 0);
		// if (FAILED(Result))
		// {
			// printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
			// errExit(1);
		// }
		// */

        // printf("========>Test SysSetDeviceIP\n");
        // SysPkgIpInfo SysIpInfo;
        
        // Result = SysGetDeviceIP(1234, 0, &SysIpInfo);
        // if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
        // printf("IP %s\n", SysIpInfo.s_IpAddress);
        // printf("NetMask %s\n", SysIpInfo.s_SubNetMask);
        // printf("Gateway %s\n", SysIpInfo.s_GateWay); 
        
        // //strcpy(SysIpInfo.s_IpAddress, "192.168.0.125");
        // //Result = SysSetDeviceIP(1234, 0, &SysIpInfo);
        // //if (FAILED(Result))
        // //{
        // //    printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
        // //    errExit(1);
        // //}
        // //errExit(0);
		
        // printf("========>Test SysGetWhiteBalance\n");
        // SysPkgWhiteBalance SysWhiteBalance;

        // Result = SysGetWhiteBalance(1234, 0, &SysWhiteBalance);
        // if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
        // printf("white balance:mode %d, rgain %d, bgain %d\n", SysWhiteBalance.s_Mode, SysWhiteBalance.s_RGain, SysWhiteBalance.s_BGain);

        // SysWhiteBalance.s_Mode = 1;
        // Result = SysSetWhiteBalance(1234, 0, &SysWhiteBalance);
        // if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
        // Result = SysGetWhiteBalance(1234, 0, &SysWhiteBalance);
        // if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
        // printf("white balance:mode %d, rgain %d, bgain %d\n", SysWhiteBalance.s_Mode, SysWhiteBalance.s_RGain, SysWhiteBalance.s_BGain);


        // printf("=========>Test SysGetDeviceStartedTime\n");
        // SysPkgSysTime SysDevTime;
        // Result = SysGetDeviceStartedTime(&SysDevTime);
        // if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
        // printf("Date:%d-%d-%d Time:%d:%d:%d\n", SysTime.s_Year, SysTime.s_Month, SysTime.s_Day, SysTime.s_Hour, SysTime.s_Minute, SysTime.s_Second);


        // printf("==========>Test SysGetEncodeStreamNum\n");
        // uint32_t VidStreamNum;
        // Result = SysGetEncodeStreamNum(1234, 0, &VidStreamNum);
        // if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
        // printf("StreamNum %d\n", VidStreamNum);

        // printf("==========>TestSysGetWorkState\n");
        // char_t XmlBuf[MAX_XML_BUFFER_LENTH] = {"0"};
        // SysPkgXml SysXml;
        // memset(&SysXml, 0, sizeof(SysPkgXml));
        // memset(XmlBuf, 0, sizeof(XmlBuf));
        // Result = SysGetWorkState(1234, 0, XmlBuf, MAX_XML_BUFFER_LENTH, &SysXml);
        // if (FAILED(Result))
        // {
            // printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            // goto errExit;
        // }
        // printf("%s\n", XmlBuf);


        printf("==========>TestSysGetUsers\n");
		uint32_t RealUserCnt;
        SysPkgUserInfo SysUserInfo[32];
        SysUserInfo[0].s_UserFlag = 3;
        SysUserInfo[0].s_UserLevel = 0;
        strcpy(SysUserInfo[0].s_UserName, "luguoqiang");
        strcpy(SysUserInfo[0].s_UserPass, "123456");        
        Result = SysSetUsers(1234, 0, &SysUserInfo[0]);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            goto errExit;
        }

		SysUserInfo[0].s_UserFlag = 3;
        SysUserInfo[0].s_UserLevel = 0;
        strcpy(SysUserInfo[0].s_UserName, "luguoqiang111");
        strcpy(SysUserInfo[0].s_UserPass, "123456");
        
        Result = SysSetUsers(1234, 0, &SysUserInfo[0]);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            goto errExit;
        }

        SysUserInfo[0].s_UserFlag = 3;
        SysUserInfo[0].s_UserLevel = 6;
        strcpy(SysUserInfo[0].s_UserName, "luguoqiang222");
        strcpy(SysUserInfo[0].s_UserPass, "123456");
        
        Result = SysSetUsers(1234, 0, &SysUserInfo[0]);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            goto errExit;
        }

        SysUserInfo[0].s_UserFlag = 3;
        SysUserInfo[0].s_UserLevel = 5;
        strcpy(SysUserInfo[0].s_UserName, "luguoqiang333");
        strcpy(SysUserInfo[0].s_UserPass, "12356");
        
        Result = SysModifyUsers(1234, 0, &SysUserInfo[0]);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            //errExit(1);
        }

        SysUserInfo[0].s_UserFlag = 3;
        SysUserInfo[0].s_UserLevel = 4;
        strcpy(SysUserInfo[0].s_UserName, "luguoqiang555");
        strcpy(SysUserInfo[0].s_UserPass, "123456");
        
        Result = SysAddUsers(1234, 0, &SysUserInfo[0]);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            //errExit(1);
        }
               
        Result = SysGetUsers(1234, 0, SysUserInfo, 32, &RealUserCnt);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            goto errExit;
        }

        uint32_t Cnt;
        for (Cnt = 0; Cnt < RealUserCnt; Cnt++)
        {
            printf("%d: %s, %s, %d, %d\n", Cnt, SysUserInfo[Cnt].s_UserName, SysUserInfo[Cnt].s_UserPass, SysUserInfo[Cnt].s_UserFlag, SysUserInfo[Cnt].s_UserLevel);
        }

        printf("=========>TestSysGetShowInfo\n");
        uint32_t StreamNum;
        Result = SysGetEncodeStreamNum(1234, 0, &StreamNum);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            goto errExit;
        }

        SysPkgShowCfg *SysShowInfoPtr = NULL;
        SysShowInfoPtr = (SysPkgShowCfg*)malloc(StreamNum * sizeof(SysPkgShowCfg));
        if (NULL == SysShowInfoPtr)
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            goto errExit;
        }

        uint32_t RspCnt;
        Result = SysGetShowInfo(1234, 0, SysShowInfoPtr, StreamNum, &RspCnt);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            free(SysShowInfoPtr);
            goto errExit;
        }

        printf("ShowInfo Num %d\n", RspCnt);

        uint32_t k;
        for (k = 0; k < StreamNum; k++)
        {
            printf("VideoId               = %d\n", SysShowInfoPtr[k].s_VideoId );
            printf("StreamId              = %d\n", SysShowInfoPtr[k].s_Flag );

            printf("TimeDisplay.Enable    = %d\n", SysShowInfoPtr[k].s_TimeInfo.s_Enable);
            printf("TimeDisplay.DateStyle = %d\n", SysShowInfoPtr[k].s_TimeInfo.s_DateStyle);
            printf("TimeDisplay.TimeStyle = %d\n", SysShowInfoPtr[k].s_TimeInfo.s_TimeStyle);
            printf("TimeDisplay.FontColor = %d\n", SysShowInfoPtr[k].s_TimeInfo.s_FontColor);
            printf("TimeDisplay.FontSize  = %d\n", SysShowInfoPtr[k].s_TimeInfo.s_FontSize);
            printf("TimeDisplay.FontBlod  = %d\n", SysShowInfoPtr[k].s_TimeInfo.s_FontBlod);
        }

        printf("=========>TestSysSetShowInfo\n");
        strcpy(SysShowInfoPtr[0].s_ChannelInfo.s_ChannelName, "TestSysSetShowInfo");
        Result = SysSetShowInfo(1234, 0, &SysShowInfoPtr[0]);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            free(SysShowInfoPtr);
            goto errExit;
        }

        printf("========>Test AutoFocus Global Scan");
        Result = SysFocusGlobalScan(1234, 0);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            goto errExit;
        }

        printf("=========>Test GetAutoFocusCfg");
        SysPkgAutoFocus SysAutoFocus;
        Result = SysGetAutoFocusCfg(1234, 0, &SysAutoFocus);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            goto errExit;
        }

        printf("FocusMode = %d\n", SysAutoFocus.s_FocusMode);

        SysAutoFocus.s_FocusMode = 2;

        Result = SysSetAutoFocusCfg(1234, 0, &SysAutoFocus);
        if (FAILED(Result))
        {
            printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
            goto errExit;
        }

        /*
        printf("=========>Teset SYS_PTZCMD_FOCUS_NEAR\n");
        SysPkgPtzCtrl SysPtzCtrl;
        memset(&SysPtzCtrl, 0, sizeof(SysPkgPtzCtrl));
        SysPtzCtrl.s_PtzId = 1;
        SysPtzCtrl.s_PtzCmd = SYS_PTZCMD_FOCUS_NEAR;
        Result = SysPtzControl(1234, 0, &SysPtzCtrl);
        if (FAILED(Result))
        {
        	printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
        	free(SysShowInfoPtr);
            errExit(1);
        }
        sleep(10);
        printf("=========>Teset SYS_PTZCMD_STOP\n");
        memset(&SysPtzCtrl, 0, sizeof(SysPkgPtzCtrl));
        SysPtzCtrl.s_PtzId = 1;
        SysPtzCtrl.s_PtzCmd = SYS_PTZCMD_STOP;
        Result = SysPtzControl(1234, 0, &SysPtzCtrl);
        if (FAILED(Result))
        {
        	printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
        	free(SysShowInfoPtr);
            errExit(1);
        }
        sleep(3);

        printf("=========>Teset SYS_PTZCMD_FOCUS_FAR\n");
        memset(&SysPtzCtrl, 0, sizeof(SysPkgPtzCtrl));
        SysPtzCtrl.s_PtzId = 1;
        SysPtzCtrl.s_PtzCmd = SYS_PTZCMD_FOCUS_FAR;
        Result = SysPtzControl(1234, 0, &SysPtzCtrl);
        if (FAILED(Result))
        {
        	printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
        	free(SysShowInfoPtr);
            errExit(1);
        }
        sleep(10);
        printf("=========>Teset SYS_PTZCMD_STOP\n");
        memset(&SysPtzCtrl, 0, sizeof(SysPkgPtzCtrl));
        SysPtzCtrl.s_PtzId = 1;
        SysPtzCtrl.s_PtzCmd = SYS_PTZCMD_STOP;
        Result = SysPtzControl(1234, 0, &SysPtzCtrl);
        if (FAILED(Result))
        {
        	printf("Line %d, Result = 0x%lx\n", __LINE__, Result);
        	free(SysShowInfoPtr);
            errExit(1);
        }
        */
       
        SysDeinitialize();
    }

    return 0;
errExit: 
    SysDeinitialize();
    return -1;
}

