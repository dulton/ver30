#include "stream_process.h"
#include <ipc_media_data_client.h>
#include "gmi_media_ctrl.h"
#include "storage_manager.h"

int test()
{
	IPC_MediaDataClient DataClient;

	DataClient.Initialize(40127, 3);
	DataClient.Register(10000,MEDIA_VIDEO_H264,0,true,NULL,NULL);
	VidAudDataToBuf(0,NULL, 0, 0);
	return 0;
}
