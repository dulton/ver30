#ifndef PSMUXLIB_INTERFACE_H
#define PSMUXLIB_INTERFACE_H


#define MPEG2MUX_VIDEO_STREAM            0x01
#define MPEG2MUX_AUDIO_STREAM            0x02
#define MPEG2MUX_AUDIO_VIDEO_STREAM      0x03
 
//������
#define STREAM_TYPE_MPEG4  0x10
//#define STREAM_TYPE_H264  0x1B
#define STREAM_TYPE_H264  0xE0
#define STREAM_TYPE_SVAC_VIDEO  0x80
#define STREAM_TYPE_G711  0x90
#define STREAM_TYPE_G722_1  0x92
#define STREAM_TYPE_G723_1  0X93
#define STREAM_TYPE_G729  0x99
#define STREAM_TYPE_SVAC_AUDIO 0X9B

 
//������������Ϣ
//��������������Ϣ��������Ƶ����Ƶ������Ƶ����Ƶ��Ϣ��s_ESInBuffer��ΪNULL
typedef struct
{
	int  bVideo;
	unsigned char *s_ESInBuffer;//����������bufferָ��
	int s_ESInLen;//����Ļ�������С
	long long s_Pts; //��Ƶ��pts(100ns)
}PSmux_input_info;
 
//ps��Ͼ���ṹ��
typedef void* PSmux_handle;

//��Ŀ����������Ϣ
typedef struct
{
	unsigned char *s_PSOutBuffer; //��Ŀ�����bufferָ��
	int s_PSOutBufferSize; //��Ŀ�����buffer ��С
	int s_PSOutLen; //����Ľ�Ŀ����С
}PSmux_out_info;
 
typedef struct
{
	int StreamMode;
	int s_VideoStreamType; //������
	int s_AudioStreamType; //������
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
*name��PsMuxInit
*func:PSMux��ʼ������þ����
*param: PSMuxHandle -- PSMux���ָ�룬
 StreamMode-- ������ģʽ��ȡֵ��MPEG2MUX_VIDEO_STREAM��MEPG2MUX_AUDIO_STREAM��MEPG2MUX_AUDIO_VIDEO_STREAM
*return:�ɹ�����0��ʧ�ܷ���-1
===================*/
int PSMuxInit(PSmux_handle *PSMuxHandle, PSmux_init_param *InitParam);
 
/*******************
*name��PSMuxProcess
*func:���������Ϊ��Ŀ������
*param: PSMuxHandle -- PSMux���ָ�룬
            ESInInfo -- ������������Ϣ
            PSMuxOutInfo -- ��ϵĽ�Ŀ�������Ϣ
*return:�ɹ�����0��ʧ�ܷ���-1
===================*/
int PSMuxProcess(PSmux_handle PSMuxHandle,  PSmux_input_info *ESInInfo,  PSmux_out_info *PSMuxOutInfo, int bNewESInfo);
 
/*******************
*name��PSMuxRelease
*func:��Դ�ͷţ�
*param: PSMuxHandle -- PSMux���ָ�룬
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

