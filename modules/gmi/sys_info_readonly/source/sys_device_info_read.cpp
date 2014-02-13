#include "sys_info_readonly.h"
#include "sys_utilitly.h"
#include "sys_device_info_read.h"
#include "gmi_system_headers.h"
#include "ipc_fw_v3.x_resource.h"
#include "ipc_fw_v3.x_setting.h"
#include "gmi_config_api.h"
#include "file_lock.h"


static SI_FileInfo l_CapAutoFileInfo = 
{
    CAPABILITY_AUTO_FILE_NAME,
    PTHREAD_MUTEX_INITIALIZER
};


static SI_FileInfo l_CapSwFileInfo = 
{
    CAPABILITY_SW_FILE_NAME,
    PTHREAD_MUTEX_INITIALIZER
};


static SI_FileInfo l_FactorySettingFileInfo = 
{
    GMI_FACTORY_SETTING_CONFIG_FILE_NAME, 
    PTHREAD_MUTEX_INITIALIZER
};


static SI_FileInfo l_SettingFileInfo = 
{
    GMI_SETTING_CONFIG_FILE_NAME, 
    PTHREAD_MUTEX_INITIALIZER
};


static SI_FileInfo l_ResourceFileInfo = 
{
    GMI_RESOURCE_CONFIG_FILE_NAME, 
    PTHREAD_MUTEX_INITIALIZER
};



GMI_RESULT SysInfoReadInitialize(void)
{
	GMI_RESULT  Result = l_CapAutoFileInfo.s_FileLock.Create(CAPABILITY_AUTO_FILE_NAME_KEY);
	if (FAILED(Result))
	{
		APP_ERROR("s_FileLock.Create(CAPABILITY_AUTO_FILE_NAME_KEY, 0) fail\n");
		return Result;
	}
	l_CapAutoFileInfo.s_Created = true;
	
	Result = l_CapSwFileInfo.s_FileLock.Create(CAPABILITY_SW_FILE_NAME_KEY);
	if (FAILED(Result))
	{
		APP_ERROR("s_FileLock.Create(CAPABILITY_SW_FILE_NAME_KEY, 0) fail\n");
		return Result;
	}
	l_CapSwFileInfo.s_Created = true;
	
	Result = l_FactorySettingFileInfo.s_FileLock.Create(GMI_FACTORY_SETTING_CONFIG_FILE_NAME_KEY);
	if (FAILED(Result))
	{
		APP_ERROR("s_FileLock.Create(GMI_FACTORY_SETTING_CONFIG_FILE_NAME_KEY, 0) fail\n");
		return Result;
	}
	l_FactorySettingFileInfo.s_Created = true;

	Result = l_SettingFileInfo.s_FileLock.Create(GMI_SETTING_CONFIG_FILE_NAME_KEY);
	if (FAILED(Result))
	{
		APP_ERROR("s_FileLock.Create(GMI_SETTING_CONFIG_FILE_NAME_KEY, 0) fail\n");
		return Result;
	}
	l_SettingFileInfo.s_Created = true;

	Result = l_ResourceFileInfo.s_FileLock.Create(GMI_RESOURCE_CONFIG_FILE_NAME_KEY);
	if (FAILED(Result))
	{
		APP_ERROR("s_FileLock.Create(GMI_RESOURCE_CONFIG_FILE_NAME_KEY, 0) fail\n");
		return Result;
	}
	l_ResourceFileInfo.s_Created = true;
	return GMI_SUCCESS;
}


GMI_RESULT SysInfoReadDeinitialize()
{
	return GMI_SUCCESS;
}


static GMI_RESULT FindFileInfo(const char_t* FileName, SI_FileInfo **FileInfo)
{
	if (0 == strcmp(FileName, l_CapAutoFileInfo.s_Name))
	{
		*FileInfo = &l_CapAutoFileInfo;
	}
	else if (0 == strcmp(FileName, l_CapSwFileInfo.s_Name))
	{
		*FileInfo = &l_CapSwFileInfo;
	}
	else if (0 == strcmp(FileName, l_FactorySettingFileInfo.s_Name))
	{
		*FileInfo = &l_FactorySettingFileInfo;
	}
	else if (0 == strcmp(FileName, l_SettingFileInfo.s_Name))
	{
		*FileInfo = &l_SettingFileInfo;
	}
	else if (0 == strcmp(FileName, l_ResourceFileInfo.s_Name))
	{
		*FileInfo = &l_ResourceFileInfo;
	}
	else 
	{
		APP_ERROR("FileName %s not support\n", FileName);
		return GMI_NOT_SUPPORT;
	}

	return GMI_SUCCESS;
}


GMI_RESULT SysInfoOpen(const char_t* FileName, FD_HANDLE *Handle)
{
	GMI_RESULT Result = GMI_SUCCESS;
	SysInfoReadHandle *SysInfoReadHandlePtr = NULL;

	SysInfoReadHandlePtr = (SysInfoReadHandle*)malloc(sizeof(SysInfoReadHandle));
	if (NULL == SysInfoReadHandlePtr)
	{
		APP_ERROR("malloc(sizeof(SysInfoReadHandle)) fail\n");
		return GMI_OUT_OF_MEMORY;
	}

	do 
	{			
		Result = FindFileInfo(FileName, &SysInfoReadHandlePtr->s_FileInfo);
		if (FAILED(Result))
		{
			APP_ERROR("FindFileInfo %s not find, Result = 0x%lx\n", FileName, Result);
			break;
		}

		if (!SysInfoReadHandlePtr->s_FileInfo->s_Created)
		{
			APP_ERROR("filelock not init\n");
			Result = GMI_FAIL;
			break;
		}

		LOCK(&SysInfoReadHandlePtr->s_FileInfo->s_Lock);
		SysInfoReadHandlePtr->s_FileInfo->s_FileLock.Lock();
		Result = GMI_XmlOpen((const char_t*)FileName, &SysInfoReadHandlePtr->s_XmlHandle);
		if (FAILED(Result))
		{	    
			SysInfoReadHandlePtr->s_FileInfo->s_FileLock.Unlock();
			UNLOCK(&SysInfoReadHandlePtr->s_FileInfo->s_Lock);
			APP_ERROR("GMI_XmlOpen %s fail, Result = 0x%lx\n", FileName, Result);
			break;
		}

		*Handle = (FD_HANDLE*)SysInfoReadHandlePtr;
		
		return GMI_SUCCESS;
	}
	while (0);

	if (NULL != SysInfoReadHandlePtr)
	{
		free(SysInfoReadHandlePtr);
	}

	return Result;
}


GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const char_t* Default , char_t *Context)
{
	GMI_RESULT Result = GMI_SUCCESS;
	SysInfoReadHandle *SysInfoReadHandlePtr = NULL;
	
	do
	{
		SysInfoReadHandlePtr = (SysInfoReadHandle*)Handle;		
		Result = GMI_XmlRead(SysInfoReadHandlePtr->s_XmlHandle, Path, KeyWord, Default, Context, GMI_CONFIG_READ_ONLY);
		if (FAILED(Result))
		{
			APP_ERROR("GMI_XmlRead [%s]:%s fail, Result = 0x%lx\n", Path, KeyWord, Result);
			break;
		}	
		return GMI_SUCCESS;
	}
	while (0);

	return Result;
}


GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const float_t Default , float_t *Context)
{
	GMI_RESULT Result = GMI_SUCCESS;
	SysInfoReadHandle *SysInfoReadHandlePtr = NULL;
	
	do
	{
		SysInfoReadHandlePtr = (SysInfoReadHandle*)Handle;		
		Result = GMI_XmlRead(SysInfoReadHandlePtr->s_XmlHandle, Path, KeyWord, Default, Context, GMI_CONFIG_READ_ONLY);
		if (FAILED(Result))
		{
			APP_ERROR("GMI_XmlRead [%s]:%s fail, Result = 0x%lx\n", Path, KeyWord, Result);
			break;
		}	
		return GMI_SUCCESS;
	}
	while (0);

	return Result;
}


GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord,const  int32_t Default , int32_t *Context)
{
	GMI_RESULT Result = GMI_SUCCESS;
	SysInfoReadHandle *SysInfoReadHandlePtr = NULL;
	
	do
	{
		SysInfoReadHandlePtr = (SysInfoReadHandle*)Handle;		
		Result = GMI_XmlRead(SysInfoReadHandlePtr->s_XmlHandle, Path, KeyWord, Default, Context, GMI_CONFIG_READ_ONLY);
		if (FAILED(Result))
		{
			APP_ERROR("GMI_XmlRead [%s]:%s fail, Result = 0x%lx\n", Path, KeyWord, Result);
			break;
		}	
		return GMI_SUCCESS;
	}
	while (0);

	return Result;
}


GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const uint32_t Default, uint32_t *Context)
{
	GMI_RESULT Result = GMI_SUCCESS;
	SysInfoReadHandle *SysInfoReadHandlePtr = NULL;
	
	do
	{
		SysInfoReadHandlePtr = (SysInfoReadHandle*)Handle;		
		Result = GMI_XmlRead(SysInfoReadHandlePtr->s_XmlHandle, Path, KeyWord, Default, (int32_t*)Context, GMI_CONFIG_READ_ONLY);
		if (FAILED(Result))
		{
			APP_ERROR("GMI_XmlRead [%s]:%s fail, Result = 0x%lx\n", Path, KeyWord, Result);
			break;
		}	
		return GMI_SUCCESS;
	}
	while (0);

	return Result;
}


GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const uint16_t Default , uint16_t *Context)
{
	GMI_RESULT Result = GMI_SUCCESS;
	SysInfoReadHandle *SysInfoReadHandlePtr = NULL;
	
	do
	{
		SysInfoReadHandlePtr = (SysInfoReadHandle*)Handle;		
		Result = GMI_XmlRead(SysInfoReadHandlePtr->s_XmlHandle, Path, KeyWord, Default, Context, GMI_CONFIG_READ_ONLY);
		if (FAILED(Result))
		{
			APP_ERROR("GMI_XmlRead [%s]:%s fail, Result = 0x%lx\n", Path, KeyWord, Result);
			break;
		}	
		return GMI_SUCCESS;
	}
	while (0);

	return Result;
}


GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const int16_t Default , int16_t *Context)
{
	GMI_RESULT Result = GMI_SUCCESS;
	SysInfoReadHandle *SysInfoReadHandlePtr = NULL;
	
	do
	{
		SysInfoReadHandlePtr = (SysInfoReadHandle*)Handle;		
		Result = GMI_XmlRead(SysInfoReadHandlePtr->s_XmlHandle, Path, KeyWord, Default, Context, GMI_CONFIG_READ_ONLY);
		if (FAILED(Result))
		{
			APP_ERROR("GMI_XmlRead [%s]:%s fail, Result = 0x%lx\n", Path, KeyWord, Result);
			break;
		}	
		return GMI_SUCCESS;
	}
	while (0);

	return Result;
}


GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const uint8_t Default , uint8_t *Context)
{
	GMI_RESULT Result = GMI_SUCCESS;
	SysInfoReadHandle *SysInfoReadHandlePtr = NULL;
	
	do
	{
		SysInfoReadHandlePtr = (SysInfoReadHandle*)Handle;		
		Result = GMI_XmlRead(SysInfoReadHandlePtr->s_XmlHandle, Path, KeyWord, Default, Context, GMI_CONFIG_READ_ONLY);
		if (FAILED(Result))
		{
			APP_ERROR("GMI_XmlRead [%s]:%s fail, Result = 0x%lx\n", Path, KeyWord, Result);
			break;
		}	
		return GMI_SUCCESS;
	}
	while (0);

	return Result;
}


GMI_RESULT SysInfoRead(FD_HANDLE Handle, const char_t *Path, const char_t *KeyWord, const int8_t Default , int8_t *Context)
{
	GMI_RESULT Result = GMI_SUCCESS;
	SysInfoReadHandle *SysInfoReadHandlePtr = NULL;
	
	do
	{
		SysInfoReadHandlePtr = (SysInfoReadHandle*)Handle;		
		Result = GMI_XmlRead(SysInfoReadHandlePtr->s_XmlHandle, Path, KeyWord, Default, Context, GMI_CONFIG_READ_ONLY);
		if (FAILED(Result))
		{
			APP_ERROR("GMI_XmlRead [%s]:%s fail, Result = 0x%lx\n", Path, KeyWord, Result);
			break;
		}	
		return GMI_SUCCESS;
	}
	while (0);

	return Result;
}


GMI_RESULT SysInfoClose(FD_HANDLE Handle)
{
	GMI_RESULT Result = GMI_SUCCESS;
	SysInfoReadHandle *SysInfoReadHandlePtr = NULL;

	do
	{
		SysInfoReadHandlePtr = (SysInfoReadHandle*)Handle;
		Result = GMI_XmlFileSave(SysInfoReadHandlePtr->s_XmlHandle);
    	if (FAILED(Result))
    	{
    		SysInfoReadHandlePtr->s_FileInfo->s_FileLock.Unlock();
    		UNLOCK(&SysInfoReadHandlePtr->s_FileInfo->s_Lock);
    		APP_ERROR("GMI_XmlFileSave %s fail, Result = 0x%lx\n", SysInfoReadHandlePtr->s_FileInfo->s_Name, Result);
    		break;
    	}

    	SysInfoReadHandlePtr->s_FileInfo->s_FileLock.Unlock();
    	UNLOCK(&SysInfoReadHandlePtr->s_FileInfo->s_Lock);
    	return GMI_SUCCESS;
	}
	while (0);

	return Result;
}


GMI_RESULT SysInfoReadCpuName(char_t Cpu[32])
{
    GMI_RESULT Result = GMI_SUCCESS;
    FD_HANDLE  Handle;
     
	if (NULL == Cpu)
	{
	   return GMI_INVALID_PARAMETER;
	}
		
    LOCK(&l_CapAutoFileInfo.s_Lock);	
	l_CapAutoFileInfo.s_FileLock.Lock();	
    Result = GMI_XmlOpen((const char_t*)l_CapAutoFileInfo.s_Name, &Handle);
	if (FAILED(Result))
	{	    
		l_CapAutoFileInfo.s_FileLock.Unlock();
	    UNLOCK(&l_CapAutoFileInfo.s_Lock);
		return Result;
	}
	Result = GMI_XmlRead(Handle, HW_AUTO_DETECT_INFO_PATH, HW_CPU_KEY, "NULL", Cpu,  GMI_CONFIG_READ_ONLY);	
	if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
		l_CapAutoFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_CapAutoFileInfo.s_Lock);
        return Result;
    }
    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
		l_CapAutoFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_CapAutoFileInfo.s_Lock);
        return Result;
    }	
	l_CapAutoFileInfo.s_FileLock.Unlock();	
    UNLOCK(&l_CapAutoFileInfo.s_Lock);
	
    return GMI_SUCCESS;
}


GMI_RESULT SysInfoReadSensorName(char_t Sensor[32])
{
	GMI_RESULT Result = GMI_SUCCESS;
    FD_HANDLE  Handle;
     
	if (NULL == Sensor)
	{
	   return GMI_INVALID_PARAMETER;
	}
	
    LOCK(&l_CapAutoFileInfo.s_Lock);
	l_CapAutoFileInfo.s_FileLock.Lock();
    Result = GMI_XmlOpen((const char_t*)l_CapAutoFileInfo.s_Name, &Handle);
	if (FAILED(Result))
	{	    
		l_CapAutoFileInfo.s_FileLock.Unlock();
	    UNLOCK(&l_CapAutoFileInfo.s_Lock);
		return Result;
	}
	Result = GMI_XmlRead(Handle, HW_AUTO_DETECT_INFO_PATH, HW_SENSOR_KEY, "NULL", Sensor,  GMI_CONFIG_READ_ONLY);
	if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
		l_CapAutoFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_CapAutoFileInfo.s_Lock);
        return Result;
    }
    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
		l_CapAutoFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_CapAutoFileInfo.s_Lock);
        return Result;
    }
	l_CapAutoFileInfo.s_FileLock.Unlock();
    UNLOCK(&l_CapAutoFileInfo.s_Lock);
	
    return GMI_SUCCESS;  
}


GMI_RESULT SysInfoReadIRcutName(char_t IRcut[32])
{
    GMI_RESULT Result = GMI_SUCCESS;
    FD_HANDLE  Handle;
     
	if (NULL == IRcut)
	{
	   return GMI_INVALID_PARAMETER;
	}
	
    LOCK(&l_FactorySettingFileInfo.s_Lock);
	l_FactorySettingFileInfo.s_FileLock.Lock();
    Result = GMI_XmlOpen((const char_t*)l_FactorySettingFileInfo.s_Name, &Handle);
	if (FAILED(Result))
	{	    
		l_FactorySettingFileInfo.s_FileLock.Unlock();
	    UNLOCK(&l_FactorySettingFileInfo.s_Lock);
		return Result;
	}
	Result = GMI_XmlRead(Handle, VIDEO_IRCUT_NAME_PATH, VIDEO_IRCUT_NAME_KEY, "NULL", IRcut,  GMI_CONFIG_READ_ONLY);
	if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
		l_FactorySettingFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_FactorySettingFileInfo.s_Lock);
        return Result;
    }
    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
		l_FactorySettingFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_FactorySettingFileInfo.s_Lock);
        return Result;
    }
	l_FactorySettingFileInfo.s_FileLock.Unlock();
    UNLOCK(&l_FactorySettingFileInfo.s_Lock);
	
	return GMI_SUCCESS;
}


GMI_RESULT SysInfoReadShieldName(char_t Shield[32])
{
	GMI_RESULT Result = GMI_SUCCESS;
    FD_HANDLE  Handle;
     
	if (NULL == Shield)
	{
	   return GMI_INVALID_PARAMETER;
	}
	
	
    LOCK(&l_FactorySettingFileInfo.s_Lock);
	l_FactorySettingFileInfo.s_FileLock.Lock();
    Result = GMI_XmlOpen((const char_t*)l_FactorySettingFileInfo.s_Name, &Handle);
	if (FAILED(Result))
	{	    
		l_FactorySettingFileInfo.s_FileLock.Unlock();
	    UNLOCK(&l_FactorySettingFileInfo.s_Lock);
		return Result;
	}
	Result = GMI_XmlRead(Handle, PTZ_SHIELD_NAME_PATH, PTZ_SHIELD_NAME_KEY, "NULL", Shield,  GMI_CONFIG_READ_ONLY);
	if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
		l_FactorySettingFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_FactorySettingFileInfo.s_Lock);
        return Result;
    }
    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
		l_FactorySettingFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_FactorySettingFileInfo.s_Lock);
        return Result;
    }
	l_FactorySettingFileInfo.s_FileLock.Unlock();
    UNLOCK(&l_FactorySettingFileInfo.s_Lock);
	
    return GMI_SUCCESS;
}


GMI_RESULT SysInfoReadMaxRes(int32_t *Width, int32_t *Height)
{
	GMI_RESULT Result = GMI_SUCCESS;
    FD_HANDLE  Handle;
     
	if (NULL == Width
		|| NULL == Height)
	{
	   return GMI_INVALID_PARAMETER;
	}
	
    LOCK(&l_CapSwFileInfo.s_Lock);
	l_CapSwFileInfo.s_FileLock.Lock();
    Result = GMI_XmlOpen((const char_t*)l_CapSwFileInfo.s_Name, &Handle);
	if (FAILED(Result))
	{	    
		l_CapSwFileInfo.s_FileLock.Unlock();
	    UNLOCK(&l_CapSwFileInfo.s_Lock);
		return Result;
	}
	Result = GMI_XmlRead(Handle, CAPABILITY_SW_MEDIA_PATH, MAX_PIC_WIDTH_KEY,  0,   Width,  GMI_CONFIG_READ_ONLY);
	Result = GMI_XmlRead(Handle, CAPABILITY_SW_MEDIA_PATH, MAX_PIC_HEIGHT_KEY, 0,   Height, GMI_CONFIG_READ_ONLY);
	if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
		l_CapSwFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_CapSwFileInfo.s_Lock);
        return Result;
    }
    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
		l_CapSwFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_CapSwFileInfo.s_Lock);
        return Result;
    }
	l_CapSwFileInfo.s_FileLock.Unlock();
    UNLOCK(&l_CapSwFileInfo.s_Lock);
	
    return GMI_SUCCESS;
}

GMI_RESULT SysInfoReadMaxStreamNum(int32_t *StreamNum)
{
    GMI_RESULT Result = GMI_SUCCESS;
    FD_HANDLE  Handle;
     
	if (NULL == StreamNum)
	{
	   return GMI_INVALID_PARAMETER;
	}
	
    LOCK(&l_CapSwFileInfo.s_Lock);
	l_CapSwFileInfo.s_FileLock.Lock();
    Result = GMI_XmlOpen((const char_t*)l_CapSwFileInfo.s_Name, &Handle);
	if (FAILED(Result))
	{	    
		l_CapSwFileInfo.s_FileLock.Unlock();
	    UNLOCK(&l_CapSwFileInfo.s_Lock);
		return Result;
	}
	Result = GMI_XmlRead(Handle, CAPABILITY_SW_MEDIA_PATH, MAX_STREAM_NUM_KEY,  0,  StreamNum,  GMI_CONFIG_READ_ONLY);	
	if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
		l_CapSwFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_CapSwFileInfo.s_Lock);
        return Result;
    }
    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
		l_CapSwFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_CapSwFileInfo.s_Lock);
        return Result;
    }
	l_CapSwFileInfo.s_FileLock.Unlock();
    UNLOCK(&l_CapSwFileInfo.s_Lock);
    return GMI_SUCCESS;
}


GMI_RESULT SysInfoReadSoftwareVer(char_t Ver[64])
{
   return GMI_NOT_SUPPORT;
}


GMI_RESULT SysInfoReadHardwareVer(char_t Ver[64])
{
    GMI_RESULT Result = GMI_SUCCESS;
    FD_HANDLE  Handle;
     
	if (NULL == Ver)
	{
	   return GMI_INVALID_PARAMETER;
	}
	
    LOCK(&l_FactorySettingFileInfo.s_Lock);
	l_FactorySettingFileInfo.s_FileLock.Lock();
    Result = GMI_XmlOpen((const char_t*)l_FactorySettingFileInfo.s_Name, &Handle);
	if (FAILED(Result))
	{	    
		l_FactorySettingFileInfo.s_FileLock.Unlock();
	    UNLOCK(&l_FactorySettingFileInfo.s_Lock);
		return Result;
	}
	Result = GMI_XmlRead(Handle, DEVICE_INFO_PATH, DEVICE_HWVER_KEY, "NULL", Ver,  GMI_CONFIG_READ_ONLY);
	if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
		l_FactorySettingFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_FactorySettingFileInfo.s_Lock);
        return Result;
    }
    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
		l_FactorySettingFileInfo.s_FileLock.Unlock();
		UNLOCK(&l_FactorySettingFileInfo.s_Lock);
        return Result;
    }
	l_FactorySettingFileInfo.s_FileLock.Unlock();
    UNLOCK(&l_FactorySettingFileInfo.s_Lock);
	
    return GMI_SUCCESS;
}


GMI_RESULT SysInfoReadAlarmState(boolean_t *Enabled)
{
	if (NULL == Enabled)
	{
		return GMI_INVALID_PARAMETER;
	}
	*Enabled = false;
    return GMI_SUCCESS;
}


GMI_RESULT SysInfoReadLanguage(char_t Language[32])
{
	if (NULL == Language)
	{
		return GMI_INVALID_PARAMETER;
	}
	strcpy(Language, "chinese");
    return GMI_SUCCESS;
}


GMI_RESULT SysInfoReadGB28181State(boolean_t *Enabled)
{
	if (NULL == Enabled)
	{
		return GMI_INVALID_PARAMETER;
	}
	*Enabled = false;
    return GMI_SUCCESS;
}


