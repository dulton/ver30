
#ifndef  __SDK_CLIENT_BUFFER_H__
#define  __SDK_CLIENT_BUFFER_H__

#include <sdk_server_session.h>
#include <sys_stream_info.h>

typedef enum
{
    cb_end_state = 0,
    cb_i_frame_state = 1,
    cb_seq_state  = 2,
    cb_block_state = 3,
    cb_pending_state = 4,
} enum_cb_state_t;


typedef enum
{
	sb_end_state = 0,
	sb_i_frame_state = 1,
	sb_seq_state = 3,
	sb_pending_state = 4,
}enum_sb_state_t;



typedef struct
{
	uint32_t m_VideoCode;
    uint8_t m_FrameType;
    uint8_t m_StreamId;
    uint8_t m_Freq;
    uint8_t m_EncType;
    uint8_t m_FOP;
    uint8_t m_Reserved[3];
    uint64_t m_Pts;
    uint32_t m_FrameIdx;
    uint16_t m_Width;
    uint16_t m_Height;
} __attribute__ ((packed)) video_i_frame_header_t ; 


typedef struct
{
	uint32_t m_VideoCode;
    uint8_t m_FrameType;
    uint8_t m_StreamId;
    uint8_t m_Reserved[2];
    uint32_t m_FrameIdx;
    uint64_t m_Pts;
} __attribute__ ((packed)) video_p_frame_header_t ;

typedef struct
{
	uint32_t m_AudioCode;
	uint8_t m_FrameType;
	uint8_t m_EncodeType;
	uint8_t m_Chan;
	uint8_t m_BitsPerSample;
	uint32_t m_SamplePerSec;
	uint32_t m_FrameId;
	uint64_t m_Pts;
	uint32_t m_AvgBytesPerSec;
} __attribute__((packed)) audio_frame_header_t;

typedef struct
{
	void* m_pData;
	int m_DataLen;
} cb_block_t;

typedef struct
{
    int m_Sock;
	sessionid_t m_SesId;
    enum_cb_state_t m_State;
	int m_Failed;
	unsigned int m_CurIdx;
	uint32_t m_SourceIdx;
	
	int m_WriteLen;
	int m_TotalLen;
	gssp_header_t m_GsspHeader;
	int m_GsspLen;
	
    union
    {
        video_i_frame_header_t m_IFrame;
        video_p_frame_header_t m_PFrame;
		audio_frame_header_t m_AFrame;
    } m_Header;
	int m_FrameLen;
	uint32_t m_Offset;
	stream_pack_t* m_pBlock;
} client_buffer_t;

#define  STREAM_DATA_SEQNUM  0
#define   MAX_STREAM_IDX_32      0xffffffff


#endif  /*__SDK_CLIENT_BUFFER_H__*/


