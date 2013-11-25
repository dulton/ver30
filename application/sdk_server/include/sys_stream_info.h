
#ifndef __SYS_STREAM_INFO_H__
#define __SYS_STREAM_INFO_H__

#include <osal/gmi_system_headers.h>
#include <sys_env_types.h>
#include <media/gmi_media_ctrl.h>

#define  MAX_STREAM_IDS     4
#define  AUDIO_STREAM_ID    MAX_STREAM_IDS     /*this is the streamid for the audio ,it will bigest for every streamid*/
#define  AUDIO_DECODE_ID    (MAX_STREAM_IDS + 1)
typedef unsigned long ptr_t;
#define MAX_REQNUM  0xffff

#define  AUDIO_DUAL_FILE_EMULATE      1
#undef AUDIO_DUAL_FILE_EMULATE

#define  NOT_MEDIA_DECODE_STREAM_PUT  1
#undef NOT_MEDIA_DECODE_STREAM_PUT

#define  VIDEO_STREAM_EMULATE         1
#undef   VIDEO_STREAM_EMULATE


typedef struct
{
    uint16_t m_Count;
    uint16_t m_AudioCount;
    SysPkgEncodeCfg m_VideoInfo[MAX_STREAM_IDS];
    SysPkgAudioEncodeCfg m_AudioInfo[2];
} sys_stream_info_t;

typedef struct
{
    uint32_t m_Result;
    uint8_t m_StreamId;
    uint8_t m_Rate;
    uint8_t m_FOP;
    uint8_t m_EncType;
    uint16_t m_Width;
    uint16_t m_Height;
} sdk_video_info_t;

typedef struct
{
    uint8_t m_EncodeType;
    uint8_t m_Reserved;
    uint8_t m_Chan;
    uint8_t m_BitPerSample;
    uint32_t m_SamplePerSec;
} sdk_audio_info_t;

#define  I_FRAME_TYPE   1
#define  P_FRAME_TYPE  0


#define  STREAM_START_OPCODE_REQ    1701
#define  STREAM_START_OPCODE_RESP   1702
#define  STREAM_STOP_OPCODE_REQ     1703
#define  STREAM_STOP_OPCODE_RESP    1704
#define  STREAM_PAUSE_OPCODE_REQ    1705
#define  STREAM_PAUSE_OPCODE_RESP   1706
#define  STREAM_RESUME_OPCODE_REQ   1707
#define  STREAM_RESUME_OPCODE_RESP  1708
#define  STREAM_QUERY_OPCODE_REQ    1709
#define  STREAM_QUERY_OPCODE_RESP   1710



#define SYSCODE_START_AUDIO_DECODE_REQ    2002 
#define SYSCODE_START_AUDIO_DECODE_RSP    2003
#define SYSCODE_STOP_AUDIO_DECODE_REQ     2004
#define SYSCODE_STOP_AUDIO_DECODE_RSP     2005



#define  STREAM_STATE_RUNNING       2
#define  STREAM_STATE_PAUSE         1
#define  STREAM_STATE_STOPPED       0

#define  AUDIO_DUAL_RESPONSE_SUCC                      0x0
#define  AUDIO_DUAL_RESPONSE_ERROR                     0x1001
#define  AUDIO_DUAL_OPEN_AUDIO_STREAM_START_FAILED     0x1002
#define  AUDIO_DUAL_OPEN_GET_STREAM_INFO_FAILED        0x1003
#define  AUDIO_DUAL_OPEN_PREPARE_STREAM_FAILED         0x1004

typedef struct
{
    uint32_t m_OpCode;
    sys_stream_info_t m_StreamInfos;
} sys_stream_request_t;

typedef struct
{
    uint32_t m_OpCode;
    uint32_t m_Result;
} sys_stream_response_t;


typedef struct _start_talk
{
    uint8_t  m_ucTranType;			// for trans type UDP/TCP
    uint8_t  m_ucReserverd[3];		// m_ucReserved[0] != 0 indicate for broadcast
    int8_t   m_szDstIpAddr[32];		// camera send ip address
    uint32_t m_uDstPort;				// camera send port
    //sdk send audio parameters
    uint8_t  m_ucEncodeType;			// 1 for g711
    uint8_t  m_ucChan;				//channel
    uint8_t  m_uBitsPerSample;		// bit per sample
    uint8_t  m_ucReserved;			//
    uint32_t m_uSamplesPerSec;		//sample for second
    uint32_t m_uAvgBytesPerSec;		//averate per second
    uint16_t  s_FrameRate;
    uint16_t  s_BitRate; //unit:kbps
    uint16_t  s_Volume; //10%, 20%, 30% ...100%
    uint16_t  s_AecFlag; //0-disable, 1-enable, Acoustic Echo Cancellation
    int16_t   s_AecDelayTime; //unit:ms
    uint16_t  s_Reserv2;
} start_talk_t;

typedef struct _start_talk_resp
{
	int8_t m_szDestIpAddr[32];          // destination address
	uint32_t m_DestPort;                // destination port
	uint8_t m_EncodeType;               // encode type 1 for g711
	uint8_t m_Channel;                  // channel ,default is 1
	uint8_t m_BitPerSample;             // bit per sample default is 8
	uint8_t m_Reserv1;
	uint32_t m_SamplePerSec;            // sample per second default is 8000
	uint32_t m_AvgBytesPerSec;          //  average bytes per second, it is channel * bitspersample * samplepersec / 8
	uint16_t s_FrameRate;               // frame rate
	uint16_t s_BitRate;                 // kps
	uint16_t s_Volume;                  // volume setting
	uint16_t s_AecFlag;                 // acoustic echo cancellation 0 for disable 1 for enable
	int16_t  s_AecDelayTime;            // delay time ,for ms
	uint16_t s_Reserv2;
}__attribute__((packed)) start_talk_resp_t;

typedef struct _start_talk_request
{
    uint32_t m_OpCode;
    start_talk_t m_Talk;
} start_talk_request_t;


typedef struct _start_talk_response
{
	uint32_t m_OpCode;
	uint32_t m_Result;
	start_talk_resp_t m_Resp;
} __attribute__((packed)) start_talk_response_t;


typedef struct
{
    uint32_t m_Idx;
    uint32_t m_Type;
    void*    m_pData;
    uint32_t m_DataLen;
    uint64_t m_Pts;
} stream_pack_t;


#define  GMI_RESOURCE_XML "/opt/config/gmi_resource.xml"
#define  GMI_SYS_SDK_PORT_PATH "/ipc/connect_port_resource"
#define  GMI_SYS_SERVER_TO_SDK_PORT_ITEM  "sys_server_to_sdk_port"
#define  GMI_SDK_TO_SYS_SERVER_PORT_ITEM  "sdk_to_sys_server_port"


#define SYS_SERVER_TO_SDK_PORT          57000
#define SDK_TO_SYS_SERVER_PORT          57002

#define   SDK_STREAM_OV_REQUEST  0x1
#define   SDK_STREAM_OV_RESPONSE 0x2
#define   SDK_STREAM_OA_REQUEST  0x3
#define   SDK_STREAM_OA_RESPONSE 0x4
#define   SDK_STREAM_VD_SEND     0x5
#define   SDK_STREAM_AD_SEND     0x6
#define   SDK_STREAM_AC_REQUEST  0x9
#define   SDK_STREAM_AC_RESPONSE 0xa





#endif /*__SYS_STREAM_INFO_H__*/

