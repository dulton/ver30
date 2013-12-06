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

