#include "storage_common.h"

#include "gmi_storage_ctrl.h"
#include "log_record.h"
#include "storage_manager.h"



GMI_RESULT GMI_StorageDeviceFormat(StorageFormatIn *StorageFormatParamPtr)
{
	int32_t RetVal = GMI_SUCCESS;
	if((NULL == StorageFormatParamPtr))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");				
		return GMI_INVALID_PARAMETER;
	}

	switch(StorageFormatParamPtr->s_StorageDevice)
	{
		case TYPE_STORAGE_USB:
		case TYPE_STORAGE_NAS:
			DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "DeviceType %d (0-sd,1-usb,2-nas)no support.\n", StorageFormatParamPtr->s_StorageDevice);
			RetVal = GMI_NOT_SUPPORT;
			break;
		case TYPE_STORAGE_SD:
		default:
			if( LOCAL_RET_OK != Sdformat(StorageFormatParamPtr->s_RecFileSize))
			{
				DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "Sdformat error[%s].\n", strerror(errno));
				RetVal = GMI_FAIL;
			}
			break;
	}

	return RetVal;
}

GMI_RESULT GMI_StorageDeviceInit(StorageInitIn *StorageInitParamPtr)
{
	int32_t RetVal = GMI_SUCCESS;
	if(NULL == StorageInitParamPtr)
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return GMI_INVALID_PARAMETER;
	}

	if(LOCAL_RET_OK != VidRecordInit())
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "VidRecordInit error.\n");
		return GMI_SYSTEM_ERROR;
	}

	switch(StorageInitParamPtr->s_StorageDevice)
	{
		case TYPE_STORAGE_USB:
		case TYPE_STORAGE_NAS:
			DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "DeviceType %d (0-sd,1-usb,2-nas)no support.\n", StorageInitParamPtr->s_StorageDevice);
			RetVal = GMI_NOT_SUPPORT;
			break;
		case TYPE_STORAGE_SD:
		default:
			if( LOCAL_RET_OK != InitPartion(StorageInitParamPtr->s_RecFileSize))
			{
				DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "Sdformat error[%s].\n", strerror(errno));
				RetVal = GMI_FAIL;
			}
			break;
	}

	do
	{
		if(GMI_SUCCESS == RetVal)
		{
			if(LOCAL_RET_OK != CreateRecProcessThread(0))
			{
				RetVal = GMI_FAIL;
				break;
			}

			if(LOCAL_RET_OK != CreateDbProcessThread())
			{
				RetVal = GMI_FAIL;
				break;
			}
			
		}
	}while(0);

	return RetVal;
}

GMI_RESULT GMI_StorageDeviceUninit(StorageUninitIn *StorageUninitParamPtr)
{
	int32_t RetVal = GMI_SUCCESS;
	if(NULL == StorageUninitParamPtr)
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return GMI_INVALID_PARAMETER;
	}

	printf("test11122\n");
	if(LOCAL_RET_OK != VidRecordUninit())
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "VidRecordUninit error.\n");
		return GMI_SYSTEM_ERROR;	
	}
	printf("test3333\n");

	return RetVal;
}


GMI_RESULT GMI_StorageDeviceStatusQuery(StorageStatusQueryIn *DevStatusQueryPtr,
	                                   StorageStatusQueryResOut **DevStatusQueryResPtr, 
	                                   uint32_t DevStatusQueryNum,uint32_t  *DevStatusNum)
{
	int32_t RetVal = GMI_SUCCESS;
	if((NULL == DevStatusQueryPtr)
		|| (NULL == DevStatusQueryResPtr)
		|| (NULL == *DevStatusQueryResPtr)
		|| (NULL == DevStatusNum)
		|| (0 == DevStatusQueryNum))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return GMI_INVALID_PARAMETER;
	}

	switch(DevStatusQueryPtr->s_StorageDevice)
	{
		case TYPE_STORAGE_USB:
		case TYPE_STORAGE_NAS:
			DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "DeviceType %d (0-sd,1-usb,2-nas)no support.\n", DevStatusQueryPtr->s_StorageDevice);
			RetVal = GMI_NOT_SUPPORT;
			break;
		case TYPE_STORAGE_SD:
		default:
			if(LOCAL_RET_OK != GetSDStatus(DevStatusQueryResPtr, DevStatusQueryNum, DevStatusNum))
			{
				DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "GetSDStatus error.\n");
				RetVal = GMI_FAIL;
			}
			break;
	}

	return RetVal;
}

GMI_RESULT GMI_RecordParamConfig(RecordParamConfigIn *RecordParamConfigPtr)
{
	if(NULL == RecordParamConfigPtr)
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return GMI_INVALID_PARAMETER;
	}

	if(LOCAL_RET_OK != SetRecConfigParam(RecordParamConfigPtr))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "SetRecConfigParam error.\n");
		return GMI_FAIL;
	}

	return GMI_SUCCESS;
}

GMI_RESULT  GMI_RecordScheduleConfig(RecordScheduleConfigIn *RecordScheduleConfigPtr)
{
	if(NULL == RecordScheduleConfigPtr)
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return GMI_INVALID_PARAMETER;
	}

	if(LOCAL_RET_OK != SetRecScheduleConfig(RecordScheduleConfigPtr))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "SetRecConfigParam error.\n");
		return GMI_FAIL;
	}
	
	return GMI_SUCCESS;
}

GMI_RESULT  GMI_NasParamConfig(NasParamConfigIn *NasParamConfigPtr)
{
	DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "NAS no support.\n");
	return GMI_NOT_SUPPORT;	
}

GMI_RESULT  GMI_RecordCtrl(RecordCtrlIn *RecordCtrlPtr)
{
	if(NULL == RecordCtrlPtr)
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return GMI_INVALID_PARAMETER;
	}

	if(LOCAL_RET_OK != RecordCtrlNotify(RecordCtrlPtr))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "SetRecConfigParam error.\n");
		return GMI_FAIL;
	}
	
	return GMI_SUCCESS;
}

GMI_RESULT  GMI_StorageVersionQuery(char_t *StorageVer, int32_t VerMaxLen)
{
	if((NULL == StorageVer) || (strlen(VERSION_STORAGE)+1 > VerMaxLen))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return GMI_INVALID_PARAMETER;
	}

	memcpy(StorageVer, VERSION_STORAGE, strlen(VERSION_STORAGE));

	return GMI_SUCCESS;
}

GMI_RESULT GMI_RecordFileQuery(RecordFileQueryIn *RecordFileQueryPtr, uint32_t *CurQueryPosNo, 
	                                 RecordFileQueryResOut **RecordFileQueryResPtr, uint32_t QueryResArraySize,
	                                 uint32_t  *QueryResTotalNum, uint32_t  *QueryResCurNum)
{
	if((NULL == RecordFileQueryPtr)
		|| (NULL == RecordFileQueryResPtr)
		|| (NULL == *RecordFileQueryResPtr)
		|| (MAX_NUM_QUERY_RECORD > QueryResArraySize)
		|| (NULL == QueryResTotalNum)
		|| (NULL == QueryResCurNum))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return GMI_INVALID_PARAMETER;
	}

	if(LOCAL_RET_OK != QueryRecordFile(RecordFileQueryPtr, CurQueryPosNo, 
	                                 RecordFileQueryResPtr, QueryResArraySize,
	                                 QueryResTotalNum, QueryResCurNum))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "QueryRecordFile error.\n");
		return GMI_FAIL;
	}

	return GMI_SUCCESS;
}


GMI_RESULT GMI_RecordDownReplayQuery(RecordDownReplayQueryIn *RecordDownReplayQueryPtr,
       RecordDownReplayQueryResOut **RecordDownReplayQueryResPtr, uint32_t QueryResArraySize)
{
	if((NULL == RecordDownReplayQueryPtr)
		|| (NULL == RecordDownReplayQueryResPtr)
		|| (NULL == *RecordDownReplayQueryResPtr))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return GMI_INVALID_PARAMETER;
	}

	if(LOCAL_RET_OK != QueryDownReplayRecordFile(RecordDownReplayQueryPtr, RecordDownReplayQueryResPtr, QueryResArraySize))
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "QueryDownReplayRecordFile error.\n");
		return GMI_FAIL;
	}

	return GMI_SUCCESS;
}


