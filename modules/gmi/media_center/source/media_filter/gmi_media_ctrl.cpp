#include "gmi_media_ctrl.h"

#if !defined( __linux__ )

FD_HANDLE  GMI_VinVoutCreate(SensorId  SId,  uint16_t  ChanId)
{
    return (FD_HANDLE) 1;
}

void_t     GMI_VinVoutDestroy( FD_HANDLE VinVoutHd )
{
    return;
}

GMI_RESULT GMI_VinVoutGetConfig( FD_HANDLE VinVoutHd, VideoInParam *VinParam, VideoOutParam *VoutParam)
{
    return GMI_SUCCESS;
}

GMI_RESULT GMI_VinVoutSetConfig( FD_HANDLE VinVoutHd, VideoInParam *VinParam, VideoOutParam *VoutParam)
{
    return GMI_SUCCESS;
}

FD_HANDLE  GMI_VideoEncCreate( uint16_t VideoPort, MediaId MId, VideoEncodeParam *VidEncParamPtr)
{
    return (FD_HANDLE) 1;
}

void_t     GMI_VideoEncDestroy( FD_HANDLE VidEncHdPtr )
{
    return;
}

GMI_RESULT GMI_VideoEncStart( FD_HANDLE VidEncHdPtr )
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_VideoEncStop( FD_HANDLE VidEncHdPtr )
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_VideoEncGetConfig( FD_HANDLE VideoEncHd, VideoEncodeParam* VidEncParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_VideoEncSetConfig( FD_HANDLE VideoEncHd, VideoEncodeParam* VidEncParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_VideoOsdGetConfig( FD_HANDLE VideoEncHd, VideoOSDParam* VidOsdParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_VideoOsdSetConfig( FD_HANDLE VideoEncHd, VideoOSDParam* VidOsdParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_VideoEncSetCB(FD_HANDLE VidEncHdPtr, MediaEncCallBack VideoEncCB, void * UserDataPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ForceSetIdrFrame(FD_HANDLE VideoEncHd)
{
    return GMI_SUCCESS;
}

FD_HANDLE   GMI_ImageOperateCreate(SensorId  SId,  uint16_t  ChanId)
{
    return (FD_HANDLE) 1;
}

void_t      GMI_ImageOperateDestroy( FD_HANDLE ImageOptHd )
{
    return;
}

GMI_RESULT  GMI_ImageGetBaseConfig( FD_HANDLE VideoEncHd, ImageBaseParam* ImageBaseParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ImageSetBaseConfig( FD_HANDLE VideoEncHd, ImageBaseParam* ImageBaseParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ImageGetAdvanceConfig( FD_HANDLE ImageOptHd , ImageAdvanceParam* ImageAdvanceParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ImageSetAdvanceConfig( FD_HANDLE ImageOptHd , ImageAdvanceParam* ImageAdvanceParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ImageGetAfConfig( FD_HANDLE ImageOptHd , ImageAfParam* ImageAfParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ImageSetAfConfig( FD_HANDLE ImageOptHd , ImageAfParam* ImageAfParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ImageGetDnConfig( FD_HANDLE ImageOptHd , ImageDnParam* ImageDnParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ImageSetDnConfig( FD_HANDLE ImageOptHd , ImageDnParam* ImageDnParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ImageGetWbConfig( FD_HANDLE ImageOptHd , ImageWbParam* ImageWbParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ImageSetWbConfig( FD_HANDLE ImageOptHd , ImageWbParam* ImageWbParamPtr)
{
    return GMI_SUCCESS;
}

FD_HANDLE   GMI_AutoFocusCreate(SensorId  SId)
{
    return (FD_HANDLE) 1;
}

void_t      GMI_AutoFocusDestroy(FD_HANDLE  AutoFocusHd)
{
    return;
}

GMI_RESULT  GMI_AutoFocusStart(FD_HANDLE  AutoFocusHd)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AutoFocusStop(FD_HANDLE  AutoFocusHd)
{
    return GMI_SUCCESS;
}

GMI_RESULT	GMI_AutoFocusPause(FD_HANDLE  AutoFocusHd, int8_t ControlStatus)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AutoFocusGlobalScan(FD_HANDLE AutoFocusHd)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AutoFocusSetMode(FD_HANDLE  AutoFocusHd, int32_t AFMode)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AFEventNotify(FD_HANDLE	AutoFocusHd, int32_t EventType, uint8_t *ExtData, uint32_t Length)
{
    return GMI_SUCCESS;
}

GMI_RESULT	GMI_FocusPositionGet (FD_HANDLE  AutoFocusHd, int32_t *CurFocusPos)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_FocusPositionSet (FD_HANDLE  AutoFocusHd, int32_t  FocusPos)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_FocusRangeGet (FD_HANDLE  AutoFocusHd, int32_t *MinFocusPos, int32_t *MaxFocusPos)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_FocusMotorReset(FD_HANDLE  AutoFocusHd)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AfCtrol (FD_HANDLE  AutoFocusHd, int8_t AfDirMode )
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AfStepSet (FD_HANDLE  AutoFocusHd, int32_t AfStep )
{
    return GMI_SUCCESS;
}

FD_HANDLE   GMI_ZoomCreate(SensorId  SId)
{
    return (FD_HANDLE) 1;
}

void_t      GMI_ZoomDestroy(FD_HANDLE  ZoomHd)
{
    return;
}

GMI_RESULT	GMI_ZoomPositionGet (FD_HANDLE	ZoomHd, int32_t *CurZoomPos)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ZoomPositionSet (FD_HANDLE  ZoomHd, int32_t  ZoomPos)
{
    return GMI_SUCCESS;
}

GMI_RESULT	GMI_ZoomRangeGet (FD_HANDLE  ZoomHd, int32_t *MinZoomPos, int32_t *MaxZoomPos)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ZoomMotorReset(FD_HANDLE  ZoomHd)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_ZoomCtrol (FD_HANDLE  ZoomHd, int8_t  ZoomMode)
{
    return GMI_SUCCESS;
}

GMI_RESULT	GMI_ZoomStepSet (FD_HANDLE	ZoomHd, int32_t  ZoomStep)
{
    return GMI_SUCCESS;
}

FD_HANDLE   GMI_AudioEncCreate( uint16_t s_Port, AudioEncParam* AudEncParamPtr )
{
    return (FD_HANDLE) 1;
}

void_t      GMI_AudioEncDestroy( FD_HANDLE AudioEncHd )
{
}

GMI_RESULT  GMI_AudioEncStart( FD_HANDLE AudioEncHd )
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AudioEncStop( FD_HANDLE AudioEncHd)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AudioSetEncConfig(FD_HANDLE AudioEncHd, AudioEncParam *AudEncParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AudioGetEncConfig(FD_HANDLE AudioEncHd, AudioEncParam *AudEncParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AudioEncSetCB( FD_HANDLE AudioEncHd, MediaEncCallBack AudioEncCB, void *UserDataPtr)
{
    return GMI_SUCCESS;
}

FD_HANDLE   GMI_AudioDecCreate( uint16_t AudioPort, AudioDecParam* AudDecParamPtr )
{
    return (FD_HANDLE) 1;
}

void_t      GMI_AudioDecDestroy( FD_HANDLE AudioDecHd )
{
    return;
}

GMI_RESULT  GMI_AudioSetDecConfig(FD_HANDLE AudioDecHd, AudioDecParam *AudDecParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AudioGetDecConfig(FD_HANDLE AudioDecHd, AudioDecParam *AudDecParamPtr)
{
    return GMI_SUCCESS;
}

GMI_RESULT  GMI_AudioDecOneFrame( FD_HANDLE AudioDecHd, uint8_t *AudStreamPtr, uint32_t AudStreamLen)
{
    return GMI_SUCCESS;
}

#endif
