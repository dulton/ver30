#include "sys_info_readonly.h"
#include "ipc_fw_v3.x_setting.h"
#include "ipc_fw_v3.x_resource.h"

static unsigned int DmGetTimeStamp()
{
    unsigned int stamp;
    struct timeval now;

    gettimeofday(&now, NULL);

    stamp = now.tv_sec * 1000 + now.tv_usec/1000;

    return stamp;
}

int main()
{	
	//init
	GMI_RESULT Result = SysInfoReadInitialize();
	if (FAILED(Result))
	{
		return Result;
	}
	
	int Time = DmGetTimeStamp();
	//get sdk port
	uint16_t SdkPort;
	FD_HANDLE Handle;
    Result = SysInfoOpen(GMI_SETTING_CONFIG_FILE_NAME, &Handle);
	if (FAILED(Result))
	{
		return Result;
	}
	
	Result = SysInfoRead(Handle, GMI_EXTERN_NETWORK_PORT_PATH, GMI_SDK_SERVER_PORT_KEY, GMI_SDK_SERVER_PORT, &SdkPort);
	if (FAILED(Result))
	{
		SysInfoClose(Handle);
		return Result;
	}
	
	SysInfoClose(Handle);
	printf("timediff %d\n", DmGetTimeStamp()-Time);
	SysInfoReadDeinitialize();
	
	printf("sdk port %d\n", SdkPort);
	return GMI_SUCCESS;
}