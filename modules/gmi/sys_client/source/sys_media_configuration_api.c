#include "log.h"
#include "sys_client.h"
#include "sys_command_excute.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"


GMI_RESULT SysGetEncodeStreamNum(uint16_t SessionId, uint32_t AuthValue, uint32_t *StreamNum)
{
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;

    if (StreamNum == NULL )
    {
        return GMI_INVALID_PARAMETER;
    }

    Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_ENCODECFG_REQ, &RspAttrCnt, &SysRspAttrPtr);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("SysGetCmdExcute fail, Result = 0x%lx\n", Result);
        goto errExit;
    }
    if (RspAttrCnt <= 0
            || SysRspAttrPtr == NULL)
    {
        SYS_CLIENT_ERROR("\n");
        goto errExit;
    }

    *StreamNum = RspAttrCnt;

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_SUCCESS;
errExit:
    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysGetEncodeCfg(uint16_t SessionId, uint32_t AuthValue, SysPkgEncodeCfg *SysEncodeCfgPtr, uint32_t ReqEncodeCfgNum, uint32_t *RspEncodeCfgNum)
{
    uint8_t    i;
    uint32_t   EncodeCfgCnt     = 0;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (NULL == SysEncodeCfgPtr
            || NULL == RspEncodeCfgNum)
    {
        return GMI_INVALID_PARAMETER;
    }

    Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_ENCODECFG_REQ, &RspAttrCnt, &SysRspAttrPtr);
    if (FAILED(Result))
    {
        goto errExit;
    }
    if (RspAttrCnt <= 0
            || SysRspAttrPtr == NULL)
    {
        goto errExit;
    }

    EncodeCfgCnt = ReqEncodeCfgNum >= RspAttrCnt ?  RspAttrCnt : ReqEncodeCfgNum;

    SysRspAttrTmpPtr = SysRspAttrPtr;
    for (i = 0; i < EncodeCfgCnt; i++)
    {
        if (SysRspAttrTmpPtr->s_Type == TYPE_ENCODECFG
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgEncodeCfg))
        {
            memcpy(&SysEncodeCfgPtr[i], SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            SysEncodeCfgPtr[i].s_VideoId        = ntohl(SysEncodeCfgPtr[i].s_VideoId);
            SysEncodeCfgPtr[i].s_Compression    = ntohl(SysEncodeCfgPtr[i].s_Compression);
            SysEncodeCfgPtr[i].s_PicWidth       = ntohl(SysEncodeCfgPtr[i].s_PicWidth);
            SysEncodeCfgPtr[i].s_PicHeight      = ntohl(SysEncodeCfgPtr[i].s_PicHeight);
            SysEncodeCfgPtr[i].s_BitrateCtrl    = ntohl(SysEncodeCfgPtr[i].s_BitrateCtrl);
            SysEncodeCfgPtr[i].s_Quality        = ntohl(SysEncodeCfgPtr[i].s_Quality);
            SysEncodeCfgPtr[i].s_FPS            = ntohl(SysEncodeCfgPtr[i].s_FPS);
            SysEncodeCfgPtr[i].s_BitRateAverage = ntohl(SysEncodeCfgPtr[i].s_BitRateAverage);
            SysEncodeCfgPtr[i].s_BitRateUp      = ntohl(SysEncodeCfgPtr[i].s_BitRateUp);
            SysEncodeCfgPtr[i].s_BitRateDown    = ntohl(SysEncodeCfgPtr[i].s_BitRateDown);
            SysEncodeCfgPtr[i].s_Gop            = ntohl(SysEncodeCfgPtr[i].s_Gop);
            SysEncodeCfgPtr[i].s_Rotate         = ntohl(SysEncodeCfgPtr[i].s_Rotate);
            SysEncodeCfgPtr[i].s_Flag           = ntohl(SysEncodeCfgPtr[i].s_Flag);
            SysEncodeCfgPtr[i].s_StreamType     = ntohl(SysEncodeCfgPtr[i].s_StreamType);
            Exist = true;
        }
        SysRspAttrTmpPtr++;
    }

    if (!Exist)
    {
        SYS_CLIENT_ERROR("not found valid data\n");
        goto errExit;
    }

    *RspEncodeCfgNum = EncodeCfgCnt;

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_SUCCESS;
errExit:
    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetEncodeCfg(uint16_t SessionId, uint32_t AuthValue, int32_t StreamId, SysPkgEncodeCfg *SysEncodeCfgPtr)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgEncodeCfg SysEncodeCfg;

    memset(&SysEncodeCfg, 0, sizeof(SysPkgEncodeCfg));
    memcpy(&SysEncodeCfg, SysEncodeCfgPtr, sizeof(SysPkgEncodeCfg));
    SysEncodeCfg.s_VideoId        = htonl(SysEncodeCfg.s_VideoId );
    SysEncodeCfg.s_Compression    = htonl(SysEncodeCfg.s_Compression);
    SysEncodeCfg.s_PicWidth       = htonl(SysEncodeCfg.s_PicWidth);
    SysEncodeCfg.s_PicHeight      = htonl(SysEncodeCfg.s_PicHeight);
    SysEncodeCfg.s_BitrateCtrl    = htonl(SysEncodeCfg.s_BitrateCtrl);
    SysEncodeCfg.s_Quality        = htonl(SysEncodeCfg.s_Quality);
    SysEncodeCfg.s_FPS            = htonl(SysEncodeCfg.s_FPS);
    SysEncodeCfg.s_BitRateAverage = htonl(SysEncodeCfg.s_BitRateAverage);
    SysEncodeCfg.s_BitRateUp      = htonl(SysEncodeCfg.s_BitRateUp);
    SysEncodeCfg.s_BitRateDown    = htonl(SysEncodeCfg.s_BitRateDown);
    SysEncodeCfg.s_Gop            = htonl(SysEncodeCfg.s_Gop);
    SysEncodeCfg.s_Rotate         = htonl(SysEncodeCfg.s_Rotate);
    SysEncodeCfg.s_Flag           = htonl(SysEncodeCfg.s_Flag);
    SysEncodeCfg.s_StreamType     = htonl(SysEncodeCfg.s_StreamType);

    SysReqAttr.s_Type = TYPE_ENCODECFG;
    SysReqAttr.s_Attr = (void_t*)&SysEncodeCfg;
    SysReqAttr.s_AttrLength = sizeof(SysPkgEncodeCfg);

    Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_ENCODECFG_REQ, ReqAttrCnt, &SysReqAttr );
    if (FAILED(Result))
    {
        goto errExit;
    }

    return GMI_SUCCESS;
errExit:
    return GMI_FAIL;
}


GMI_RESULT SysGetVideoEncodeMirror(uint16_t SessionId, uint32_t AuthValue, int32_t StreamId, int32_t *Mirror)
{
    GMI_RESULT        Result;
    uint32_t          StreamNum;
    uint32_t          RealStreamNum;
    SysPkgEncodeCfg  *SysEncodeCfgPtr = NULL;

    if (NULL == Mirror)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Result = SysGetEncodeStreamNum(SessionId, AuthValue, &StreamNum);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("get stream num fail, Result = 0x%lx\n", Result);
            break;
        }

        if ((uint32_t)StreamId >= StreamNum)
        {
            return GMI_INVALID_PARAMETER;
        }

        SysEncodeCfgPtr = (SysPkgEncodeCfg*)malloc(sizeof(SysPkgEncodeCfg) * StreamNum);
        if (NULL == SysEncodeCfgPtr)
        {
            return GMI_OUT_OF_MEMORY;
        }

        Result = SysGetEncodeCfg(SessionId, AuthValue, SysEncodeCfgPtr, StreamNum, &RealStreamNum);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("get encode config fail, Result = 0x%lx\n", Result);
            break;
        }

        *Mirror = SysEncodeCfgPtr[StreamId].s_Rotate;

        if (NULL != SysEncodeCfgPtr)
        {
            free(SysEncodeCfgPtr);
            SysEncodeCfgPtr = NULL;
        }

        return GMI_SUCCESS;
    }
    while(0);

    if (NULL != SysEncodeCfgPtr)
    {
        free(SysEncodeCfgPtr);
        SysEncodeCfgPtr = NULL;
    }
    return GMI_FAIL;
}


GMI_RESULT SysSetVideoEncodeMirror(uint16_t SessionId, uint32_t AuthValue, int32_t StreamId, int32_t Mirror)
{
    GMI_RESULT        Result;
    uint32_t          StreamNum;
    uint32_t          RealStreamNum;
    SysPkgEncodeCfg  *SysEncodeCfgPtr = NULL;

    do
    {
        Result = SysGetEncodeStreamNum(SessionId, AuthValue, &StreamNum);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("get stream num fail, Result = 0x%lx\n", Result);
            break;
        }

        if ((uint32_t)StreamId >= StreamNum)
        {
            return GMI_INVALID_PARAMETER;
        }

        SysEncodeCfgPtr = (SysPkgEncodeCfg*)malloc(sizeof(SysPkgEncodeCfg) * StreamNum);
        if (NULL == SysEncodeCfgPtr)
        {
            return GMI_OUT_OF_MEMORY;
        }

        Result = SysGetEncodeCfg(SessionId, AuthValue, SysEncodeCfgPtr, StreamNum, &RealStreamNum);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("get encode config fail, Result = 0x%lx\n", Result);
            break;
        }

        SysEncodeCfgPtr[StreamId].s_Rotate = Mirror;
        Result = SysSetEncodeCfg(SessionId, AuthValue, StreamId, &SysEncodeCfgPtr[StreamId]);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("set encode config fail, Result = 0x%lx\n", Result);
            break;
        }

        if (NULL != SysEncodeCfgPtr)
        {
            free(SysEncodeCfgPtr);
            SysEncodeCfgPtr = NULL;
        }
        return GMI_SUCCESS;
    }
    while(0);

    if (NULL != SysEncodeCfgPtr)
    {
        free(SysEncodeCfgPtr);
        SysEncodeCfgPtr = NULL;
    }
    return GMI_FAIL;
}


GMI_RESULT SysGetVideoSource(uint16_t SessionId, uint32_t AuthValue, SysPkgVideoSource *SysVideoSource)
{
    uint8_t    i;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (SysVideoSource == NULL )
    {
        return GMI_INVALID_PARAMETER;
    }

    Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_VIDEO_SOURCE_INFO_REQ, &RspAttrCnt, &SysRspAttrPtr);
    if (FAILED(Result))
    {
        goto errExit;
    }
    if (RspAttrCnt <= 0
            || SysRspAttrPtr == NULL)
    {
        goto errExit;
    }

    SysRspAttrTmpPtr = SysRspAttrPtr;
    for (i = 0; i < RspAttrCnt; i++)
    {
        if (SysRspAttrTmpPtr->s_Type == TYPE_VIDEO_SOURCE
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgVideoSource))
        {
            memcpy(SysVideoSource, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            SysVideoSource->s_Mirror    = ntohl(SysVideoSource->s_Mirror);
            SysVideoSource->s_SrcFps    = ntohl(SysVideoSource->s_SrcFps);
            SysVideoSource->s_SrcHeight = ntohl(SysVideoSource->s_SrcHeight);
            SysVideoSource->s_SrcWidth  = ntohl(SysVideoSource->s_SrcWidth);
            SysVideoSource->s_VideoId   = ntohl(SysVideoSource->s_VideoId);
            Exist = true;
            break;
        }
        SysRspAttrTmpPtr++;
    }

    if (!Exist)
    {
        SYS_CLIENT_ERROR("not found valid data\n");
        goto errExit;
    }

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_SUCCESS;
errExit:
    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysGetVideoSourceMirror(uint16_t SessionId, uint32_t AuthValue, int32_t *MirrorPtr)
{
    GMI_RESULT Result = GMI_SUCCESS;
    SysPkgVideoSource SysVideoSource;

    if (NULL == MirrorPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        memset(&SysVideoSource, 0, sizeof(SysPkgVideoSource));
        Result = SysGetVideoSource(SessionId, AuthValue, &SysVideoSource);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("get video source fail, Result = 0x%lx\n", Result);
            break;
        }

        *MirrorPtr = SysVideoSource.s_Mirror;

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysSetVideoSourceMirror(uint16_t SessionId, uint32_t AuthValue, int32_t Mirror)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgVideoSource SysVideoSource;

    if (0 > Mirror
            || 4 < Mirror)
    {
        SYS_CLIENT_ERROR("Mirror %d incorrect\n", Mirror);
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        memset(&SysVideoSource, 0, sizeof(SysPkgVideoSource));
        Result = SysGetVideoSource(SessionId, AuthValue, &SysVideoSource);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("get video source fail, Result = 0x%lx\n", Result);
            break;
        }

        if (SysVideoSource.s_Mirror == Mirror)
        {
            SYS_CLIENT_INFO("Device Mirror %d in current, User mirror %d is the same\n", SysVideoSource.s_Mirror, Mirror);
            return GMI_SUCCESS;
        }

        SysVideoSource.s_Mirror    = htonl(Mirror);
        SysVideoSource.s_SrcFps    = htonl(SysVideoSource.s_SrcFps);
        SysVideoSource.s_SrcHeight = htonl(SysVideoSource.s_SrcHeight);
        SysVideoSource.s_SrcWidth  = htonl(SysVideoSource.s_SrcWidth);
        SysVideoSource.s_VideoId   = htonl(SysVideoSource.s_VideoId);

        SysReqAttr.s_Type = TYPE_VIDEO_SOURCE;
        SysReqAttr.s_Attr = (void_t*)&SysVideoSource;
        SysReqAttr.s_AttrLength = sizeof(SysPkgVideoSource);

        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_VIDEO_SOURCE_INFO_REQ, ReqAttrCnt, &SysReqAttr );
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("set video source fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysGetImaging(uint16_t SessionId, uint32_t AuthValue, SysPkgImaging *SysImagingPtr)
{
    uint8_t    i;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (SysImagingPtr == NULL )
    {
        return GMI_INVALID_PARAMETER;
    }

    Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_IMAGING_REQ, &RspAttrCnt, &SysRspAttrPtr);
    if (FAILED(Result))
    {
        goto errExit;
    }
    if (RspAttrCnt <= 0
            || SysRspAttrPtr == NULL)
    {
        goto errExit;
    }

    SysRspAttrTmpPtr = SysRspAttrPtr;
    for (i = 0; i < RspAttrCnt; i++)
    {
        if (SysRspAttrTmpPtr->s_Type == TYPE_IMAGING
                && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgImaging))
        {
            memcpy(SysImagingPtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
            SysImagingPtr->s_VideoId    = ntohl(SysImagingPtr->s_VideoId);
            SysImagingPtr->s_Brightness = ntohl(SysImagingPtr->s_Brightness);
            SysImagingPtr->s_Contrast   = ntohl(SysImagingPtr->s_Contrast);
            SysImagingPtr->s_Saturation = ntohl(SysImagingPtr->s_Saturation);
            SysImagingPtr->s_Hue        = ntohl(SysImagingPtr->s_Hue);
            SysImagingPtr->s_Sharpness  = ntohl(SysImagingPtr->s_Sharpness);
            SysImagingPtr->s_Exposure.s_ExposureMode     = ntohl(SysImagingPtr->s_Exposure.s_ExposureMode);
            SysImagingPtr->s_Exposure.s_ShutterMax       = ntohl(SysImagingPtr->s_Exposure.s_ShutterMax);
            SysImagingPtr->s_Exposure.s_ShutterMin       = ntohl(SysImagingPtr->s_Exposure.s_ShutterMin);
            SysImagingPtr->s_Exposure.s_GainMax          = ntohl(SysImagingPtr->s_Exposure.s_GainMax);
            Exist = true;
            break;
        }
        SysRspAttrTmpPtr++;
    }

    if (!Exist)
    {
        SYS_CLIENT_ERROR("not found valid data\n");
        goto errExit;
    }

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_SUCCESS;
errExit:
    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetImaging(uint16_t SessionId, uint32_t AuthValue, SysPkgImaging *SysImagingPtr)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgImaging SysImaging;

    memset(&SysImaging, 0, sizeof(SysPkgImaging));
    memcpy(&SysImaging, SysImagingPtr, sizeof(SysPkgImaging));
    SysImaging.s_VideoId    = htonl(SysImaging.s_VideoId);
    SysImaging.s_Brightness = htonl(SysImaging.s_Brightness);
    SysImaging.s_Contrast   = htonl(SysImaging.s_Contrast);
    SysImaging.s_Saturation = htonl(SysImaging.s_Saturation);
    SysImaging.s_Hue        = htonl(SysImaging.s_Hue);
    SysImaging.s_Sharpness  = htonl(SysImaging.s_Sharpness);
    SysImaging.s_Exposure.s_ExposureMode     = htonl(SysImaging.s_Exposure.s_ExposureMode);
    SysImaging.s_Exposure.s_ShutterMax       = htonl(SysImaging.s_Exposure.s_ShutterMax);
    SysImaging.s_Exposure.s_ShutterMin       = htonl(SysImaging.s_Exposure.s_ShutterMin);
    SysImaging.s_Exposure.s_GainMax          = htonl(SysImaging.s_Exposure.s_GainMax);

    SysReqAttr.s_Type = TYPE_IMAGING;
    SysReqAttr.s_Attr = (void_t*)&SysImaging;
    SysReqAttr.s_AttrLength = sizeof(SysPkgImaging);

    Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_IMAGING_REQ, ReqAttrCnt, &SysReqAttr );
    if (FAILED(Result))
    {
        goto errExit;
    }

    return GMI_SUCCESS;
errExit:
    return GMI_FAIL;
}


GMI_RESULT SysGetWhiteBalance(uint16_t SessionId, uint32_t AuthValue, SysPkgWhiteBalance *SysWhiteBalancePtr)
{
    uint8_t    i;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;


    if (NULL == SysWhiteBalancePtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_WHITEBALANCE_REQ, &RspAttrCnt, &SysRspAttrPtr);
        if (FAILED(Result))
        {
            break;
        }
        if (RspAttrCnt <= 0
                || SysRspAttrPtr == NULL)
        {
            break;
        }

        SysRspAttrTmpPtr = SysRspAttrPtr;
        for (i = 0; i < RspAttrCnt; i++)
        {
            if (SysRspAttrTmpPtr->s_Type == TYPE_WHITE_BALANCE
                    && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgWhiteBalance))
            {
                memcpy(SysWhiteBalancePtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysWhiteBalancePtr->s_Mode    = ntohl(SysWhiteBalancePtr->s_Mode);
                SysWhiteBalancePtr->s_BGain   = ntohl(SysWhiteBalancePtr->s_BGain);
                SysWhiteBalancePtr->s_RGain   = ntohl(SysWhiteBalancePtr->s_RGain);
                Exist = true;
                break;
            }
            SysRspAttrTmpPtr++;
        }

        if (!Exist)
        {
            break;
        }

        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
    }
    while(0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetWhiteBalance(uint16_t SessionId, uint32_t AuthValue, SysPkgWhiteBalance *SysWhiteBalancePtr)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgWhiteBalance SysWhiteBalance;

    memset(&SysWhiteBalance, 0, sizeof(SysPkgWhiteBalance));
    memcpy(&SysWhiteBalance, SysWhiteBalancePtr, sizeof(SysPkgWhiteBalance));
    SysWhiteBalance.s_Mode    = htonl(SysWhiteBalance.s_Mode);
    SysWhiteBalance.s_BGain   = htonl(SysWhiteBalance.s_BGain);
    SysWhiteBalance.s_RGain   = htonl(SysWhiteBalance.s_RGain);

    SysReqAttr.s_Type = TYPE_WHITE_BALANCE;
    SysReqAttr.s_Attr = (void_t*)&SysWhiteBalance;
    SysReqAttr.s_AttrLength = sizeof(SysPkgWhiteBalance);

    do
    {
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_WHITEBALANCE_REQ, ReqAttrCnt, &SysReqAttr );
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SYSCODE_SET_WHITEBALANCE_REQ fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysGetDaynight(uint16_t SessionId, uint32_t AuthValue, SysPkgDaynight *SysDaynightPtr)
{
    uint8_t    i;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;


    if (NULL == SysDaynightPtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_DAY_NIGHT_REQ, &RspAttrCnt, &SysRspAttrPtr);
        if (FAILED(Result))
        {
            break;
        }
        if (RspAttrCnt <= 0
                || SysRspAttrPtr == NULL)
        {
            SYS_CLIENT_ERROR("RspAttrCnt %d, or SysRspAttrPtr is null\n", RspAttrCnt);
            break;
        }

        SysRspAttrTmpPtr = SysRspAttrPtr;
        for (i = 0; i < RspAttrCnt; i++)
        {
            if (SysRspAttrTmpPtr->s_Type == TYPE_DAY_NIGHT
                    && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgDaynight))
            {
                memcpy(SysDaynightPtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysDaynightPtr->s_Mode			= ntohl(SysDaynightPtr->s_Mode);
                SysDaynightPtr->s_DurationTime  = ntohl(SysDaynightPtr->s_DurationTime);
                SysDaynightPtr->s_NightToDayThr = ntohl(SysDaynightPtr->s_NightToDayThr);
                SysDaynightPtr->s_DayToNightThr = ntohl(SysDaynightPtr->s_DayToNightThr);
                for (i = 0; i < DAY_NIGHT_SCHED_WEEKS; i++)
                {
                    SysDaynightPtr->s_SchedEnable[i]    = ntohl(SysDaynightPtr->s_SchedEnable[i]);
                    SysDaynightPtr->s_SchedStartTime[i] = ntohl(SysDaynightPtr->s_SchedStartTime[i]);
                    SysDaynightPtr->s_SchedEndTime[i]   = ntohl(SysDaynightPtr->s_SchedEndTime[i]);
                }
                Exist = true;
                break;
            }
            SysRspAttrTmpPtr++;
        }

        if (!Exist)
        {
            break;
        }

        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
    }
    while(0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetDaynight(uint16_t SessionId, uint32_t AuthValue, SysPkgDaynight *SysDaynightPtr)
{
    int32_t    i;
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgDaynight SysDaynight;

    memset(&SysDaynight, 0, sizeof(SysPkgDaynight));
    memcpy(&SysDaynight, SysDaynightPtr, sizeof(SysPkgDaynight));

    SysReqAttr.s_Type = TYPE_DAY_NIGHT;
    SysReqAttr.s_Attr = (void_t*)&SysDaynight;
    SysReqAttr.s_AttrLength = sizeof(SysPkgDaynight);
    SysDaynight.s_Mode          = htonl(SysDaynight.s_Mode);
    SysDaynight.s_DurationTime  = htonl(SysDaynight.s_DurationTime);
    SysDaynight.s_NightToDayThr = htonl(SysDaynight.s_NightToDayThr);
    SysDaynight.s_DayToNightThr = htonl(SysDaynight.s_DayToNightThr);
    for (i = 0; i < DAY_NIGHT_SCHED_WEEKS; i++)
    {
        SysDaynight.s_SchedEnable[i]    = htonl(SysDaynight.s_SchedEnable[i]);
        SysDaynight.s_SchedStartTime[i] = htonl(SysDaynight.s_SchedStartTime[i]);
        SysDaynight.s_SchedEndTime[i]   = htonl(SysDaynight.s_SchedEndTime[i]);
    }

    do
    {
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_DAY_NIGHT_REQ, ReqAttrCnt, &SysReqAttr );
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SYSCODE_SET_DAY_NIGHT_REQ fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysForceIdr(uint16_t SessionId, uint32_t AuthValue, int32_t StreamId)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};

    SysPkgForceIdr SysForceIdr;
    memset (&SysForceIdr, 0, sizeof(SysPkgForceIdr));    
    SysForceIdr.s_VideoId   = htonl(1);
    SysForceIdr.s_Flag      = htonl(StreamId);
    
    SysReqAttr.s_Type       = TYPE_FORCEIDR;
    SysReqAttr.s_Attr       = (void_t*)&SysForceIdr;
    SysReqAttr.s_AttrLength = sizeof(SysPkgForceIdr);

    Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_FORCE_IDR_REQ, ReqAttrCnt, &SysReqAttr );
    if (FAILED(Result))
    {
        goto errExit;
    }

    return GMI_SUCCESS;
errExit:
    return GMI_FAIL;
}


GMI_RESULT SysGetAdvancedImaging(uint16_t SessionId, uint32_t AuthValue, SysPkgAdvancedImaging *SysAdvancedImagingPtr)
{
    uint8_t    i;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (SysAdvancedImagingPtr == NULL )
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_ADVANCED_IMAGING_REQ, &RspAttrCnt, &SysRspAttrPtr);
        if (FAILED(Result))
        {
            break;
        }
        if (RspAttrCnt <= 0
                || SysRspAttrPtr == NULL)
        {
            break;
        }

        SysRspAttrTmpPtr = SysRspAttrPtr;
        for (i = 0; i < RspAttrCnt; i++)
        {
            if (SysRspAttrTmpPtr->s_Type == TYPE_ADVANCED_IMAGING
                    && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgAdvancedImaging))
            {
                memcpy(SysAdvancedImagingPtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysAdvancedImagingPtr->s_VideoId           = ntohl(SysAdvancedImagingPtr->s_VideoId);
                SysAdvancedImagingPtr->s_LocalExposure     = ntohs(SysAdvancedImagingPtr->s_LocalExposure);
                SysAdvancedImagingPtr->s_MctfStrength      = ntohs(SysAdvancedImagingPtr->s_MctfStrength);
                SysAdvancedImagingPtr->s_DcIrisDuty        = ntohs(SysAdvancedImagingPtr->s_DcIrisDuty);
                SysAdvancedImagingPtr->s_AeTargetRatio     = ntohs(SysAdvancedImagingPtr->s_AeTargetRatio);
                Exist = true;
                break;
            }
            SysRspAttrTmpPtr++;
        }

        if (!Exist)
        {
            SYS_CLIENT_ERROR("not found valid data\n");
            break;
        }

        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
    }
    while(0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetAdvancedImaging(uint16_t SessionId, uint32_t AuthValue, SysPkgAdvancedImaging *SysAdvancedImagingPtr)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgAdvancedImaging SysAdvancedImaging;

    do
    {
        memset(&SysAdvancedImaging, 0, sizeof(SysPkgImaging));
        memcpy(&SysAdvancedImaging, SysAdvancedImagingPtr, sizeof(SysPkgAdvancedImaging));
        SysAdvancedImaging.s_VideoId        = htonl(SysAdvancedImaging.s_VideoId);
        SysAdvancedImaging.s_LocalExposure  = htons(SysAdvancedImaging.s_LocalExposure);
        SysAdvancedImaging.s_MctfStrength   = htons(SysAdvancedImaging.s_MctfStrength);
        SysAdvancedImaging.s_DcIrisDuty     = htons(SysAdvancedImaging.s_DcIrisDuty);
        SysAdvancedImaging.s_AeTargetRatio  = htons(SysAdvancedImaging.s_AeTargetRatio);

        SysReqAttr.s_Type = TYPE_ADVANCED_IMAGING;
        SysReqAttr.s_Attr = (void_t*)&SysAdvancedImaging;
        SysReqAttr.s_AttrLength = sizeof(SysPkgAdvancedImaging);

        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_ADVANCED_IMAGING_REQ, ReqAttrCnt, &SysReqAttr );
        if (FAILED(Result))
        {
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysGetShowInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgShowCfg *SysShowInfoPtr, uint32_t ReqShowInfoNum, uint32_t *RspShowInfoNum)
{
    uint8_t    i;
    uint32_t   ShowInfoCnt;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (NULL == SysShowInfoPtr
            || NULL == RspShowInfoNum)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_SHOWCFG_REQ, &RspAttrCnt, &SysRspAttrPtr);
        if (FAILED(Result))
        {
            break;
        }
        if (RspAttrCnt <= 0
                || SysRspAttrPtr == NULL)
        {
            break;
        }


        ShowInfoCnt = ReqShowInfoNum >= RspAttrCnt ? RspAttrCnt : ReqShowInfoNum;

        SysRspAttrTmpPtr = SysRspAttrPtr;
        for (i = 0; i < ShowInfoCnt; i++)
        {
            if (SysRspAttrTmpPtr->s_Type == TYPE_SHOWCFG
                    && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgShowCfg))
            {
                memcpy(&SysShowInfoPtr[i], SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysShowInfoPtr[i].s_VideoId                = ntohl(SysShowInfoPtr[i].s_VideoId);
                SysShowInfoPtr[i].s_Flag                   = ntohl(SysShowInfoPtr[i].s_Flag);

                SysShowInfoPtr[i].s_TimeInfo.s_Enable      = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_Enable);
                SysShowInfoPtr[i].s_TimeInfo.s_Language    = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_Language);
                SysShowInfoPtr[i].s_TimeInfo.s_DisplayX    = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_DisplayX );
                SysShowInfoPtr[i].s_TimeInfo.s_DisplayY    = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_DisplayY);
                SysShowInfoPtr[i].s_TimeInfo.s_DateStyle   = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_DateStyle);
                SysShowInfoPtr[i].s_TimeInfo.s_TimeStyle   = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_TimeStyle);
                SysShowInfoPtr[i].s_TimeInfo.s_FontColor   = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_FontColor);
                SysShowInfoPtr[i].s_TimeInfo.s_FontSize    = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_FontSize);
                SysShowInfoPtr[i].s_TimeInfo.s_FontBlod    = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_FontBlod);
                SysShowInfoPtr[i].s_TimeInfo.s_FontRotate  = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_FontRotate);
                SysShowInfoPtr[i].s_TimeInfo.s_FontItalic  = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_FontItalic);
                SysShowInfoPtr[i].s_TimeInfo.s_FontOutline = ntohl(SysShowInfoPtr[i].s_TimeInfo.s_FontOutline);

                SysShowInfoPtr[i].s_ChannelInfo.s_Enable     = ntohl(SysShowInfoPtr[i].s_ChannelInfo.s_Enable);
                SysShowInfoPtr[i].s_ChannelInfo.s_DisplayX   = ntohl(SysShowInfoPtr[i].s_ChannelInfo.s_DisplayX);
                SysShowInfoPtr[i].s_ChannelInfo.s_DisplayY   = ntohl(SysShowInfoPtr[i].s_ChannelInfo.s_DisplayY);
                SysShowInfoPtr[i].s_ChannelInfo.s_FontColor  = ntohl(SysShowInfoPtr[i].s_ChannelInfo.s_FontColor);
                SysShowInfoPtr[i].s_ChannelInfo.s_FontSize   = ntohl(SysShowInfoPtr[i].s_ChannelInfo.s_FontSize);
                SysShowInfoPtr[i].s_ChannelInfo.s_FontBlod   = ntohl(SysShowInfoPtr[i].s_ChannelInfo.s_FontBlod);
                SysShowInfoPtr[i].s_ChannelInfo.s_FontRotate = ntohl(SysShowInfoPtr[i].s_ChannelInfo.s_FontRotate);
                SysShowInfoPtr[i].s_ChannelInfo.s_FontItalic = ntohl(SysShowInfoPtr[i].s_ChannelInfo.s_FontItalic);
                SysShowInfoPtr[i].s_ChannelInfo.s_FontOutline= ntohl(SysShowInfoPtr[i].s_ChannelInfo.s_FontOutline);
                Exist = true;
            }
            SysRspAttrTmpPtr++;
        }

        if (!Exist)
        {
            SYS_CLIENT_ERROR("not found valid data\n");
            break;
        }

        *RspShowInfoNum = ShowInfoCnt;

        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
    }
    while(0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetShowInfo(uint16_t SessionId, uint32_t AuthValue, SysPkgShowCfg *SysShowInfoPtr)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgShowCfg SysShowCfg;

    memset(&SysShowCfg, 0, sizeof(SysPkgShowCfg));
    memcpy(&SysShowCfg, SysShowInfoPtr, sizeof(SysPkgShowCfg));

    SysShowCfg.s_VideoId                = htonl(SysShowCfg.s_VideoId);
    SysShowCfg.s_Flag                   = ntohl(SysShowCfg.s_Flag);

    SysShowCfg.s_TimeInfo.s_Enable      = htonl(SysShowCfg.s_TimeInfo.s_Enable);
    SysShowCfg.s_TimeInfo.s_Language    = htonl(SysShowCfg.s_TimeInfo.s_Language);
    SysShowCfg.s_TimeInfo.s_DisplayX    = htonl(SysShowCfg.s_TimeInfo.s_DisplayX );
    SysShowCfg.s_TimeInfo.s_DisplayY    = htonl(SysShowCfg.s_TimeInfo.s_DisplayY);
    SysShowCfg.s_TimeInfo.s_DateStyle   = htonl(SysShowCfg.s_TimeInfo.s_DateStyle);
    SysShowCfg.s_TimeInfo.s_TimeStyle   = htonl(SysShowCfg.s_TimeInfo.s_TimeStyle);
    SysShowCfg.s_TimeInfo.s_FontColor   = htonl(SysShowCfg.s_TimeInfo.s_FontColor);
    SysShowCfg.s_TimeInfo.s_FontSize    = htonl(SysShowCfg.s_TimeInfo.s_FontSize);
    SysShowCfg.s_TimeInfo.s_FontBlod    = htonl(SysShowCfg.s_TimeInfo.s_FontBlod);
    SysShowCfg.s_TimeInfo.s_FontRotate  = htonl(SysShowCfg.s_TimeInfo.s_FontRotate);
    SysShowCfg.s_TimeInfo.s_FontItalic  = htonl(SysShowCfg.s_TimeInfo.s_FontItalic);
    SysShowCfg.s_TimeInfo.s_FontOutline = htonl(SysShowCfg.s_TimeInfo.s_FontOutline);

    SysShowCfg.s_ChannelInfo.s_Enable     = htonl(SysShowCfg.s_ChannelInfo.s_Enable);
    SysShowCfg.s_ChannelInfo.s_DisplayX   = htonl(SysShowCfg.s_ChannelInfo.s_DisplayX);
    SysShowCfg.s_ChannelInfo.s_DisplayY   = htonl(SysShowCfg.s_ChannelInfo.s_DisplayY);
    SysShowCfg.s_ChannelInfo.s_FontColor  = htonl(SysShowCfg.s_ChannelInfo.s_FontColor);
    SysShowCfg.s_ChannelInfo.s_FontSize   = htonl(SysShowCfg.s_ChannelInfo.s_FontSize);
    SysShowCfg.s_ChannelInfo.s_FontBlod   = htonl(SysShowCfg.s_ChannelInfo.s_FontBlod);
    SysShowCfg.s_ChannelInfo.s_FontRotate = htonl(SysShowCfg.s_ChannelInfo.s_FontRotate);
    SysShowCfg.s_ChannelInfo.s_FontItalic = htonl(SysShowCfg.s_ChannelInfo.s_FontItalic);
    SysShowCfg.s_ChannelInfo.s_FontOutline= htonl(SysShowCfg.s_ChannelInfo.s_FontOutline);

    SysReqAttr.s_Type = TYPE_SHOWCFG;
    SysReqAttr.s_Attr = (void_t*)&SysShowCfg;
    SysReqAttr.s_AttrLength = sizeof(SysPkgShowCfg);

    Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_SHOWCFG_REQ, ReqAttrCnt, &SysReqAttr);
    if (FAILED(Result))
    {
        goto errExit;
    }

    return GMI_SUCCESS;
errExit:
    return GMI_FAIL;
}


GMI_RESULT SysStartAudioDecode(uint16_t SessionId, uint32_t AuthValue, SysPkgAudioDecParam *SysAudioDecParamPtr)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgAudioDecParam SysAudioDecParam;

    memset(&SysAudioDecParam, 0, sizeof(SysPkgAudioDecParam));
    memcpy(&SysAudioDecParam, SysAudioDecParamPtr, sizeof(SysPkgAudioDecParam));

    SysReqAttr.s_Type = TYPE_AUDIO_DECODE;
    SysReqAttr.s_Attr = (void_t*)&SysAudioDecParam;
    SysReqAttr.s_AttrLength = sizeof(SysPkgAudioDecParam);
    SysAudioDecParam.s_AudioId      = ntohl(SysAudioDecParam.s_AudioId);
    SysAudioDecParam.s_AecDelayTime = ntohs(SysAudioDecParam.s_AecDelayTime);
    SysAudioDecParam.s_AecFlag      = ntohs(SysAudioDecParam.s_AecFlag);
    SysAudioDecParam.s_BitRate      = ntohs(SysAudioDecParam.s_BitRate);
    SysAudioDecParam.s_BitWidth     = ntohl(SysAudioDecParam.s_BitWidth);
    SysAudioDecParam.s_ChannelNum   = ntohs(SysAudioDecParam.s_ChannelNum);
    SysAudioDecParam.s_Codec        = ntohl(SysAudioDecParam.s_Codec);
    SysAudioDecParam.s_FrameRate    = ntohs(SysAudioDecParam.s_FrameRate);
    SysAudioDecParam.s_SampleFreq   = ntohl(SysAudioDecParam.s_SampleFreq);
    SysAudioDecParam.s_Volume       = ntohs(SysAudioDecParam.s_Volume);

    do
    {
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_START_AUDIO_DECODE_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("start audio decode fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysStopAudioDecode(uint16_t SessionId, uint32_t AuthValue, int32_t AudioId)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};

    SysReqAttr.s_Type = TYPE_INTVALUE;
    SysReqAttr.s_Attr = (void_t*)&AudioId;
    SysReqAttr.s_AttrLength = sizeof(AudioId);
    AudioId = ntohl(AudioId);

    do
    {
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_STOP_AUDIO_DECODE_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("stop audio decode fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysGetAudioEncodeCfg(uint16_t SessionId, uint32_t AuthValue, SysPkgAudioEncodeCfg *SysAudioEncodeCfgPtr)
{
    uint8_t    i;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;

    if (SysAudioEncodeCfgPtr == NULL )
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_AUDIO_ENCODE_REQ, &RspAttrCnt, &SysRspAttrPtr);
        if (FAILED(Result))
        {
            break;
        }
        if (RspAttrCnt <= 0
                || SysRspAttrPtr == NULL)
        {
            break;
        }

        SysRspAttrTmpPtr = SysRspAttrPtr;
        for (i = 0; i < RspAttrCnt; i++)
        {
            if (SysRspAttrTmpPtr->s_Type == TYPE_AUDIO_ENCODECFG
                    && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgAudioEncodeCfg))
            {
                memcpy(SysAudioEncodeCfgPtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysAudioEncodeCfgPtr->s_AudioId       = ntohl(SysAudioEncodeCfgPtr->s_AudioId);
                SysAudioEncodeCfgPtr->s_SamplesPerSec = ntohl(SysAudioEncodeCfgPtr->s_SamplesPerSec);
                SysAudioEncodeCfgPtr->s_CapVolume     = ntohs(SysAudioEncodeCfgPtr->s_CapVolume);
                SysAudioEncodeCfgPtr->s_PlayVolume    = ntohs(SysAudioEncodeCfgPtr->s_PlayVolume);
                SysAudioEncodeCfgPtr->s_AecDelayTime  = ntohs(SysAudioEncodeCfgPtr->s_AecDelayTime);
                SysAudioEncodeCfgPtr->s_BitRate       = ntohs(SysAudioEncodeCfgPtr->s_BitRate);
                SysAudioEncodeCfgPtr->s_FrameRate     = ntohs(SysAudioEncodeCfgPtr->s_FrameRate);
                Exist = true;
                break;
            }
            SysRspAttrTmpPtr++;
        }

        if (!Exist)
        {
            SYS_CLIENT_ERROR("not found valid data\n");
            break;
        }

        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
    }
    while(0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetAudioEncodeCfg(uint16_t SessionId, uint32_t AuthValue, SysPkgAudioEncodeCfg *SysAudioEncodeCfgPtr)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgAudioEncodeCfg SysAudioEncodeCfg;

    memset(&SysAudioEncodeCfg, 0, sizeof(SysPkgAudioEncodeCfg));
    memcpy(&SysAudioEncodeCfg, SysAudioEncodeCfgPtr, sizeof(SysPkgAudioEncodeCfg));

    SysReqAttr.s_Type = TYPE_AUDIO_ENCODECFG;
    SysReqAttr.s_Attr = (void_t*)&SysAudioEncodeCfg;
    SysReqAttr.s_AttrLength = sizeof(SysPkgAudioEncodeCfg);
    SysAudioEncodeCfg.s_AudioId       = htonl(SysAudioEncodeCfg.s_AudioId);
    SysAudioEncodeCfg.s_SamplesPerSec = htonl(SysAudioEncodeCfg.s_SamplesPerSec);
    SysAudioEncodeCfg.s_CapVolume     = htons(SysAudioEncodeCfg.s_CapVolume);
    SysAudioEncodeCfg.s_PlayVolume    = htons(SysAudioEncodeCfg.s_PlayVolume);
    SysAudioEncodeCfg.s_AecDelayTime  = htons(SysAudioEncodeCfg.s_AecDelayTime);
    SysAudioEncodeCfg.s_BitRate       = htons(SysAudioEncodeCfg.s_BitRate);
    SysAudioEncodeCfg.s_FrameRate     = htons(SysAudioEncodeCfg.s_FrameRate);

    do
    {
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_AUDIO_ENCODE_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("start audio decode fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}


GMI_RESULT SysGetAudioEncCfg(uint16_t SessionId, uint32_t AuthValue, uint32_t *AudioId, uint8_t *EncodeType, uint16_t *CapVolume, uint16_t *PlayVolume)
{
    SysPkgAudioEncodeCfg SysAudioEncCfg;

    GMI_RESULT Result = SysGetAudioEncodeCfg(SessionId, AuthValue, &SysAudioEncCfg);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("get audio encode config fail, Result = 0x%lx\n", Result);
        return Result;
    }

    *AudioId    = SysAudioEncCfg.s_AudioId;
    *EncodeType = SysAudioEncCfg.s_EncodeType;
    *CapVolume  = SysAudioEncCfg.s_CapVolume;
    *PlayVolume = SysAudioEncCfg.s_PlayVolume;

    return GMI_SUCCESS;
}


GMI_RESULT SysSetAudioEncCfg(uint16_t SessionId, uint32_t AuthValue, uint32_t AudioId, uint8_t EncodeType, uint16_t CapVolume, uint16_t PlayVolume)
{
    SysPkgAudioEncodeCfg SysAudioEncCfg;

    GMI_RESULT Result = SysGetAudioEncodeCfg(SessionId, AuthValue, &SysAudioEncCfg);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("get audio encode config fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SysAudioEncCfg.s_AudioId    = AudioId;
    SysAudioEncCfg.s_EncodeType = EncodeType;
    SysAudioEncCfg.s_CapVolume  = CapVolume;
    SysAudioEncCfg.s_PlayVolume = PlayVolume;

    Result = SysSetAudioEncodeCfg(SessionId, AuthValue, &SysAudioEncCfg);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("set audio encode config fail, Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT SysGetVideoEncStreamCombine(uint16_t SessionId, uint32_t AuthValue, SysPkgEncStreamCombine *SysEncStreamCombinePtr)
{
    uint8_t    i;
    boolean_t  Exist            = false;
    GMI_RESULT Result           = GMI_SUCCESS;
    uint16_t   RspAttrCnt       = 0;
    SysAttr   *SysRspAttrPtr    = NULL;
    SysAttr   *SysRspAttrTmpPtr = NULL;


    if (NULL == SysEncStreamCombinePtr)
    {
        return GMI_INVALID_PARAMETER;
    }

    do
    {
        Result = SysGetCmdExcute(SessionId, AuthValue, SYSCODE_GET_ENCSTREAM_COMBINE_REQ, &RspAttrCnt, &SysRspAttrPtr);
        if (FAILED(Result))
        {
            break;
        }
        if (RspAttrCnt <= 0
                || SysRspAttrPtr == NULL)
        {
            SYS_CLIENT_ERROR("RspAttrCnt %d, or SysRspAttrPtr is null\n", RspAttrCnt);
            break;
        }

        SysRspAttrTmpPtr = SysRspAttrPtr;
        for (i = 0; i < RspAttrCnt; i++)
        {
            if (SysRspAttrTmpPtr->s_Type == TYPE_ENCSTREAM_COMBINE
                    && SysRspAttrTmpPtr->s_AttrLength == sizeof(SysPkgEncStreamCombine))
            {
                memcpy(SysEncStreamCombinePtr, SysRspAttrTmpPtr->s_Attr, SysRspAttrTmpPtr->s_AttrLength);
                SysEncStreamCombinePtr->s_VideoId = ntohl(SysEncStreamCombinePtr->s_VideoId);
                SysEncStreamCombinePtr->s_EnableStreamNum = ntohl(SysEncStreamCombinePtr->s_EnableStreamNum);
                SysEncStreamCombinePtr->s_StreamCombineNo = ntohl(SysEncStreamCombinePtr->s_StreamCombineNo);
                Exist = true;
                break;
            }
            SysRspAttrTmpPtr++;
        }

        if (!Exist)
        {
            break;
        }

        SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
        return GMI_SUCCESS;
    }
    while(0);

    SysGetCmdAttrFree(RspAttrCnt, SysRspAttrPtr);
    return GMI_FAIL;
}


GMI_RESULT SysSetVideoEncStreamCombine(uint16_t SessionId, uint32_t AuthValue, SysPkgEncStreamCombine *SysEncStreamCombinePtr)
{
    GMI_RESULT Result     = GMI_SUCCESS;
    uint16_t   ReqAttrCnt = 1;
    SysAttr    SysReqAttr = {0};
    SysPkgEncStreamCombine SysEncStreamCombine;

    memset(&SysEncStreamCombine, 0, sizeof(SysPkgEncStreamCombine));
    memcpy(&SysEncStreamCombine, SysEncStreamCombinePtr, sizeof(SysPkgEncStreamCombine));

    SysReqAttr.s_Type = TYPE_ENCSTREAM_COMBINE;
    SysReqAttr.s_Attr = (void_t*)&SysEncStreamCombine;
    SysReqAttr.s_AttrLength = sizeof(SysPkgEncStreamCombine);
    SysEncStreamCombine.s_VideoId = htonl(SysEncStreamCombine.s_VideoId);
    SysEncStreamCombine.s_EnableStreamNum = htonl(SysEncStreamCombine.s_EnableStreamNum);
    SysEncStreamCombine.s_StreamCombineNo = htonl(SysEncStreamCombine.s_StreamCombineNo);

    do
    {
        Result = SysSetCmdExcute(SessionId, AuthValue, SYSCODE_SET_ENCSTREAM_COMBINE_REQ, ReqAttrCnt, &SysReqAttr);
        if (FAILED(Result))
        {
            SYS_CLIENT_ERROR("SYSCODE_SET_ENCSTREAM_COMBINE_REQ fail, Result = 0x%lx\n", Result);
            break;
        }

        return GMI_SUCCESS;
    }
    while(0);

    return GMI_FAIL;
}

