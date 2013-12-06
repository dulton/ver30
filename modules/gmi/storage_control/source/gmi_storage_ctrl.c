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
				RetVal = GMI_NOT_SUPPORT;
			}
			break;
	}

	return RetVal;
}

GMI_RESULT GMI_StorageDeviceInit(StorageInitIn *StorageInitParamPtr)
{
	if(NULL == StorageInitParamPtr)
	{
		DEBUG_LOG(&LogClientHd, e_DebugLogLevel_Exception, "InParam NULL.\n");
		return GMI_INVALID_PARAMETER;
	}

	
	
	return GMI_SUCCESS;
}



