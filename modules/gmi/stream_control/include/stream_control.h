

#ifndef  __STREAM_CONTROL_H__
#define  __STREAM_CONTROL_H__

#include <gmi_system_headers.h>
#include <sys_env_types.h>

#ifdef __cplusplus
extern "C" {
#endif

GMI_RESULT StopStreamTransfer(FD_HANDLE hd,int sdkport,int timeout);
GMI_RESULT StartStreamTransfer(FD_HANDLE hd,int sdkport,SysPkgEncodeCfg* pVideoCfg,int count,SysPkgAudioEncodeCfg *SysAudioCfgPtr, int AudioCount, int timeout);
GMI_RESULT PauseStreamTransfer(FD_HANDLE hd,int sdkport,int timeout);
GMI_RESULT ResumeStreamTransfer(FD_HANDLE hd,int sdkport,SysPkgEncodeCfg* pVideoCfg,int count,SysPkgAudioEncodeCfg *SysAudioCfgPtr, int AudioCount, int timeout);
GMI_RESULT QueryStreamTransfer(FD_HANDLE hd,int sdkport,int timeout,int *pstarted);




#ifdef __cplusplus
}
#endif

#endif /*__STREAM_CONTROL_H__*/



