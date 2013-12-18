#ifndef PSMUXLIB_INTERFACE_H
#define PSMUXLIB_INTERFACE_H


#define MPEG2MUX_VIDEO_STREAM            0x01
#define MPEG2MUX_AUDIO_STREAM            0x02
#define MPEG2MUX_AUDIO_VIDEO_STREAM      0x03
 
//流类型
#define STREAM_TYPE_MPEG4  0x10
//#define STREAM_TYPE_H264  0x1B
#define STREAM_TYPE_H264  0xE0
#define STREAM_TYPE_SVAC_VIDEO  0x80
#define STREAM_TYPE_G711  0x90
#define STREAM_TYPE_G722_1  0x92
#define STREAM_TYPE_G723_1  0X93
#define STREAM_TYPE_G729  0x99
#define STREAM_TYPE_SVAC_AUDIO 0X9B

 
//基本流输入信息
//若基本流输入信息不包含音频或视频，将音频或视频信息的s_ESInBuffer置为NULL
typedef struct
{
	int  bVideo;
	unsigned char *s_ESInBuffer;//基本流输入buffer指针
	int s_ESInLen;//输入的基本流大小
	long long s_Pts; //视频流pts(100ns)
}PSmux_input_info;
 
//ps混合句柄结构体
typedef void* PSmux_handle;

//节目混合流输出信息
typedef struct
{
	unsigned char *s_PSOutBuffer; //节目流输出buffer指针
	int s_PSOutBufferSize; //节目流输出buffer 大小
	int s_PSOutLen; //输出的节目流大小
}PSmux_out_info;
 
typedef struct
{
	int StreamMode;
	int s_VideoStreamType; //流类型
	int s_AudioStreamType; //流类型
	int s_MaxPacketlength;
	int MuxRate;
	union
	{
		struct
		{
			float FrameRate;
		}Video_init_param;
	}ES_Init_param;
}PSmux_init_param;
//return value
#define PS_ERROR_NONE      0
#define PS_ERROR_MEM       -1
#define PS_ERROR_PARAM     -2
#define PS_ERROR_NOT_ENOUGH_MEM -3
#define PS_ERROR_EOF_STREAM    -4

#ifdef __cplusplus
extern "C" {
#endif

/*******************
*name：PsMuxInit
*func:PSMux初始化，获得句柄；
*param: PSMuxHandle -- PSMux句柄指针，
 StreamMode-- 输入流模式，取值：MPEG2MUX_VIDEO_STREAM、MEPG2MUX_AUDIO_STREAM、MEPG2MUX_AUDIO_VIDEO_STREAM
*return:成功返回0，失败返回-1
===================*/
int PSMuxInit(PSmux_handle *PSMuxHandle, PSmux_init_param *InitParam);
 
/*******************
*name：PSMuxProcess
*func:基本流混合为节目流处理；
*param: PSMuxHandle -- PSMux句柄指针，
            ESInInfo -- 基本流输入信息
            PSMuxOutInfo -- 混合的节目流输出信息
*return:成功返回0，失败返回-1
===================*/
int PSMuxProcess(PSmux_handle PSMuxHandle,  PSmux_input_info *ESInInfo,  PSmux_out_info *PSMuxOutInfo, int bNewESInfo);
 
/*******************
*name：PSMuxRelease
*func:资源释放；
*param: PSMuxHandle -- PSMux句柄指针，
===================*/
void PSMuxRelease(PSmux_handle PSMuxHandle );

int InitPSOutput(PSmux_out_info* pOutput);
int ResetPSOutputBuffer(PSmux_out_info* pOutput,int size,int bVideo);
void FreePSOutput(PSmux_out_info* pOutput);

void* GetOutputPtr(PSmux_out_info* pOutput);
int GetOutputLength(PSmux_out_info* pOutput);


#ifdef __cplusplus
}
#endif


#endif

