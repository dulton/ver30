#ifndef __RTSP_STREAM_CONTROL_H__
#define __RTSP_STREAM_CONTROL_H__
#include "gmi_media_ctrl.h"
#include "rtsp_stream_ctrl.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"

#define RTSP_STREAM_STATE_START     (2)
#define RTSP_STREAM_STATE_STOP      (0)
#define RTSP_STREAM_STATE_PAUSE     (1)

class RtspStreamControl
{
public:
	RtspStreamControl();
	~RtspStreamControl();
	GMI_RESULT Initialize();
	GMI_RESULT Deinitialize();	
	GMI_RESULT Query(int32_t Timeout, int32_t VideoCount, int32_t *Started);
	GMI_RESULT Start(SysPkgEncodeCfg *SysEncodeCfg, int32_t VideoCount, SysPkgAudioEncodeCfg *SysAudioCfg, int32_t AudioCount, int32_t Timeout);
	GMI_RESULT Stop(int32_t Timeout);	
private:
	GMI_RESULT Lock();
	GMI_RESULT Unlock();
private:	
	GMI_Mutex m_Mutex;
	int32_t m_VideoCount;
};

#endif

