#include "stream_process.h"
#include <ipc_fw_v3.x_resource.h>
#include <ipc_media_data_client.h>
#include <ipc_media_data_dispatch.h>
#include "PSMuxlib.h"
#include "Putbits.h"

#include "gmi_media_ctrl.h"
#include "storage_manager.h"

extern int32_t l_IsStartDataRecTask[4];
extern int32_t l_StreamNum;
extern RecordCtrlIn g_RecDataInfo;

#define  VFREQ_PER_SECOND(sec)  ( (unsigned long long)(((unsigned long long)sec)*10000000))
#define  VFREQ_PER_USECOND(usec)  ((unsigned long long)(((unsigned long long)usec)*10))


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
	uint8_t *PBuf = NULL;
	uint8_t *PResBuf = NULL;
	struct timeval TmVal;
	int32_t BufLen = 0;
	int32_t ExtInfoSize = 100;
	int32_t ExtDataLen = 0;
	ExtMediaEncInfo *PExtInfo = NULL;
	char_t *PExtData = NULL;
	int32_t FrameType;
	#if 1
	PSmux_handle PSMuxHandle = NULL;
	PSmux_init_param InitParam;
	PSmux_input_info ESInInfo;
	PSmux_out_info PSMuxOutInfo;
	int32_t BNewESInfo;
	#endif
	printf("malloc...\n");
	
	pthread_detach(pthread_self()); 

	if(NULL == (PBuf = (uint8_t *)malloc(BufSize)))
	{
		goto ErrExit;
	}

	if(NULL == (PResBuf = (uint8_t *)malloc(BufSize)))
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
	printf("StreamId=%d\n", StreamId);
	if((StreamId < 0) || (StreamId > 1))
	{
		StreamId = 0;
	}

	RetVal = DataClient.Initialize(ClientPort, GB_28181_STREAM_APPLICATION_ID);
	if(RetVal != GMI_SUCCESS)
    {
    	printf("Initialize error[0x%x]\n", RetVal);
        goto ErrExit;
    }
	RetVal = DataClient.Register(ServerPort,MEDIA_VIDEO_H264,StreamId,true,NULL,NULL);
	if(RetVal != GMI_SUCCESS)
    {
    	
    	printf("Register error[0x%x]\n", RetVal);
    	DataClient.Deinitialize();
        goto ErrExit;
    }
	#if 1
	InitParam.StreamMode = MPEG2MUX_VIDEO_STREAM;
	InitParam.s_VideoStreamType = STREAM_TYPE_H264;
	InitParam.s_AudioStreamType = 0;
	InitParam.s_MaxPacketlength = 5000;
	InitParam.MuxRate = 0;
	//InitParam.ES_Init_param.Video_init_param.FrameRate = 25; 
	PSMuxInit(&PSMuxHandle, &InitParam);
	#endif

	printf("RecordDataReceiveTask start...\n");
	
	while(1 == l_IsStartDataRecTask[StreamId])
	{
		BufLen = BufSize;
		ExtDataLen = ExtInfoSize;
		printf("start read data ...\n");
		RetVal = DataClient.Read(PBuf, (size_t*)&BufLen, &TmVal, PExtData, (size_t*)&ExtDataLen);
		printf("start read data OK...\n");
		if(GMI_SUCCESS == RetVal)
		{
			PExtInfo = (ExtMediaEncInfo *)PExtData;
			 FrameType = FRAME_TYPE_P;
			 if((VIDEO_I_FRAME == PExtInfo->s_FrameType)
			 	|| (VIDEO_P_FRAME == PExtInfo->s_FrameType))
			 {
			 	FrameType = FRAME_TYPE_I;
			 }
			 ESInInfo.bVideo = 1;
			 ESInInfo.s_Pts = VFREQ_PER_SECOND( TmVal.tv_sec) + VFREQ_PER_USECOND(TmVal.tv_usec );//
			 BNewESInfo = 1;
			 ESInInfo.s_ESInBuffer = PBuf;
			 ESInInfo.s_ESInLen = BufLen;
			 PSMuxOutInfo.s_PSOutBuffer = PResBuf;
			 PSMuxOutInfo.s_PSOutBufferSize = BufSize;
			 PSMuxOutInfo.s_PSOutLen = 0;
			 
			 PSMuxProcess(PSMuxHandle,  &ESInInfo,  &PSMuxOutInfo, BNewESInfo);
			 printf("BufLen=%d, PSMuxOutInfo.s_PSOutLen=%d\n", BufLen, PSMuxOutInfo.s_PSOutLen);
			 if(PSMuxOutInfo.s_PSOutLen > 0)
			 {
			 	if(1 != l_IsStartDataRecTask[StreamId])
			 	{
			 		break;
			 	}
			 	VidAudDataToBuf(0,(char_t*)(PSMuxOutInfo.s_PSOutBuffer), PSMuxOutInfo.s_PSOutLen, FrameType);
			 }
		}
	}
	DataClient.Unregister();
	DataClient.Deinitialize();
	if(NULL != PSMuxHandle)
	{
		PSMuxRelease(PSMuxHandle);
		PSMuxHandle = NULL;
	}
ErrExit:
	
	printf("RecordDataReceiveTask stop00...\n");
	pthread_exit(NULL);
	printf("RecordDataReceiveTask stop...\n");
	l_IsStartDataRecTask[StreamId] = 0;
}
