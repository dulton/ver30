#include "stream_process.h"
#include <ipc_fw_v3.x_resource.h>
#include <ipc_media_data_client.h>
#include <ipc_media_data_dispatch.h>

#include "gmi_media_ctrl.h"
#include "storage_manager.h"

extern int32_t l_IsStartDataRecTask[4];
int32_t GetClientStartPort()
{
	return GMI_STREAMING_MEDIA_GB28181_ENCODE_VIDEO1;
}

int32_t GetServerPort()
{
	return GMI_STREAMING_MEDIA_SERVER_ENCODE_VIDEO1;
}

void *RecordDataReceiveTask(void *InParam)
{
	IPC_MediaDataClient DataClient;
	int32_t ClientPort;
	int32_t ServerPort;
	int32_t StreamId;
	GMI_RESULT RetVal;
	uint32_t BufSize = 1<<20;
	char *PBuf = NULL;
	struct timeval TmVal;
	int32_t BufLen = 0;
	int32_t ExtInfoSize = 100;
	int32_t ExtDataLen = 0;
	ExtMediaEncInfo *PExtInfo = NULL;
	char_t *PExtData = NULL;
	int32_t FrameType;

	if(NULL == (PBuf = (char_t *)malloc(BufSize)))
	{
		goto ErrExit;
	}

	if(NULL == (PExtData = (char_t *)malloc(ExtInfoSize)))
	{
		goto ErrExit;
	}
	
	ClientPort = GetClientStartPort();
	ServerPort = GetServerPort();
	StreamId = *(int32_t*)InParam;
	if((StreamId < 0) || (StreamId > 1))
	{
		StreamId = 0;
	}

	RetVal = DataClient.Initialize(ClientPort, GB_28181_STREAM_APPLICATION_ID);
	if(RetVal != GMI_SUCCESS)
    {
        goto ErrExit;
    }
	RetVal = DataClient.Register(ServerPort,MEDIA_VIDEO_H264,StreamId,true,NULL,NULL);
	if(RetVal != GMI_SUCCESS)
    {
    	DataClient.Deinitialize();
        goto ErrExit;
    }

	while(1 == l_IsStartDataRecTask[StreamId])
	{
		BufLen = BufSize;
		ExtDataLen = ExtInfoSize;
		RetVal = DataClient.Read(PBuf, (size_t*)&BufLen, &TmVal, PExtData, (size_t*)&ExtDataLen);
		if(GMI_SUCCESS == RetVal)
		{
			PExtInfo = (ExtMediaEncInfo *)PExtData;
			 FrameType = FRAME_TYPE_P;
			 if((VIDEO_I_FRAME == PExtInfo->s_FrameType)
			 	|| (VIDEO_P_FRAME == PExtInfo->s_FrameType))
			 {
			 	FrameType = FRAME_TYPE_I;
			 }
			 VidAudDataToBuf(0,PBuf, BufLen, FrameType);
		}
	}
	DataClient.Unregister();
	DataClient.Deinitialize();
ErrExit:
	l_IsStartDataRecTask[StreamId] = 0;
}
