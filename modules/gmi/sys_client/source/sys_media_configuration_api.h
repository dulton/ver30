#ifndef __SYS_MEDIA_CONFIGURATION_API_H__
#define __SYS_MEDIA_CONFIGURATION_API_H__

#include "sys_env_types.h"
#include "gmi_system_headers.h"

#ifndef __cplusplus
extern "C"
{
#endif

    GMI_RESULT SysGetEncodeStreamNum(int32_t *StreamNum);
    GMI_RESULT SysGetEncodeCfg(SysPkgEncodeCfg *SysEncodeCfgPtr);
    GMI_RESULT SysSetEncodeCfg(int32_t StreamId, SysPkgEncodeCfg *SysEncodeCfgPtr);
    GMI_RESULT SysGetVideoSource(SysPkgVideoSource *SysVideoSource);
    GMI_RESULT SysGetImaging(SysPkgImaging *SysImagingPtr);
    GMI_RESULT SysSetImaging(int32_t SourceId, SysPkgImaging *SysImagingPtr);
    GMI_RESULT SysForceIdr(int32_t StreamId);

#ifndef __cplusplus
}
#endif


#endif
