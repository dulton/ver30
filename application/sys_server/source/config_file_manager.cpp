#include <string.h>
#include "config_file_manager.h"
#include "log.h"
#include "factory_setting_operation.h"
#include "gmi_system_headers.h"
#include "gmi_config_api.h"

//OSD Font Size Map Table
static int32_t l_OSD_FontSize[][2] = 
{
	{RES_1080P_HEIGHT, OSD_FONT_SIZE_BIGNESS},
	{RES_720P_HEIGHT, OSD_FONT_SIZE_BIG},
	{RES_D1_PAL_HEIGHT, OSD_FONT_SIZE_MEDIUM},
	{RES_D1_NTSC_HEIGHT, OSD_FONT_SIZE_MEDIUM},
	{RES_CIF_PAL_HEIGHT, OSD_FONT_SIZE_SMALL},
	{RES_CIF_NTSC_HEIGHT, OSD_FONT_SIZE_SMALL},
	{RES_QCIF_PAL_HEIGHT, OSD_FONT_SIZE_SMALLNESS},
	{RES_QVGA_HEIGHT, OSD_FONT_SIZE_SMALLNESS}
};

ConfigFileManager::ConfigFileManager()
{
}


ConfigFileManager::~ConfigFileManager()
{
}


GMI_RESULT ConfigFileManager::Initialize()
{
    memcpy(m_FactorySettingFile, GMI_FACTORY_SETTING_CONFIG_FILE_NAME, sizeof(m_FactorySettingFile));
    memcpy(m_DefaultSettingFile, GMI_DEFAULT_SETTING_CONFIG_FILE_NAME, sizeof(m_DefaultSettingFile));
    memcpy(m_SettingFile, GMI_SETTING_CONFIG_FILE_NAME, sizeof(m_SettingFile));
    memcpy(m_ResourceFile, GMI_RESOURCE_CONFIG_FILE_NAME, sizeof(m_ResourceFile));
    memcpy(m_PresetsFile, GMI_PTZ_PRESETS_PATROLS_FILE_NAME, sizeof(m_PresetsFile));    
    GMI_RESULT Result = m_Mutex.Create(NULL);
    if (FAILED(Result))
    {
        SYS_ERROR("m_Mutex.Create error, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_Mutex.Create error\n");
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::Deinitialize()
{
    m_Mutex.Destroy();
    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::Lock()
{
    m_Mutex.Lock(TIMEOUT_INFINITE);
    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::Unlock()
{
    m_Mutex.Unlock();
    return GMI_SUCCESS;
}

GMI_RESULT ConfigFileManager::GetVideoEncodeStreamNum(int32_t * StreamNum)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    Result = GMI_XmlRead(Handle, VIDEO_ENCODE_PATH, VIDEO_ENCODE_STREAM_NUM_KEY, VIDEO_ENCODE_STREAM_NUM, StreamNum, GMI_CONFIG_READ_WRITE);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetVideoEncodeStreamNum(int32_t StreamNum)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    Result = GMI_XmlWrite(Handle, VIDEO_ENCODE_PATH, VIDEO_ENCODE_STREAM_NUM_KEY, StreamNum);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetStreamCombine(SysPkgEncStreamCombine *SysEncStreamCombinePtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    int32_t    StreamNum;

    Result = GetVideoEncodeStreamNum(&StreamNum);
    if (FAILED(Result))
    {
        return Result;
    }

    int32_t MaxStreamNum;
    int32_t MaxPicWidth;
    int32_t MaxPicHeight;
    FactorySettingOperation FactoryOperation;
    Result = FactoryOperation.GetVideoMaxCapability(&MaxStreamNum, &MaxPicWidth, &MaxPicHeight);
    if (FAILED(Result))
    {
        return Result;
    }

    //get default combineno, according to the capability of MaxPicHeight;
    int32_t DefaultCombineNo;
    switch (MaxPicHeight)
    {
    case RES_1080P_HEIGHT:
        DefaultCombineNo = VIDEO_ENCODE_1080P_TWOSTREAM_COMBINE_NO;
        break;
    case RES_720P_HEIGHT:
        DefaultCombineNo = VIDEO_ENCODE_720P_TWOSTREAM_COMBINE_NO;
        break;
    default:
        return GMI_NOT_SUPPORT;
    }

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    int32_t CombineNo;
    Result = GMI_XmlRead(Handle, VIDEO_ENCODE_PATH, VIDEO_ENCODE_COMBINE_NO_KEY, (const int32_t)DefaultCombineNo, &CombineNo, GMI_CONFIG_READ_WRITE);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    SysEncStreamCombinePtr->s_VideoId = 1;
    SysEncStreamCombinePtr->s_EnableStreamNum = StreamNum;
    SysEncStreamCombinePtr->s_StreamCombineNo = CombineNo;

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetSysEncStreamCombineDefault(SysPkgEncStreamCombine *SysEncStreamCombinePtr)
{
    int32_t MaxStreamNum;
    int32_t MaxPicWidth;
    int32_t MaxPicHeight;
    FactorySettingOperation FactoryOperation;
    GMI_RESULT Result = FactoryOperation.GetVideoMaxCapability(&MaxStreamNum, &MaxPicWidth, &MaxPicHeight);
    if (FAILED(Result))
    {
        return Result;
    }

    SysEncStreamCombinePtr->s_VideoId = 1;
    SysEncStreamCombinePtr->s_EnableStreamNum = VIDEO_ENCODE_STREAM_NUM;
    int32_t DefaultCombineNo;
    switch (MaxPicHeight)
    {
    case RES_1080P_HEIGHT:
        DefaultCombineNo = VIDEO_ENCODE_1080P_TWOSTREAM_COMBINE_NO;
        break;
    case RES_720P_HEIGHT:
        DefaultCombineNo = VIDEO_ENCODE_720P_TWOSTREAM_COMBINE_NO;
        break;
    default:
        return GMI_NOT_SUPPORT;
    }
    SysEncStreamCombinePtr->s_StreamCombineNo = DefaultCombineNo;
    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetStreamCombine(SysPkgEncStreamCombine *SysEncStreamCombinePtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Result = GMI_XmlWrite(Handle, VIDEO_ENCODE_PATH, VIDEO_ENCODE_COMBINE_NO_KEY, (const int32_t)(SysEncStreamCombinePtr->s_StreamCombineNo));
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetBitrates(int32_t Width, int32_t Height, int32_t *BitRateAverage, int32_t *BitRateUp, int32_t *BitRateDown)
{
    if (Width == RES_1080P_WIDTH
            && Height == RES_1080P_HEIGHT)
    {
        *BitRateAverage = VIDEO_ENCODE_1080P_STREAM_BITRATE;
        *BitRateUp = VIDEO_ENCODE_1080P_STREAM_BITRATE_UP;
        *BitRateDown = VIDEO_ENCODE_1080P_STREAM_BITRATE_DOWN;
    }
    else if (Width == RES_720P_WIDTH
             && Height == RES_720P_HEIGHT)
    {
        *BitRateAverage = VIDEO_ENCODE_720P_STREAM_BITRATE;
        *BitRateUp = VIDEO_ENCODE_720P_STREAM_BITRATE_UP;
        *BitRateDown = VIDEO_ENCODE_720P_STREAM_BITRATE_DOWN;
    }
    else if ((Width == RES_D1_PAL_WIDTH
              && Height == RES_D1_PAL_HEIGHT)
             ||(Width == RES_D1_NTSC_WIDTH
                && Height == RES_D1_NTSC_HEIGHT))
    {
        *BitRateAverage = VIDEO_ENCODE_576P_STREAM_BITRATE;
        *BitRateUp = VIDEO_ENCODE_576P_STREAM_BITRATE_UP;
        *BitRateDown = VIDEO_ENCODE_576P_STREAM_BITRATE_DOWN;
    }
    else if ((Width == RES_CIF_PAL_WIDTH
              && Height == RES_CIF_PAL_HEIGHT)
             || (Width == RES_CIF_NTSC_WIDTH
                 && Height == RES_CIF_NTSC_HEIGHT))
    {
        *BitRateAverage = VIDEO_ENCODE_CIF_STREAM_BITRATE;
        *BitRateUp = VIDEO_ENCODE_CIF_STREAM_BITRATE_UP;
        *BitRateDown = VIDEO_ENCODE_CIF_STREAM_BITRATE_DOWN;
    }
    else if ((Width == RES_QCIF_PAL_WIDTH
              && Height == RES_QCIF_PAL_HEIGHT)
             || (Width == RES_QVGA_WIDTH
                 && Height == RES_QVGA_HEIGHT))
    {
        *BitRateAverage = VIDEO_ENCODE_QCIF_STREAM_BITRATE;
        *BitRateUp = VIDEO_ENCODE_QCIF_STREAM_BITRATE_UP;
        *BitRateDown = VIDEO_ENCODE_QCIF_STREAM_BITRATE_DOWN;
    }
    else
    {
        SYS_ERROR("the device not support Width %d x Height %d\n", Width, Height);
        return GMI_NOT_SUPPORT;
    }

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetVideoEncodeSettings(int32_t StreamId, SysPkgEncStreamCombine *SysEncStreamCombine, VideoEncodeParam *VidEncParam)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    int32_t    Id;
    int32_t    StreamNum;
    int32_t    CombineNo;
    char_t     StreamXPath[128] = {0};

    StreamNum = SysEncStreamCombine->s_EnableStreamNum;
    CombineNo = SysEncStreamCombine->s_StreamCombineNo;

    //first col:width, second col:height
    int32_t Res[16][4];
    memset(Res, 0, sizeof(Res));
    FactorySettingOperation FactoryOperation;
    Result = FactoryOperation.GetVideoParams(StreamNum, CombineNo, Res);
    if (FAILED(Result))
    {
        SYS_ERROR("get video params from capability fail , Result=0x%lx\n", Result);
        return Result;
    }

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    if (StreamId == 0xff)
    {
        for (Id = 0; Id < StreamNum; Id++)
        {
            VidEncParam[Id].s_StreamId = Id;
            memset(StreamXPath, 0, sizeof(StreamXPath));
            sprintf(StreamXPath, VIDEO_ENCODE_STREAM_PATH, Id);
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_CODEC_KEY,               VIDEO_ENCODE_CODEC,                &VidEncParam[Id].s_EncodeType,    GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_WIDTH_KEY,        Res[Id][0],                        &VidEncParam[Id].s_EncodeWidth,   GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_HEIGHT_KEY,       Res[Id][1],                        &VidEncParam[Id].s_EncodeHeight,  GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_FRAME_RATE_KEY,   Res[Id][2],                        &VidEncParam[Id].s_FrameRate,     GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_BITRATE_TYPE_KEY, VIDEO_ENCODE_STREAM_BITRATE_TYPE,  &VidEncParam[Id].s_BitRateType,   GMI_CONFIG_READ_WRITE);
            int32_t BitRate;
            int32_t BitRateUp;
            int32_t BitRateDown;
            Result = GetBitrates(Res[Id][0], Res[Id][1], &BitRate, &BitRateUp, &BitRateDown);
            if (FAILED(Result))
            {
                GMI_XmlFileSave(Handle);
                Unlock();
                return Result;
            }
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_BITRATE_KEY,       BitRate,                         (int32_t*)&VidEncParam[Id].s_BitRateAverage,GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_BITRATEUP_KEY,     BitRateUp,                       (int32_t*)&VidEncParam[Id].s_BitRateUp,     GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_BITRATEDOWN_KEY,   BitRateDown,                     (int32_t*)&VidEncParam[Id].s_BitRateDown,   GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_GOP_KEY,           VIDEO_ENCODE_STREAM_GOP,          &VidEncParam[Id].s_FrameInterval, GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_QUALITY_KEY,       VIDEO_ENCODE_STREAM_QUALITY,      &VidEncParam[Id].s_EncodeQulity,  GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_ROTATE_KEY,        VIDEO_ENCODE_STREAM_ROTATE,       &VidEncParam[Id].s_Rotate,        GMI_CONFIG_READ_WRITE);            
        }

        if (FAILED(Result))
        {
            GMI_XmlFileSave(Handle);
            Unlock();
            return Result;
        }
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


static int32_t FloatToInt(float_t Value)
{
    if (Value > 0.0)
    {
        return (int32_t)(Value + 0.5);
    }
    else
    {
        return (int32_t)(Value - 0.5);
    }
}


GMI_RESULT ConfigFileManager::GetVideoEncodeSettingsDefault(int32_t StreamId, SysPkgEncStreamCombine *SysEncStreamCombine, SysPkgEncodeCfg *SysEncodeCfgPtr)
{
    GMI_RESULT Result;
    int32_t    Id;
    int32_t    StreamNum;
    int32_t    CombineNo;

    StreamNum = SysEncStreamCombine->s_EnableStreamNum;
    CombineNo = SysEncStreamCombine->s_StreamCombineNo;

    //first col:width, second col:height
    int32_t Res[16][4];
    memset(Res, 0, sizeof(Res));
    FactorySettingOperation FactoryOperation;
    Result = FactoryOperation.GetVideoParams(StreamNum, CombineNo, Res);
    if (FAILED(Result))
    {
        SYS_ERROR("get video params from capability fail , Result=0x%lx\n", Result);
        return Result;
    }

    for (Id = 0; Id < StreamNum; Id++)
    {
        SysEncodeCfgPtr[Id].s_VideoId   = 1;
        SysEncodeCfgPtr[Id].s_Flag      = Id;
        SysEncodeCfgPtr[Id].s_PicWidth  = Res[Id][0];
        SysEncodeCfgPtr[Id].s_PicHeight = Res[Id][1];
        SysEncodeCfgPtr[Id].s_FPS       = Res[Id][2];
        int32_t BitRate;
        int32_t BitRateUp;
        int32_t BitRateDown;
        Result = GetBitrates(Res[Id][0], Res[Id][1], &BitRate, &BitRateUp, &BitRateDown);
        if (FAILED(Result))
        {
            return Result;
        }
        SysEncodeCfgPtr[Id].s_BitRateAverage = BitRate;
        SysEncodeCfgPtr[Id].s_BitRateUp      = BitRateUp;
        SysEncodeCfgPtr[Id].s_BitRateDown    = BitRateDown;

        uint16_t EncodeType = VIDEO_ENCODE_CODEC;
        switch (EncodeType)
        {
        case 1://h264
            SysEncodeCfgPtr[Id].s_Compression = SYS_COMP_H264;
            break;
        case 2://mjpeg
            SysEncodeCfgPtr[Id].s_Compression = SYS_COMP_MJPEG;
            break;
        default:
            SYS_ERROR("EncodeType %d\n", EncodeType);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "EncodeType %d\n", EncodeType);
            return GMI_INVALID_PARAMETER;
        }

        uint16_t BitRateType = VIDEO_ENCODE_STREAM_BITRATE_TYPE;
        switch (BitRateType)
        {
        case 1://cbr
            SysEncodeCfgPtr[Id].s_BitrateCtrl = SYS_BRC_CBR;
            break;
        case 2://vbr
            SysEncodeCfgPtr[Id].s_BitrateCtrl = SYS_BRC_VBR;
            break;
        default:
            SYS_ERROR("BitRateType %d\n", BitRateType);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "BitRateType %d\n", BitRateType);
            return GMI_INVALID_PARAMETER;
        }

        SysEncodeCfgPtr[Id].s_StreamType = VIDEO_ENCODE_STREAM_TYPE;
        SysEncodeCfgPtr[Id].s_Gop        = VIDEO_ENCODE_STREAM_GOP;
        SysEncodeCfgPtr[Id].s_Rotate     = VIDEO_ENCODE_STREAM_ROTATE;
        SysEncodeCfgPtr[Id].s_Quality    =  FloatToInt((float_t)(VIDEO_ENCODE_STREAM_QUALITY * 6) / 100);
    }

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetVideoEncodeSettingsDefault(int32_t StreamId, SysPkgEncStreamCombine *SysEncStreamCombine, VideoEncodeParam *VidEncParam)
{
    GMI_RESULT Result;
    int32_t    Id;
    int32_t    StreamNum;
    int32_t    CombineNo;

    StreamNum = SysEncStreamCombine->s_EnableStreamNum;
    CombineNo = SysEncStreamCombine->s_StreamCombineNo;

    //first col:width, second col:height
    int32_t Res[16][4];
    memset(Res, 0, sizeof(Res));
    FactorySettingOperation FactoryOperation;
    Result = FactoryOperation.GetVideoParams(StreamNum, CombineNo, Res);
    if (FAILED(Result))
    {
        SYS_ERROR("get video params from capability fail , Result=0x%lx\n", Result);
        return Result;
    }

    for (Id = 0; Id < StreamNum; Id++)
    {
        VidEncParam[Id].s_StreamId    = Id;
        VidEncParam[Id].s_EncodeWidth = Res[Id][0]; //width
        VidEncParam[Id].s_EncodeHeight= Res[Id][1]; //height
        VidEncParam[Id].s_FrameRate   = Res[Id][2]; //framerate
        int32_t BitRate;
        int32_t BitRateUp;
        int32_t BitRateDown;
        Result = GetBitrates(Res[Id][0], Res[Id][1], &BitRate, &BitRateUp, &BitRateDown);
        if (FAILED(Result))
        {
            return Result;
        }
        VidEncParam[Id].s_BitRateAverage = BitRate;
        VidEncParam[Id].s_BitRateUp      = BitRateUp;
        VidEncParam[Id].s_BitRateDown    = BitRateDown;
        VidEncParam[Id].s_EncodeType     = VIDEO_ENCODE_CODEC;
        VidEncParam[Id].s_BitRateType    = VIDEO_ENCODE_STREAM_BITRATE_TYPE;
        VidEncParam[Id].s_FrameInterval  = VIDEO_ENCODE_STREAM_GOP;
        VidEncParam[Id].s_Rotate         = VIDEO_ENCODE_STREAM_ROTATE;
        VidEncParam[Id].s_EncodeQulity   = VIDEO_ENCODE_STREAM_QUALITY;
    }

    return GMI_SUCCESS;
}



GMI_RESULT ConfigFileManager::SetVideoEncodeSetting(int32_t StreamId, VideoEncodeParam * VidEncParam)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    //int32_t    StreamNum;
    char_t     StreamXPath[128] = {0};

    //Result = GetStreamNum(&StreamNum);
    //if (FAILED(Result))
    //{
    //	return Result;
    //}

    //if (StreamId > StreamNum - 1)
    //{
    //	return GMI_FAIL;
    //}

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(StreamXPath, 0, sizeof(StreamXPath));
    sprintf(StreamXPath, VIDEO_ENCODE_STREAM_PATH, StreamId);
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_CODEC_KEY,              (const uint16_t)(VidEncParam->s_EncodeType));
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_WIDTH_KEY,       (const uint16_t)(VidEncParam->s_EncodeWidth));
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_HEIGHT_KEY,      (const uint16_t)(VidEncParam->s_EncodeHeight));
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_BITRATE_TYPE_KEY,(const uint16_t)(VidEncParam->s_BitRateType));
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_BITRATE_KEY,     (const int32_t)(VidEncParam->s_BitRateAverage));
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_BITRATEUP_KEY,   (const int32_t)(VidEncParam->s_BitRateUp));
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_BITRATEDOWN_KEY, (const int32_t)(VidEncParam->s_BitRateDown));
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_FRAME_RATE_KEY,  (const uint16_t)(VidEncParam->s_FrameRate));
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_GOP_KEY,         (const uint16_t)(VidEncParam->s_FrameInterval));
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_QUALITY_KEY,     (const uint16_t)(VidEncParam->s_EncodeQulity));
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_ROTATE_KEY,      (const uint8_t)(VidEncParam->s_Rotate));

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetVideoStreamType(int32_t StreamId, int32_t *StreamTypePtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     StreamXPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(StreamXPath, 0, sizeof(StreamXPath));
    sprintf(StreamXPath, VIDEO_ENCODE_STREAM_PATH, StreamId);
    Result = GMI_XmlRead(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_TYPE_KEY, VIDEO_ENCODE_STREAM_TYPE, StreamTypePtr, GMI_CONFIG_READ_WRITE);

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetVideoStreamType(int32_t StreamId, int32_t StreamType)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     StreamXPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(StreamXPath, 0, sizeof(StreamXPath));
    sprintf(StreamXPath, VIDEO_ENCODE_STREAM_PATH, StreamId);
    Result = GMI_XmlWrite(Handle, (const char_t*)StreamXPath, VIDEO_ENCODE_STREAM_TYPE_KEY, (const int32_t)StreamType);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetOsdTextNum(int32_t StreamId, int32_t *TextNum)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t	   OsdXPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(OsdXPath, 0, sizeof(OsdXPath));
    sprintf(OsdXPath, OSD_PATH, StreamId);

    Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_NUM_KEY, OSD_DEFAULT_TEXT_NUM, TextNum, GMI_CONFIG_READ_WRITE);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetOsdFontSizeDefault(int32_t StreamId, int32_t *FontSize)
{	
	SysPkgEncStreamCombine SysEncStreamCombine;	
	GMI_RESULT Result = GetStreamCombine(&SysEncStreamCombine);
	if (FAILED(Result))
	{
		return Result;
	}		
	
	if (0xff == StreamId)
	{		
		ReferrencePtr<VideoEncodeParam, DefaultObjectsDeleter> VidEncParamPtr(BaseMemoryManager::Instance().News<VideoEncodeParam>(SysEncStreamCombine.s_EnableStreamNum));
		if (NULL == VidEncParamPtr.GetPtr())
		{
			return GMI_OUT_OF_MEMORY;
		}
		
		Result = GetVideoEncodeSettings(StreamId, &SysEncStreamCombine, VidEncParamPtr.GetPtr());
		if (FAILED(Result))
		{
			return Result;
		}

		for (int32_t Id = 0; Id < SysEncStreamCombine.s_EnableStreamNum; Id++)
		{
			uint32_t Row;
			for (Row = 0; Row < sizeof(l_OSD_FontSize)/(sizeof(l_OSD_FontSize[0][0])*2); Row++)
			{
			    if (VidEncParamPtr.GetPtr()[Id].s_EncodeHeight == l_OSD_FontSize[Row][0])
			    {
			        FontSize[Id] = l_OSD_FontSize[Row][1];
			        break;
			    }
			}

			if (Row >= sizeof(l_OSD_FontSize)/(sizeof(l_OSD_FontSize[0][0])*2))
			{
				FontSize[Id] = OSD_FONT_SIZE_MEDIUM;
			}
		}
	}	

	return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetOsdSettings(int32_t StreamId, int32_t StreamNum, VideoOSDParam *OsdParamPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    int32_t    Id;
    int32_t    TextId;
    int32_t    TextNum[16];
    char_t     OsdXPath[128] = {0};
    ReferrencePtr<int32_t, DefaultObjectsDeleter> FontSize;

    if (StreamId == 0xff)
    {
        for (Id = 0; Id < StreamNum; Id++)
        {
            Result = GetOsdTextNum(Id, &TextNum[Id]);
            if (FAILED(Result))
            {
                return Result;
            }
        }

		FontSize = BaseMemoryManager::Instance().News<int32_t>(StreamNum);
		if (NULL == FontSize.GetPtr())
		{
			return GMI_OUT_OF_MEMORY;
		}
		//get font size according to resolution   
        Result = GetOsdFontSizeDefault(StreamId, FontSize.GetPtr());
        if (FAILED(Result))
        {
        	memset(FontSize.GetPtr(), OSD_FONT_SIZE_MEDIUM, sizeof(int32_t)*StreamNum);
        }
    }
    
    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    if (StreamId == 0xff)
    {
        for (Id = 0; Id < StreamNum; Id++)
        {        	     	        	
            OsdParamPtr[Id].s_StreamId = Id;
            memset(OsdXPath, 0, sizeof(OsdXPath));
            sprintf(OsdXPath, OSD_PATH, Id);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_DISPLAY_TYPE_KEY, OSD_DISPLAY_TYPE,    &(OsdParamPtr[Id].s_DisplayType), GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_LANGUAGE_KEY,  OSD_LANGUAGE,           (int32_t*)&(OsdParamPtr[Id].s_Language), GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_ENABLE_KEY,  OSD_ENABLE,               (int32_t*)&(OsdParamPtr[Id].s_Flag), GMI_CONFIG_READ_WRITE);
            memset(OsdXPath, 0, sizeof(OsdXPath));
            sprintf(OsdXPath, OSD_TIME_PATH, Id);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_ENABLE_KEY,    OSD_TIME_ENABLE,       &(OsdParamPtr[Id].s_TimeDisplay.s_Flag),      GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_DATE_STYLE_KEY, OSD_TIME_DATE_STYLE,   &(OsdParamPtr[Id].s_TimeDisplay.s_DateStyle), GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_STYLE_KEY, OSD_TIME_STYLE,        &(OsdParamPtr[Id].s_TimeDisplay.s_TimeStyle), GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_FONT_COLOR_KEY, OSD_TIME_FONT_COLOR,   &(OsdParamPtr[Id].s_TimeDisplay.s_FontColor), GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_FONT_STYLE_KEY, FontSize.GetPtr()[Id],   &(OsdParamPtr[Id].s_TimeDisplay.s_FontStyle), GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_FONT_BLOD_KEY,  OSD_TIME_FONT_BLOD,    &(OsdParamPtr[Id].s_TimeDisplay.s_FontBlod),  GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_ROTATE_KEY,    OSD_TIME_ROTATE,       &(OsdParamPtr[Id].s_TimeDisplay.s_Rotate),    GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_ITALIC_KEY,    OSD_TIME_ITALIC,       &(OsdParamPtr[Id].s_TimeDisplay.s_Italic),    GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_OUTLINE_KEY,   OSD_TIME_OUTLINE,      &(OsdParamPtr[Id].s_TimeDisplay.s_Outline),   GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_DISPLAY_X_KEY,  OSD_TIME_DISPLAY_X,    &(OsdParamPtr[Id].s_TimeDisplay.s_DisplayX),  GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_DISPLAY_Y_KEY,  OSD_TIME_DISPLAY_Y,    &(OsdParamPtr[Id].s_TimeDisplay.s_DisplayY),  GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_DISPLAY_H_KEY,  OSD_TIME_DISPLAY_H,    &(OsdParamPtr[Id].s_TimeDisplay.s_DisplayH),  GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TIME_DISPLAY_W_KEY,  OSD_TIME_DISPLAY_W,    &(OsdParamPtr[Id].s_TimeDisplay.s_DisplayW),  GMI_CONFIG_READ_WRITE);

            for (TextId = 0; TextId < TextNum[Id]; TextId++)
            {
                memset(OsdXPath, 0, sizeof(OsdXPath));
                sprintf(OsdXPath, OSD_TEXT_PATH, Id, TextId);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_ENABLE_KEY,     OSD_TEXT_ENABLE,       &(OsdParamPtr[Id].s_TextDisplay[TextId].s_Flag),      GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_FONT_COLOR_KEY, OSD_TEXT_FONT_COLOR,   &(OsdParamPtr[Id].s_TextDisplay[TextId].s_FontColor), GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_FONT_STYLE_KEY, FontSize.GetPtr()[Id],   &(OsdParamPtr[Id].s_TextDisplay[TextId].s_FontStyle), GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_FONT_BLOD_KEY,  OSD_TEXT_FONT_BLOD,    &(OsdParamPtr[Id].s_TextDisplay[TextId].s_FontBlod),  GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_ROTATE_KEY,     OSD_TEXT_ROTATE,       &(OsdParamPtr[Id].s_TextDisplay[TextId].s_Rotate),    GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_ITALIC_KEY,     OSD_TEXT_ITALIC,       &(OsdParamPtr[Id].s_TextDisplay[TextId].s_Italic),    GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_OUTLINE_KEY,    OSD_TEXT_OUTLINE,      &(OsdParamPtr[Id].s_TextDisplay[TextId].s_Outline),   GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_DISPLAY_X_KEY,  OSD_TEXT_DISPLAY_X,    &(OsdParamPtr[Id].s_TextDisplay[TextId].s_DisplayX),  GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_DISPLAY_Y_KEY,  OSD_TEXT_DISPLAY_Y,    &(OsdParamPtr[Id].s_TextDisplay[TextId].s_DisplayY),  GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_DISPLAY_H_KEY,  OSD_TEXT_DISPLAY_H,    &(OsdParamPtr[Id].s_TextDisplay[TextId].s_DisplayH),  GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_DISPLAY_W_KEY,  OSD_TEXT_DISPLAY_W,    &(OsdParamPtr[Id].s_TextDisplay[TextId].s_DisplayW),  GMI_CONFIG_READ_WRITE);
                Result = GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_CONTENT_KEY,    OSD_TEXT_CONTENT,       (char_t*)&(OsdParamPtr[Id].s_TextDisplay[TextId].s_TextContent),  GMI_CONFIG_READ_WRITE);
                if (0 == strlen((const char_t*)OsdParamPtr[Id].s_TextDisplay[TextId].s_TextContent))
                {
                    SYS_ERROR("%s is null, so reset default value %s\n", OSD_TEXT_CONTENT_KEY, OSD_TEXT_CONTENT);
                    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "%s is null, so reset default value %s\n", OSD_TEXT_CONTENT_KEY, OSD_TEXT_CONTENT);
                    GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_CONTENT_KEY,  OSD_TEXT_CONTENT);
                    GMI_XmlRead(Handle, (const char_t*)OsdXPath, OSD_TEXT_CONTENT_KEY,   OSD_TEXT_CONTENT,  (char_t*)&(OsdParamPtr[Id].s_TextDisplay[TextId].s_TextContent),  GMI_CONFIG_READ_ONLY);
                    strcpy((char_t*)OsdParamPtr[Id].s_TextDisplay[TextId].s_TextContent, OSD_TEXT_CONTENT);
                }
                OsdParamPtr[Id].s_TextDisplay[TextId].s_TextContentLen = strlen((char_t*)OsdParamPtr[Id].s_TextDisplay[TextId].s_TextContent);
            }
        }

        if (FAILED(Result))
        {
            GMI_XmlFileSave(Handle);
            Unlock();
            return Result;
        }
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetOsdSettings(int32_t StreamId, VideoOSDParam *OsdParamPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    int32_t    TextId;
    int32_t    TextNum;
    char_t     OsdXPath[128] = {0};

    Result = GetOsdTextNum(StreamId, &TextNum);
    if (FAILED(Result))
    {
        return Result;
    }

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(OsdXPath, 0, sizeof(OsdXPath));
    sprintf(OsdXPath, OSD_PATH, StreamId);
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_DISPLAY_TYPE_KEY, (const uint16_t)(OsdParamPtr->s_DisplayType));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_LANGUAGE_KEY,     (const int32_t)(OsdParamPtr->s_Language));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_ENABLE_KEY,       (const int32_t)(OsdParamPtr->s_Flag));

    memset(OsdXPath, 0, sizeof(OsdXPath));
    sprintf(OsdXPath, OSD_TIME_PATH, StreamId);
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_ENABLE_KEY,      (const uint8_t)(OsdParamPtr->s_TimeDisplay.s_Flag));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_DATE_STYLE_KEY,  (const uint8_t)(OsdParamPtr->s_TimeDisplay.s_DateStyle));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_STYLE_KEY,       (const uint8_t)(OsdParamPtr->s_TimeDisplay.s_TimeStyle));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_FONT_COLOR_KEY,  (const uint8_t)(OsdParamPtr->s_TimeDisplay.s_FontColor));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_FONT_STYLE_KEY,  (const uint8_t)(OsdParamPtr->s_TimeDisplay.s_FontStyle));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_FONT_BLOD_KEY,   (const uint8_t)(OsdParamPtr->s_TimeDisplay.s_FontBlod));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_ROTATE_KEY,      (const uint8_t)(OsdParamPtr->s_TimeDisplay.s_Rotate));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_ITALIC_KEY,      (const uint8_t)(OsdParamPtr->s_TimeDisplay.s_Italic));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_OUTLINE_KEY,     (const uint8_t)(OsdParamPtr->s_TimeDisplay.s_Outline));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_DISPLAY_X_KEY,   (const uint16_t)(OsdParamPtr->s_TimeDisplay.s_DisplayX));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_DISPLAY_Y_KEY,   (const uint16_t)(OsdParamPtr->s_TimeDisplay.s_DisplayY));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_DISPLAY_H_KEY,   (const uint16_t)(OsdParamPtr->s_TimeDisplay.s_DisplayH));
    Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TIME_DISPLAY_W_KEY,   (const uint16_t)(OsdParamPtr->s_TimeDisplay.s_DisplayW));

    for (TextId = 0; TextId < TextNum; TextId++)
    {
        memset(OsdXPath, 0, sizeof(OsdXPath));
        sprintf(OsdXPath, OSD_TEXT_PATH, StreamId, TextId);
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_ENABLE_KEY,     (const uint8_t)(OsdParamPtr->s_TextDisplay[TextId].s_Flag));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_FONT_COLOR_KEY, (const uint8_t)(OsdParamPtr->s_TextDisplay[TextId].s_FontColor));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_FONT_STYLE_KEY, (const uint8_t)(OsdParamPtr->s_TextDisplay[TextId].s_FontStyle));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_FONT_BLOD_KEY,  (const uint8_t)(OsdParamPtr->s_TextDisplay[TextId].s_FontBlod));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_ROTATE_KEY,     (const uint8_t)(OsdParamPtr->s_TextDisplay[TextId].s_Rotate));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_ITALIC_KEY,     (const uint8_t)(OsdParamPtr->s_TextDisplay[TextId].s_Italic));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_OUTLINE_KEY,    (const uint8_t)(OsdParamPtr->s_TextDisplay[TextId].s_Outline));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_DISPLAY_X_KEY,  (const uint16_t)(OsdParamPtr->s_TextDisplay[TextId].s_DisplayX));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_DISPLAY_Y_KEY,  (const uint16_t)(OsdParamPtr->s_TextDisplay[TextId].s_DisplayY));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_DISPLAY_H_KEY,  (const uint16_t)(OsdParamPtr->s_TextDisplay[TextId].s_DisplayH));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_DISPLAY_W_KEY,  (const uint16_t)(OsdParamPtr->s_TextDisplay[TextId].s_DisplayW));
        Result = GMI_XmlWrite(Handle, (const char_t*)OsdXPath, OSD_TEXT_CONTENT_KEY,    (const char_t*)(OsdParamPtr->s_TextDisplay[TextId].s_TextContent));
    }

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetImageSettings(int32_t SourceId, int32_t ChanId, ImageBaseParam *ImageBaseParamPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     ImageXPath[128] = {0};

    boolean_t  GetImageFail = false;
    int32_t    Images[32];
    memset(Images, 0, sizeof(Images));
    FactorySettingOperation FactoryOperation;
    Result = FactoryOperation.GetImageParams(Images);
    if (FAILED(Result))
    {
        SYS_ERROR("get image params form capoperation fail, Result = 0x%lx\n", Result);
        GetImageFail = true;
    }

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(ImageXPath, 0, sizeof(ImageXPath));
    strcpy(ImageXPath, VIDEO_SOURCE_IMAGE_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_EXPOSURE_MODE_KEY,      VIDEO_SOURCE_IMAGE_EXPOSURE_MODE,      (int8_t*)&(ImageBaseParamPtr->s_ExposureMode),     GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MIN_KEY, VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MIN, (int32_t*)&(ImageBaseParamPtr->s_ExposureValueMin), GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MAX_KEY, VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MAX, (int32_t*)&(ImageBaseParamPtr->s_ExposureValueMax), GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_GAIN_MAX_KEY,           VIDEO_SOURCE_IMAGE_GAIN_MAX,           (int16_t*)&(ImageBaseParamPtr->s_GainMax),          GMI_CONFIG_READ_WRITE);
    if (GetImageFail)
    {
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_BRIGHTNESS_KEY,         VIDEO_SOURCE_IMAGE_BRIGHTNESS,         (int16_t*)&(ImageBaseParamPtr->s_Brightness),       GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_CONTRAST_KEY,           VIDEO_SOURCE_IMAGE_CONTRAST,           (int16_t*)&(ImageBaseParamPtr->s_Contrast),         GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_SATURATION_KEY,         VIDEO_SOURCE_IMAGE_SATURATION,         (int16_t*)&(ImageBaseParamPtr->s_Saturation),       GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_HUE_KEY,                VIDEO_SOURCE_IMAGE_HUE,                (int16_t*)&(ImageBaseParamPtr->s_Hue),              GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_SHARPNESS_KEY,          VIDEO_SOURCE_IMAGE_SHARPNESS,          (int16_t*)&(ImageBaseParamPtr->s_Sharpness),        GMI_CONFIG_READ_WRITE);
    }
    else
    {
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_BRIGHTNESS_KEY,         Images[0],         (int16_t*)&(ImageBaseParamPtr->s_Brightness),       GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_CONTRAST_KEY,           Images[1],         (int16_t*)&(ImageBaseParamPtr->s_Contrast),         GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_SATURATION_KEY,         Images[2],         (int16_t*)&(ImageBaseParamPtr->s_Saturation),       GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_HUE_KEY,                Images[3],         (int16_t*)&(ImageBaseParamPtr->s_Hue),              GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_SHARPNESS_KEY,          Images[4],         (int16_t*)&(ImageBaseParamPtr->s_Sharpness),        GMI_CONFIG_READ_WRITE);
    }

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetImageSettingsDefault(int32_t SourceId, int32_t ChanId, SysPkgImaging *SysImagingPtr)
{
    boolean_t  GetImageFail = false;
    int32_t    Images[32];
    memset(Images, 0, sizeof(Images));
    FactorySettingOperation FactoryOperation;
    GMI_RESULT Result = FactoryOperation.GetImageParams(Images);
    if (FAILED(Result))
    {
        SYS_ERROR("get image params form capoperation fail, Result = 0x%lx\n", Result);
        GetImageFail = true;
    }

    if (GetImageFail)
    {
        SysImagingPtr->s_Brightness                  = FloatToInt((float_t)(VIDEO_SOURCE_IMAGE_BRIGHTNESS + 255) / 2);
        SysImagingPtr->s_Contrast                    = VIDEO_SOURCE_IMAGE_CONTRAST;
        SysImagingPtr->s_Saturation                  = VIDEO_SOURCE_IMAGE_SATURATION;
        SysImagingPtr->s_Hue                         = FloatToInt((float_t)((VIDEO_SOURCE_IMAGE_HUE + 15) * 255) / 30);
        SysImagingPtr->s_Sharpness                   = VIDEO_SOURCE_IMAGE_SHARPNESS;
    }
    else
    {
        SysImagingPtr->s_Brightness                  = FloatToInt((float_t)(Images[0] + 255) / 2);
        SysImagingPtr->s_Contrast                    = Images[1];
        SysImagingPtr->s_Saturation                  = Images[2];
        SysImagingPtr->s_Hue                         = FloatToInt((float_t)((Images[3] + 15) * 255) / 30);
        SysImagingPtr->s_Sharpness                   = Images[4];
    }
    SysImagingPtr->s_VideoId                     = 1;
    SysImagingPtr->s_Exposure.s_ExposureMode     = VIDEO_SOURCE_IMAGE_EXPOSURE_MODE;
    SysImagingPtr->s_Exposure.s_ShutterMax       = VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MAX;
    SysImagingPtr->s_Exposure.s_ShutterMin       = VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MIN;
    SysImagingPtr->s_Exposure.s_GainMax          = VIDEO_SOURCE_IMAGE_GAIN_MAX;

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetImageSettings(int32_t SourceId, int32_t ChanId, ImageBaseParam *ImageBaseParamPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     ImageXPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(ImageXPath, 0, sizeof(ImageXPath));
    strcpy(ImageXPath, VIDEO_SOURCE_IMAGE_PATH);
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_EXPOSURE_MODE_KEY,      (const int8_t)(ImageBaseParamPtr->s_ExposureMode));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MIN_KEY, (const int32_t)(ImageBaseParamPtr->s_ExposureValueMin));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_EXPOSURE_VALUE_MAX_KEY, (const int32_t)(ImageBaseParamPtr->s_ExposureValueMax));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_GAIN_MAX_KEY,           (const int16_t)(ImageBaseParamPtr->s_GainMax));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_BRIGHTNESS_KEY,         (const int16_t)(ImageBaseParamPtr->s_Brightness));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_CONTRAST_KEY,           (const int16_t)(ImageBaseParamPtr->s_Contrast));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_SATURATION_KEY,         (const int16_t)(ImageBaseParamPtr->s_Saturation));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_HUE_KEY,                (const int16_t)(ImageBaseParamPtr->s_Hue));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_SOURCE_IMAGE_SHARPNESS_KEY,          (const int16_t)(ImageBaseParamPtr->s_Sharpness));

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetImageAdvanceSettings(int32_t SourceId, int32_t ChanId, ImageAdvanceParam *ImageAdvanceParamPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     ImageXPath[128] = {0};

    boolean_t  GetImageFail = false;
    int32_t    Images[32];
    memset(Images, 0, sizeof(Images));
    FactorySettingOperation FactoryOperation;
    Result = FactoryOperation.GetImageParams(Images);
    if (FAILED(Result))
    {
        SYS_ERROR("get image params form capoperation fail, Result = 0x%lx\n", Result);
        GetImageFail = true;
    }

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(ImageXPath, 0, sizeof(ImageXPath));
    strcpy(ImageXPath, VIDEO_SOURCE_ADVANCED_IMAGE_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_METERING_MODE_KEY,  VIDEO_IMAGE_ADVANCE_METERING_MODE,      (int8_t*)&(ImageAdvanceParamPtr->s_MeteringMode),      GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_BACKLIGHTCOMP_KEY,  VIDEO_IMAGE_ADVANCE_BACKLIGHTCOMP_FLAG, (int8_t*)&(ImageAdvanceParamPtr->s_BackLightCompFlag), GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_DCIRIS_FLAG_KEY,    VIDEO_IMAGE_ADVANCE_DCIRIS_FLAG,        (int8_t*)&(ImageAdvanceParamPtr->s_DcIrisFlag),        GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_LOCAL_EXPOSURE_KEY, VIDEO_IMAGE_ADVANCE_LOCAL_EXPOSURE,     (int16_t*)&(ImageAdvanceParamPtr->s_LocalExposure),    GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_MCTF_STRLENGTH_KEY, VIDEO_IMAGE_ADVANCE_MCTF_STRLENGTH,     (int16_t*)&(ImageAdvanceParamPtr->s_MctfStrength),     GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_DCIRIS_DUTY_KEY,    VIDEO_IMAGE_ADVANCE_DCIRIS_DUTY,        (int16_t*)&(ImageAdvanceParamPtr->s_DcIrisDuty),       GMI_CONFIG_READ_WRITE);
    if (GetImageFail)
    {
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_AETARGETRATIO_KEY,  VIDEO_IMAGE_ADVANCE_AETARGETRATIO,      (int16_t*)&(ImageAdvanceParamPtr->s_AeTargetRatio),    GMI_CONFIG_READ_WRITE);
    }
    else
    {
        Result = GMI_XmlRead(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_AETARGETRATIO_KEY,  Images[5],      (int16_t*)&(ImageAdvanceParamPtr->s_AeTargetRatio),    GMI_CONFIG_READ_WRITE);
    }
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetImageAdvanceSettingsDefault(int32_t SourceId, int32_t ChanId, SysPkgAdvancedImaging *SysAdvancedImagingPtr)
{
    boolean_t  GetImageFail = false;
    int32_t    Images[32];
    memset(Images, 0, sizeof(Images));
    FactorySettingOperation FactoryOperation;
    GMI_RESULT Result = FactoryOperation.GetImageParams(Images);
    if (FAILED(Result))
    {
        SYS_ERROR("get image params form capoperation fail, Result = 0x%lx\n", Result);
        GetImageFail = true;
    }

    SysAdvancedImagingPtr->s_VideoId           = 1;
    SysAdvancedImagingPtr->s_MeteringMode      = VIDEO_IMAGE_ADVANCE_METERING_MODE;
    SysAdvancedImagingPtr->s_BackLightCompFlag = VIDEO_IMAGE_ADVANCE_BACKLIGHTCOMP_FLAG;
    SysAdvancedImagingPtr->s_DcIrisFlag        = VIDEO_IMAGE_ADVANCE_DCIRIS_FLAG;
    SysAdvancedImagingPtr->s_MctfStrength      = VIDEO_IMAGE_ADVANCE_MCTF_STRLENGTH;
    SysAdvancedImagingPtr->s_DcIrisDuty        = VIDEO_IMAGE_ADVANCE_DCIRIS_DUTY;
    SysAdvancedImagingPtr->s_LocalExposure     = VIDEO_IMAGE_ADVANCE_LOCAL_EXPOSURE;
    if (GetImageFail)
    {
        SysAdvancedImagingPtr->s_AeTargetRatio     = VIDEO_IMAGE_ADVANCE_AETARGETRATIO;
    }
    else
    {
        SysAdvancedImagingPtr->s_AeTargetRatio     = Images[5];
    }

    return GMI_SUCCESS;

}


GMI_RESULT ConfigFileManager::SetImageAdvanceSettings(int32_t SourceId, int32_t ChanId, ImageAdvanceParam *ImageAdvanceParamPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     ImageXPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(ImageXPath, 0, sizeof(ImageXPath));
    strcpy(ImageXPath, VIDEO_SOURCE_ADVANCED_IMAGE_PATH);
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_METERING_MODE_KEY, (const int8_t)(ImageAdvanceParamPtr->s_MeteringMode));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_BACKLIGHTCOMP_KEY, (const int8_t)(ImageAdvanceParamPtr->s_BackLightCompFlag));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_DCIRIS_FLAG_KEY,   (const int16_t)(ImageAdvanceParamPtr->s_DcIrisFlag));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_LOCAL_EXPOSURE_KEY,(const int16_t)(ImageAdvanceParamPtr->s_LocalExposure));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_MCTF_STRLENGTH_KEY,(const int16_t)(ImageAdvanceParamPtr->s_MctfStrength));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_DCIRIS_DUTY_KEY,   (const int16_t)(ImageAdvanceParamPtr->s_DcIrisDuty));
    Result = GMI_XmlWrite(Handle, (const char_t*)ImageXPath, VIDEO_IMAGE_ADVANCE_AETARGETRATIO_KEY, (const int16_t)(ImageAdvanceParamPtr->s_AeTargetRatio));

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetVideoSourceSettings(int32_t SourceId, SysPkgVideoSource *VideoSourcePtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     SourceXPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(SourceXPath, 0, sizeof(SourceXPath));
    strcpy(SourceXPath, VIDEO_SOURCE_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)SourceXPath, VIDEO_SOURCE_WIDTH_KEY,       VIDEO_SOURCE_WIDTH,      (int32_t*)&(VideoSourcePtr->s_SrcWidth),  GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)SourceXPath, VIDEO_SOURCE_HEIGHT_KEY,      VIDEO_SOURCE_HEIGHT,     (int32_t*)&(VideoSourcePtr->s_SrcHeight), GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)SourceXPath, VIDEO_SOURCE_FRAME_RATE_KEY,  VIDEO_SOURCE_FRAME_RATE, (int32_t*)&(VideoSourcePtr->s_SrcFps),    GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)SourceXPath, VIDEO_SOURCE_MIRROR_KEY,      VIDEO_SOURCE_MIRROR,     (int32_t*)&(VideoSourcePtr->s_Mirror),    GMI_CONFIG_READ_WRITE);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetVideoSourceSettings(int32_t SourceId, SysPkgVideoSource *VideoSourcePtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     SourceXPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(SourceXPath, 0, sizeof(SourceXPath));
    strcpy(SourceXPath, VIDEO_SOURCE_PATH);
    Result = GMI_XmlWrite(Handle, (const char_t*)SourceXPath, VIDEO_SOURCE_WIDTH_KEY,        (const int32_t)(VideoSourcePtr->s_SrcWidth));
    Result = GMI_XmlWrite(Handle, (const char_t*)SourceXPath, VIDEO_SOURCE_HEIGHT_KEY,       (const int32_t)(VideoSourcePtr->s_SrcHeight));
    Result = GMI_XmlWrite(Handle, (const char_t*)SourceXPath, VIDEO_SOURCE_FRAME_RATE_KEY,   (const int32_t)(VideoSourcePtr->s_SrcFps));
    Result = GMI_XmlWrite(Handle, (const char_t*)SourceXPath, VIDEO_SOURCE_MIRROR_KEY,       (const int32_t)(VideoSourcePtr->s_Mirror));
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetWhiteBalanceSettings(int32_t SourceId, ImageWbParam *ImageWbParamPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     WhiteBalancePath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(WhiteBalancePath, 0, sizeof(WhiteBalancePath));
    strcpy(WhiteBalancePath, VIDEO_IMAGE_WHITE_BALANCE_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)WhiteBalancePath, VIDEO_IMAGE_WHITE_BALANCE_MODE_KEY,   VIDEO_IMAGE_WHITE_BALANCE_MODE,  &(ImageWbParamPtr->s_WbMode),  GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)WhiteBalancePath, VIDEO_IMAGE_WHITE_BALANCE_RGAIN_KEY,  VIDEO_IMAGE_WHITE_BALANCE_RGAIN, &(ImageWbParamPtr->s_WbRgain), GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)WhiteBalancePath, VIDEO_IMAGE_WHITE_BALANCE_BGAIN_KEY,  VIDEO_IMAGE_WHITE_BALANCE_BGAIN, &(ImageWbParamPtr->s_WbBgain), GMI_CONFIG_READ_WRITE);

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetWhiteBalanceSettings(int32_t SourceId, ImageWbParam *ImageWbParamPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     WhiteBalancePath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(WhiteBalancePath, 0, sizeof(WhiteBalancePath));
    strcpy(WhiteBalancePath, VIDEO_IMAGE_WHITE_BALANCE_PATH);
    Result = GMI_XmlWrite(Handle, (const char_t*)WhiteBalancePath, VIDEO_IMAGE_WHITE_BALANCE_MODE_KEY,  (const int16_t)(ImageWbParamPtr->s_WbMode));
    Result = GMI_XmlWrite(Handle, (const char_t*)WhiteBalancePath, VIDEO_IMAGE_WHITE_BALANCE_RGAIN_KEY, (const int16_t)(ImageWbParamPtr->s_WbRgain));
    Result = GMI_XmlWrite(Handle, (const char_t*)WhiteBalancePath, VIDEO_IMAGE_WHITE_BALANCE_BGAIN_KEY, (const int16_t)(ImageWbParamPtr->s_WbBgain));

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetDaynightSettings(int32_t SourceId, ImageDnParam *ImageDnParamPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     DayNightPath[128] = {0};
    char_t     SchedEnableKey[64];
    char_t     SchedStartTimeKey[64];
    char_t     SchedEndTimeKey[64];

    boolean_t  GetIrcutDnFail = false;
    int8_t     IrcutMode;
    int8_t     DayToNightThr;
    int8_t     NightToDayThr;
    FactorySettingOperation FactoryOperation;
    Result = FactoryOperation.GetIrCutModeDnThr(&IrcutMode, &DayToNightThr, &NightToDayThr);
    if (FAILED(Result))
    {
        SYS_ERROR("get ircut mode daytonight nighttoday threshold form capoperation fail, Result = 0x%lx\n", Result);
        GetIrcutDnFail = true;
    }

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(DayNightPath, 0, sizeof(DayNightPath));
    strcpy(DayNightPath, VIDEO_IMAGE_DAY_NIGHT_PATH);
    if (GetIrcutDnFail)
    {
        Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_DAY_NIGHT_MODE_KEY,          VIDEO_IMAGE_DAY_NIGHT_MODE,         &(ImageDnParamPtr->s_DnMode),        GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_DAY_TO_NIGHT_THRESHOLD_KEY,  VIDEO_IMAGE_DAY_TO_NIGHT_THRESHOLD, &(ImageDnParamPtr->s_DayToNightThr), GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_NIGHT_TO_DAY_THRESHOLD_KEY,  VIDEO_IMAGE_NIGHT_TO_DAY_THRESHOLD, &(ImageDnParamPtr->s_NightToDayThr), GMI_CONFIG_READ_WRITE);
    }
    else
    {
        if (IRCUT_MODE_ADC == IrcutMode)
        {
            Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_DAY_NIGHT_MODE_KEY, DN_MODE_AUTO, &(ImageDnParamPtr->s_DnMode), GMI_CONFIG_READ_WRITE);
        }
        else
        {
            Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_DAY_NIGHT_MODE_KEY, DN_MODE_DAY,  &(ImageDnParamPtr->s_DnMode), GMI_CONFIG_READ_WRITE);
        }
        Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_DAY_TO_NIGHT_THRESHOLD_KEY,  DayToNightThr, &(ImageDnParamPtr->s_DayToNightThr), GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_NIGHT_TO_DAY_THRESHOLD_KEY,  NightToDayThr, &(ImageDnParamPtr->s_NightToDayThr), GMI_CONFIG_READ_WRITE);
    }
    Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_DAY_NIGHT_DURTIME_KEY,  VIDEO_IMAGE_DAY_NIGHT_DURTIME,  &(ImageDnParamPtr->s_DnDurtime), GMI_CONFIG_READ_WRITE);

    for (int32_t i = 0; i < SCHEDULE_WEEK_DAYS; i++)
    {
        memset(SchedEnableKey, 0, sizeof(SchedEnableKey));
        memset(SchedStartTimeKey, 0, sizeof(SchedStartTimeKey));
        memset(SchedEndTimeKey, 0, sizeof(SchedEndTimeKey));

        sprintf(SchedEnableKey,    VIDEO_IMAGE_DAY_NIGHT_SCHED_ENABLE_KEY, i);
        sprintf(SchedStartTimeKey, VIDEO_IMAGE_DAY_NIGHT_SCHED_START_TIME_KEY, i);
        sprintf(SchedEndTimeKey,   VIDEO_IMAGE_DAY_NIGHT_SCHED_END_TIME_KEY, i);
        Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, SchedEnableKey,     VIDEO_IMAGE_DAY_NIGHT_SCHED_ENABLE,     (int32_t*)&(ImageDnParamPtr->s_DnSchedule.s_DnSchedFlag[i]), GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, SchedStartTimeKey,  VIDEO_IMAGE_DAY_NIGHT_SCHED_START_TIME, (int32_t*)&(ImageDnParamPtr->s_DnSchedule.s_StartTime[i]),   GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)DayNightPath, SchedEndTimeKey,    VIDEO_IMAGE_DAY_NIGHT_SCHED_END_TIME,   (int32_t*)&(ImageDnParamPtr->s_DnSchedule.s_EndTime[i]),     GMI_CONFIG_READ_WRITE);
    }

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetDaynightSettings(int32_t SourceId, ImageDnParam *ImageDnParamPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t	   DayNightPath[128] = {0};
    char_t	   SchedEnableKey[64];
    char_t	   SchedStartTimeKey[64];
    char_t	   SchedEndTimeKey[64];

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(DayNightPath, 0, sizeof(DayNightPath));
    strcpy(DayNightPath, VIDEO_IMAGE_DAY_NIGHT_PATH);
    Result = GMI_XmlWrite(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_DAY_NIGHT_MODE_KEY,         (const int8_t)(ImageDnParamPtr->s_DnMode));
    Result = GMI_XmlWrite(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_DAY_NIGHT_DURTIME_KEY,      (const int8_t)(ImageDnParamPtr->s_DnDurtime));
    Result = GMI_XmlWrite(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_DAY_TO_NIGHT_THRESHOLD_KEY, (const int8_t)(ImageDnParamPtr->s_DayToNightThr));
    Result = GMI_XmlWrite(Handle, (const char_t*)DayNightPath, VIDEO_IMAGE_NIGHT_TO_DAY_THRESHOLD_KEY, (const int8_t)(ImageDnParamPtr->s_NightToDayThr));
    for (int32_t i = 0; i < SCHEDULE_WEEK_DAYS; i++)
    {
        memset(SchedEnableKey, 0, sizeof(SchedEnableKey));
        memset(SchedStartTimeKey, 0, sizeof(SchedStartTimeKey));
        memset(SchedEndTimeKey, 0, sizeof(SchedEndTimeKey));

        sprintf(SchedEnableKey,    VIDEO_IMAGE_DAY_NIGHT_SCHED_ENABLE_KEY, i);
        sprintf(SchedStartTimeKey, VIDEO_IMAGE_DAY_NIGHT_SCHED_START_TIME_KEY, i);
        sprintf(SchedEndTimeKey,   VIDEO_IMAGE_DAY_NIGHT_SCHED_END_TIME_KEY, i);

        Result = GMI_XmlWrite(Handle, (const char_t*)DayNightPath, SchedEnableKey,    (const int32_t)(ImageDnParamPtr->s_DnSchedule.s_DnSchedFlag[i]));
        Result = GMI_XmlWrite(Handle, (const char_t*)DayNightPath, SchedStartTimeKey, (const int32_t)(ImageDnParamPtr->s_DnSchedule.s_StartTime[i]));
        Result = GMI_XmlWrite(Handle, (const char_t*)DayNightPath, SchedEndTimeKey,   (const int32_t)(ImageDnParamPtr->s_DnSchedule.s_EndTime[i]));
    }

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetDaynightSettingsDefault(int32_t SourceId, SysPkgDaynight *SysDaynight)
{
    boolean_t  GetIrcutDnFail = false;
    int8_t     IrcutMode;
    int8_t     DayToNightThr;
    int8_t     NightToDayThr;  
    
    FactorySettingOperation FactoryOperation;
    GMI_RESULT Result = FactoryOperation.GetIrCutModeDnThr(&IrcutMode, &DayToNightThr, &NightToDayThr);
    if (FAILED(Result))
    {
        SYS_ERROR("get ircut mode daytonight nighttoday threshold form capoperation fail, Result = 0x%lx\n", Result);
        GetIrcutDnFail = true;
    }

    if (GetIrcutDnFail)
    {
        SysDaynight->s_Mode = SYS_ENV_DEFAULT_DN_MODE;
        SysDaynight->s_NightToDayThr = SYS_ENV_DEFAULT_NIGHT_TO_DAY_THR;
        SysDaynight->s_DayToNightThr = SYS_ENV_DEFAULT_DAY_TO_NIGHT_THR;
    }
    else
    {
        if (IRCUT_MODE_ADC == IrcutMode)
        {
            SysDaynight->s_Mode = DN_MODE_AUTO;//ir device
        }
        else
        {
            SysDaynight->s_Mode = DN_MODE_DAY;
        }
        SysDaynight->s_NightToDayThr = NightToDayThr;
        SysDaynight->s_DayToNightThr = DayToNightThr;
    }

    SysDaynight->s_DurationTime = SYS_ENV_DEFAULT_DN_DURTIME;

    for (uint8_t i = 0; i < DAY_NIGHT_SCHED_WEEKS; i++)
    {
        SysDaynight->s_SchedEnable[i] = SYS_ENV_DEFAULT_SCHED_ENABLE;
        SysDaynight->s_SchedStartTime[i] = SYS_ENV_DEFAULT_START_TIME;
        SysDaynight->s_SchedEndTime[i] = SYS_ENV_DEFAULT_END_TIME;
    }

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetAudioEncodeSettings(AudioEncParam *AudioEncodeCfgPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     AudioEncodePath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    sprintf(AudioEncodePath, AUDIO_ENCODE_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_TYPE_KEY,           AUDIO_ENCODE_TYPE,           (int32_t*)&AudioEncodeCfgPtr->s_Codec,      GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_CHANNEL_KEY,        AUDIO_ENCODE_CHANNEL,        &AudioEncodeCfgPtr->s_ChannelNum,           GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_BITS_PERSAMPLE_KEY, AUDIO_ENCODE_BITS_PERSAMPLE, (int32_t*)&AudioEncodeCfgPtr->s_BitWidth,   GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_SAMPLES_PERSEC_KEY, AUDIO_ENCODE_SAMPLES_PERSEC, (int32_t*)&AudioEncodeCfgPtr->s_SampleFreq, GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_CAP_VOLUME_KEY,     AUDIO_ENCODE_CAP_VOLUME,     &AudioEncodeCfgPtr->s_Volume,               GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_BIT_RATE_KEY,       AUDIO_ENCODE_BIT_RATE,       &AudioEncodeCfgPtr->s_BitRate,              GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_FRAME_RATE_KEY,     AUDIO_ENCODE_FRAME_RATE,     &AudioEncodeCfgPtr->s_FrameRate,            GMI_CONFIG_READ_WRITE);
    //Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_PLAY_VOLUME_KEY, AUDIO_ENCODE_PLAY_VOLUME, &AudioEncodeCfgPtr->s_PlayVolume, GMI_CONFIG_READ_WRITE);
    //Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_PLAY_ENABLE_KEY, AUDIO_ENCODE_PLAY_ENABLE, &AudioEncodeCfgPtr->s_, GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_AEC_ENABLE_KEY,     AUDIO_ENCODE_AEC_ENABLE,     &AudioEncodeCfgPtr->s_AecFlag,              GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_AEC_DELAY_TIME_KEY, AUDIO_ENCODE_AEC_DELAY_TIME, &AudioEncodeCfgPtr->s_AecDelayTime,         GMI_CONFIG_READ_WRITE);

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetAudioEncodeSettings(SysPkgAudioEncodeCfg *AudioEncodeCfgPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     AudioEncodePath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    sprintf(AudioEncodePath, AUDIO_ENCODE_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_TYPE_KEY,           AUDIO_ENCODE_TYPE,           &AudioEncodeCfgPtr->s_EncodeType,                GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_CHANNEL_KEY,        AUDIO_ENCODE_CHANNEL,        &AudioEncodeCfgPtr->s_Chan,                      GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_BITS_PERSAMPLE_KEY, AUDIO_ENCODE_BITS_PERSAMPLE, &AudioEncodeCfgPtr->s_BitsPerSample,             GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_SAMPLES_PERSEC_KEY, AUDIO_ENCODE_SAMPLES_PERSEC, (int32_t*)&AudioEncodeCfgPtr->s_SamplesPerSec,   GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_CAP_VOLUME_KEY,     AUDIO_ENCODE_CAP_VOLUME,     &AudioEncodeCfgPtr->s_CapVolume,                 GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_BIT_RATE_KEY,       AUDIO_ENCODE_BIT_RATE,       &AudioEncodeCfgPtr->s_BitRate,                   GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_FRAME_RATE_KEY,     AUDIO_ENCODE_FRAME_RATE,     &AudioEncodeCfgPtr->s_FrameRate,                 GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_PLAY_VOLUME_KEY,    AUDIO_ENCODE_PLAY_VOLUME,    &AudioEncodeCfgPtr->s_PlayVolume,                GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_PLAY_ENABLE_KEY,    AUDIO_ENCODE_PLAY_ENABLE,    &AudioEncodeCfgPtr->s_PlayEnable,                GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_AEC_ENABLE_KEY,     AUDIO_ENCODE_AEC_ENABLE,     &AudioEncodeCfgPtr->s_AecFlag,                   GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_AEC_DELAY_TIME_KEY, AUDIO_ENCODE_AEC_DELAY_TIME, &AudioEncodeCfgPtr->s_AecDelayTime,              GMI_CONFIG_READ_WRITE);

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetAudioEncodeSettings(SysPkgAudioEncodeCfg *AudioEncodeCfgPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     AudioEncodePath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    sprintf(AudioEncodePath, AUDIO_ENCODE_PATH);
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_TYPE_KEY,            (const uint8_t)(AudioEncodeCfgPtr->s_EncodeType));
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_CHANNEL_KEY,         (const uint8_t)(AudioEncodeCfgPtr->s_Chan));
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_BITS_PERSAMPLE_KEY,  (const uint8_t)(AudioEncodeCfgPtr->s_BitsPerSample));
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_SAMPLES_PERSEC_KEY,  (const int32_t)(AudioEncodeCfgPtr->s_SamplesPerSec));
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_CAP_VOLUME_KEY,      (const uint16_t)(AudioEncodeCfgPtr->s_CapVolume));
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_PLAY_VOLUME_KEY,     (const uint16_t)(AudioEncodeCfgPtr->s_PlayVolume));
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_PLAY_ENABLE_KEY,     (const uint8_t)(AudioEncodeCfgPtr->s_PlayEnable));
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_AEC_ENABLE_KEY,      (const uint8_t)(AudioEncodeCfgPtr->s_AecFlag));
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_AEC_DELAY_TIME_KEY,  (const uint16_t)(AudioEncodeCfgPtr->s_AecDelayTime));
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_BIT_RATE_KEY,        (const uint16_t)(AudioEncodeCfgPtr->s_BitRate));
    Result = GMI_XmlWrite(Handle, (const char_t*)AudioEncodePath, AUDIO_ENCODE_FRAME_RATE_KEY,      (const uint16_t)(AudioEncodeCfgPtr->s_FrameRate));
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetHwAutoDetectInfo(SysPkgComponents *SysComponents)
{
#define SENSOR_ID_2715   "ov2715"
#define SENSOR_ID_9715   "ov9715"
#define SENSOR_ID_IMX122 "imx122"
#define SENSOR_ID_34041  "mn34041pl"
#define SENSOR_ID_TW9910 "tw9910"

#define CPU_ID_A55       "A5S_55"
#define CPU_ID_A66       "A5S_66"
#define CPU_ID_A88       "A5S_88"

#define LENS_NONE        "NONE"
#define LENS_DF003       "DF003"
#define LENS_YB22        "YB22"

    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     HwAutoDetectPath[128] = {0};
    char_t     Cpu[64];
    char_t     Sensor[64];
    char_t     Lens[64];

    Result = GMI_XmlOpen("/opt/config/capability_auto.xml", &Handle);
    if (FAILED(Result))
    {
        return Result;
    }

    memset(HwAutoDetectPath, 0, sizeof(HwAutoDetectPath));
    strcpy(HwAutoDetectPath, HW_AUTO_DETECT_INFO_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)HwAutoDetectPath, HW_CPU_KEY,    HW_CPU,    Cpu,    GMI_CONFIG_READ_ONLY);
    Result = GMI_XmlRead(Handle, (const char_t*)HwAutoDetectPath, HW_SENSOR_KEY, HW_SENSOR, Sensor, GMI_CONFIG_READ_ONLY);
    Result = GMI_XmlRead(Handle, (const char_t*)HwAutoDetectPath, HW_LENS_KEY,   HW_LENS,   Lens,   GMI_CONFIG_READ_ONLY);

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        return Result;
    }

    SYS_INFO("Cpu %s, Sensor %s, Lens %s\n", Cpu, Sensor, Lens);

    if (strcmp(Sensor, SENSOR_ID_2715) == 0)
    {
        SysComponents->s_SensorId = e_SENSOR_OV2715;
    }
    else if (strcmp(Sensor, SENSOR_ID_9715) == 0)
    {
        SysComponents->s_SensorId = e_SENSOR_OV9715;
    }
    else if (strcmp(Sensor, SENSOR_ID_34041) == 0)
    {
        SysComponents->s_SensorId = e_SENSOR_MN34041;
    }
    else if (strcmp(Sensor, SENSOR_ID_IMX122) == 0)
    {
        SysComponents->s_SensorId = e_SENSOR_IMX122;
    }
    else
    {
        SysComponents->s_SensorId = e_SENSOR_OV9715;
    }

    if (strcmp(Cpu, CPU_ID_A88) == 0)
    {
        SysComponents->s_CpuId = e_CPU_A5S_88;
    }
    else if (strcmp(Cpu, CPU_ID_A66) == 0)
    {
        SysComponents->s_CpuId = e_CPU_A5S_66;
    }
    else
    {
        SysComponents->s_CpuId = e_CPU_A5S_55;
    }

    if (strcmp(Lens, LENS_DF003) == 0)
    {
        SysComponents->s_ZoomLensId = e_ZOOM_LENS_DF003;
    }
    else if (strcmp(Lens, LENS_YB22) == 0)
    {
        SysComponents->s_ZoomLensId = e_ZOOM_LENS_YB22;
    }
    else
    {
        SysComponents->s_ZoomLensId = e_ZOOM_LENS_NONE;
    }

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetDeviceInfo(SysPkgDeviceInfo *SysDeviceInfoPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     DeviceInfoPath[128] = {0};

	boolean_t  GetDeviceInfoFail = false;
	SysPkgDeviceInfo SysDeviceInfoFactory;
	memset(&SysDeviceInfoFactory, 0, sizeof(SysPkgDeviceInfo));
	FactorySettingOperation FactoryOperation;
	Result = FactoryOperation.GetDeviceInfo(&SysDeviceInfoFactory);
	if (FAILED(Result))
	{
		SYS_ERROR("get factory device informaiton form capoperation fail, Result = 0x%lx\n", Result);
		GetDeviceInfoFail = true;
	}

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(DeviceInfoPath, 0, sizeof(DeviceInfoPath));
    strcpy(DeviceInfoPath, DEVICE_INFO_PATH);
    if (GetDeviceInfoFail)
    {
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_NAME_KEY,  DEVICE_NAME,  (char_t*)&(SysDeviceInfoPtr->s_DeviceName),     GMI_CONFIG_READ_WRITE);
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_ID_KEY,    DEVICE_ID,    (int32_t*)&(SysDeviceInfoPtr->s_DeviceId),      GMI_CONFIG_READ_WRITE);
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_MODEL_KEY, DEVICE_MODEL, (char_t*)&(SysDeviceInfoPtr->s_DeviceModel),    GMI_CONFIG_READ_WRITE);
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_MANUFACTUER_KEY, DEVICE_MANUFACTUER, (char_t*)&(SysDeviceInfoPtr->s_DeviceManufactuer), GMI_CONFIG_READ_WRITE);
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_SN_KEY,    DEVICE_SN,    (char_t*)&(SysDeviceInfoPtr->s_DeviceSerialNum),GMI_CONFIG_READ_WRITE);	   
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_HWVER_KEY, DEVICE_HWVER, (char_t*)&(SysDeviceInfoPtr->s_DeviceHwVer),    GMI_CONFIG_READ_WRITE);
    }
    else
    {
    	Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_NAME_KEY,  SysDeviceInfoFactory.s_DeviceName,  (char_t*)&(SysDeviceInfoPtr->s_DeviceName),     GMI_CONFIG_READ_WRITE);
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_ID_KEY,    SysDeviceInfoFactory.s_DeviceId,    (int32_t*)&(SysDeviceInfoPtr->s_DeviceId),      GMI_CONFIG_READ_WRITE);
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_MODEL_KEY, SysDeviceInfoFactory.s_DeviceModel, (char_t*)&(SysDeviceInfoPtr->s_DeviceModel),    GMI_CONFIG_READ_WRITE);
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_MANUFACTUER_KEY, SysDeviceInfoFactory.s_DeviceManufactuer, (char_t*)&(SysDeviceInfoPtr->s_DeviceManufactuer), GMI_CONFIG_READ_WRITE);
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_SN_KEY,    SysDeviceInfoFactory.s_DeviceSerialNum,    (char_t*)&(SysDeviceInfoPtr->s_DeviceSerialNum),GMI_CONFIG_READ_WRITE);	    
	    Result = GMI_XmlRead(Handle, (const char_t*)DeviceInfoPath, DEVICE_HWVER_KEY, SysDeviceInfoFactory.s_DeviceHwVer, (char_t*)&(SysDeviceInfoPtr->s_DeviceHwVer),    GMI_CONFIG_READ_WRITE);
    }
    
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetDeviceInfo(SysPkgDeviceInfo *SysDeviceInfoPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     DeviceInfoPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(DeviceInfoPath, 0, sizeof(DeviceInfoPath));
    strcpy(DeviceInfoPath, DEVICE_INFO_PATH);

    Result = GMI_XmlWrite(Handle, (const char_t*)DeviceInfoPath,  DEVICE_NAME_KEY, (const char_t*)(SysDeviceInfoPtr->s_DeviceName));
    Result = GMI_XmlWrite(Handle, (const char_t*)DeviceInfoPath,  DEVICE_ID_KEY,   (const int32_t)(SysDeviceInfoPtr->s_DeviceId));
    //Result = GMI_XmlWrite(Handle, (const char_t*)DeviceInfoPath,  DEVICE_MODEL_KEY,(const char_t*)(SysDeviceInfoPtr->s_DeviceModel));
    //Result = GMI_XmlWrite(Handle, (const char_t*)DeviceInfoPath,  DEVICE_MANUFACTUER_KEY,(const char_t*)(SysDeviceInfoPtr->s_DeviceManufactuer));
    //Result = GMI_XmlWrite(Handle, (const char_t*)DeviceInfoPath,  DEVICE_SN_KEY,   (const char_t*)(SysDeviceInfoPtr->s_DeviceSerialNum));
    //Result = GMI_XmlWrite(Handle, (const char_t*)DeviceInfoPath,  DEVICE_FWVER_KEY,(const char_t*)(SysDeviceInfoPtr->s_DeviceFwVer));
    //Result = GMI_XmlWrite(Handle, (const char_t*)DeviceInfoPath,  DEVICE_HWVER_KEY,(const char_t*)(SysDeviceInfoPtr->s_DeviceHwVer));
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetNtpServerInfo(SysPkgNtpServerInfo *SysNtpServerInfoPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     NtpInfoPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(NtpInfoPath, 0, sizeof(NtpInfoPath));
    strcpy(NtpInfoPath, NTP_INFO_PATH);

    Result = GMI_XmlWrite(Handle, (const char_t*)NtpInfoPath,  NTP_INFO_1_KEY, (const char_t*)(SysNtpServerInfoPtr->s_NtpAddr_1));
    Result = GMI_XmlWrite(Handle, (const char_t*)NtpInfoPath,  NTP_INFO_2_KEY, (const char_t*)(SysNtpServerInfoPtr->s_NtpAddr_2));
    Result = GMI_XmlWrite(Handle, (const char_t*)NtpInfoPath,  NTP_INFO_3_KEY, (const char_t*)(SysNtpServerInfoPtr->s_NtpAddr_3));
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetNtpServerInfo(SysPkgNtpServerInfo *SysNtpServerInfoPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     NtpInfoPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(NtpInfoPath, 0, sizeof(NtpInfoPath));
    strcpy(NtpInfoPath, NTP_INFO_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)NtpInfoPath, NTP_INFO_1_KEY,  NTP_INFO_1,  (char_t*)&(SysNtpServerInfoPtr->s_NtpAddr_1),  GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)NtpInfoPath, NTP_INFO_2_KEY,  NTP_INFO_2,  (char_t*)&(SysNtpServerInfoPtr->s_NtpAddr_2),  GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)NtpInfoPath, NTP_INFO_3_KEY,  NTP_INFO_3,  (char_t*)&(SysNtpServerInfoPtr->s_NtpAddr_3),  GMI_CONFIG_READ_WRITE);

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetSysTimeType(SysPkgDateTimeType *SysDateTimePtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     TimeTypeInfoPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(TimeTypeInfoPath, 0, sizeof(TimeTypeInfoPath));
    strcpy(TimeTypeInfoPath, DATE_TIME_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)TimeTypeInfoPath, DATE_TIME_TYPE_KEY,  DATE_TIME_TYPE,  (int32_t*)&(SysDateTimePtr->s_Type),  GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)TimeTypeInfoPath, DATE_TIME_NTP_INTERVAL_KEY,  DATE_TIME_NTP_INTERVAL,  (int32_t*)&(SysDateTimePtr->s_NtpInterval),  GMI_CONFIG_READ_WRITE);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetSysTimeType(SysPkgDateTimeType *SysDateTimePtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     TimeTypeInfoPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(TimeTypeInfoPath, 0, sizeof(TimeTypeInfoPath));
    strcpy(TimeTypeInfoPath, DATE_TIME_PATH);

    Result = GMI_XmlWrite(Handle, (const char_t*)TimeTypeInfoPath,  DATE_TIME_TYPE_KEY, (const int32_t)(SysDateTimePtr->s_Type));
    Result = GMI_XmlWrite(Handle, (const char_t*)TimeTypeInfoPath,  DATE_TIME_NTP_INTERVAL_KEY, (const int32_t)(SysDateTimePtr->s_NtpInterval));
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetExternNetworkPort(SysPkgNetworkPort *SysNetworkPortPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     ExternNetworkPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(ExternNetworkPath, 0, sizeof(ExternNetworkPath));
    strcpy(ExternNetworkPath, GMI_EXTERN_NETWORK_PORT_PATH);

    Result = GMI_XmlWrite(Handle, (const char_t*)ExternNetworkPath,  GMI_HTTP_SERVER_PORT_KEY, (const int32_t)(SysNetworkPortPtr->s_HTTP_Port));
    Result = GMI_XmlWrite(Handle, (const char_t*)ExternNetworkPath,  GMI_RTSP_SERVER_TCP_PORT_KEY, (const int32_t)(SysNetworkPortPtr->s_RTSP_Port));
    Result = GMI_XmlWrite(Handle, (const char_t*)ExternNetworkPath,  GMI_SDK_SERVER_PORT_KEY, (const int32_t)(SysNetworkPortPtr->s_SDK_Port));
    Result = GMI_XmlWrite(Handle, (const char_t*)ExternNetworkPath,  GMI_DAEMON_UPDATE_SERVER_PORT_KEY, (const int32_t)(SysNetworkPortPtr->s_Upgrade_Port));
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();
    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetExternNetworkPort(SysPkgNetworkPort *SysNetworkPortPt)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     ExternNetworkPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(ExternNetworkPath, 0, sizeof(ExternNetworkPath));
    strcpy(ExternNetworkPath, GMI_EXTERN_NETWORK_PORT_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)ExternNetworkPath, GMI_HTTP_SERVER_PORT_KEY,  GMI_HTTP_SERVER_PORT,  (int32_t*)&(SysNetworkPortPt->s_HTTP_Port),  GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ExternNetworkPath, GMI_RTSP_SERVER_TCP_PORT_KEY,  GMI_RTSP_SERVER_TCP_PORT,  (int32_t*)&(SysNetworkPortPt->s_RTSP_Port),  GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ExternNetworkPath, GMI_SDK_SERVER_PORT_KEY,  GMI_SDK_SERVER_PORT,  (int32_t*)&(SysNetworkPortPt->s_SDK_Port),  GMI_CONFIG_READ_WRITE);
    Result = GMI_XmlRead(Handle, (const char_t*)ExternNetworkPath, GMI_DAEMON_UPDATE_SERVER_PORT_KEY,  GMI_DAEMON_UPDATE_SERVER_PORT,  (int32_t*)&(SysNetworkPortPt->s_Upgrade_Port),  GMI_CONFIG_READ_WRITE);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();
    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetAutoFocusMode(int32_t *FocusModePtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     AutoFocusConfigPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(AutoFocusConfigPath, 0, sizeof(AutoFocusConfigPath));
    strcpy(AutoFocusConfigPath, PTZ_AUTO_FOCUS_CONFIG_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)AutoFocusConfigPath, PTZ_AUTO_FOCUS_MODE_KEY,  PTZ_AUTO_FOCUS_MODE,  (int32_t*)FocusModePtr,  GMI_CONFIG_READ_WRITE);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetAutoFocusMode(int32_t FocusMode)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     AutoFocusConfigPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(AutoFocusConfigPath, 0, sizeof(AutoFocusConfigPath));
    strcpy(AutoFocusConfigPath, PTZ_AUTO_FOCUS_CONFIG_PATH);

    Result = GMI_XmlWrite(Handle, (const char_t*)AutoFocusConfigPath,  PTZ_AUTO_FOCUS_MODE_KEY, (const int32_t)FocusMode);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetPresetsInfo(SysPkgPresetInfo_Inner SysPresetsInfoInner[256])
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     PresetsPath[128] = {0};

    Result = GMI_XmlOpen((const char_t*)m_PresetsFile, &Handle);
    if (FAILED(Result))
    {
        return Result;
    }

    memset(PresetsPath, 0, sizeof(PresetsPath));

    for (uint32_t i = 0; i < MAX_PRESETS; i++)
    {
        sprintf(PresetsPath, PTZ_PRESET_INFO_PATH, i);
        Result = GMI_XmlRead(Handle, (const char_t*)PresetsPath, PTZ_PRESET_INDEX_KEY, (const int32_t)i, (int32_t*)&SysPresetsInfoInner[i].s_Index, GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)PresetsPath, PTZ_PRESET_NAME_KEY, (const char_t*)"preset", SysPresetsInfoInner[i].s_Name, GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)PresetsPath, PTZ_PRESET_SETTED_KEY, 0, (int32_t*)&SysPresetsInfoInner[i].s_Setted, GMI_CONFIG_READ_WRITE);
        Result = GMI_XmlRead(Handle, (const char_t*)PresetsPath, PTZ_PRESET_ZOOM_POSITION_KEY, 0, &SysPresetsInfoInner[i].s_ZoomPosition, GMI_CONFIG_READ_WRITE);
        if (FAILED(Result))
        {
            GMI_XmlFileSave(Handle);
            return Result;
        }
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetPresetsInfo(SysPkgPresetInfo_Inner *SysPresetInfoInnerPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     PresetsPath[128] = {0};

    Result = GMI_XmlOpen((const char_t*)m_PresetsFile, &Handle);
    if (FAILED(Result))
    {
        return Result;
    }

    memset(PresetsPath, 0, sizeof(PresetsPath));
    sprintf(PresetsPath, PTZ_PRESET_INFO_PATH, SysPresetInfoInnerPtr->s_Index);

    Result = GMI_XmlWrite(Handle, (const char_t*)PresetsPath,  PTZ_PRESET_INDEX_KEY, (const int32_t)SysPresetInfoInnerPtr->s_Index);
    Result = GMI_XmlWrite(Handle, (const char_t*)PresetsPath,  PTZ_PRESET_NAME_KEY,  (const char_t*)SysPresetInfoInnerPtr->s_Name);
    Result = GMI_XmlWrite(Handle, (const char_t*)PresetsPath,  PTZ_PRESET_SETTED_KEY,  (const int32_t)SysPresetInfoInnerPtr->s_Setted);
    Result = GMI_XmlWrite(Handle, (const char_t*)PresetsPath,  PTZ_PRESET_ZOOM_POSITION_KEY,  (const int32_t)SysPresetInfoInnerPtr->s_ZoomPosition);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetCurrentZoomPos(int32_t *ZoomPosPtr)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     ZoomPosConfigPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(ZoomPosConfigPath, 0, sizeof(ZoomPosConfigPath));
    strcpy(ZoomPosConfigPath, PTZ_CURRENT_ZOOM_CONFIG_PATH);
    Result = GMI_XmlRead(Handle, (const char_t*)ZoomPosConfigPath, PTZ_CURRENT_ZOOM_KEY,  PTZ_CURRENT_ZOOM,  (int32_t*)ZoomPosPtr,  GMI_CONFIG_READ_WRITE);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::SetCurrentZoomPos(int32_t ZoomPos)
{
    GMI_RESULT Result;
    FD_HANDLE  Handle;
    char_t     ZoomPosConfigPath[128] = {0};

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    memset(ZoomPosConfigPath, 0, sizeof(ZoomPosConfigPath));
    strcpy(ZoomPosConfigPath, PTZ_CURRENT_ZOOM_CONFIG_PATH);

    Result = GMI_XmlWrite(Handle, (const char_t*)ZoomPosConfigPath,  PTZ_CURRENT_ZOOM_KEY, (const int32_t)ZoomPos);
    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::GetPtzSpeedMap(char_t HSpeed[10][64], char_t VSpeed[10][64])
{
    boolean_t GetSpeedFail = false;
    char_t Hs[10][64]; // ptz speed range 0~100, step by 10
    char_t Vs[10][64];
    memset(Hs, 0, sizeof(Hs));
    memset(Vs, 0, sizeof(Vs));
    FactorySettingOperation FactoryOperation;
    GMI_RESULT Result = FactoryOperation.GetPtzSpeed(Hs, Vs);
    if (FAILED(Result))
    {
        SYS_ERROR("get ptz speed map form gmi_factory_setting.xml fail, Result = 0x%lx\n", Result);
        GetSpeedFail = true;
    }

    FD_HANDLE Handle;

    Lock();
    Result = GMI_XmlOpen((const char_t*)m_SettingFile, &Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }

    char_t PTZSpeedPath[64];
    memset(PTZSpeedPath, 0, sizeof(PTZSpeedPath));
    strcpy(PTZSpeedPath, PTZ_SPEED_MAP_PATH);

    char_t HKey[32];
    char_t VKey[32];
    if (GetSpeedFail)
    {
        const char_t *SpeedDefault = "50,";
        for (int32_t i = 0; i < 10; i++)
        {
            memset(HKey, 0, sizeof(HKey));
            sprintf(HKey, PTZ_H_SPEED_KEY, i);
            memset(VKey, 0, sizeof(VKey));
            sprintf(VKey, PTZ_V_SPEED_KEY, i);
            Result = GMI_XmlRead(Handle, (const char_t*)PTZSpeedPath, (const char_t*)HKey, SpeedDefault, HSpeed[i], GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)PTZSpeedPath, (const char_t*)VKey, SpeedDefault, VSpeed[i], GMI_CONFIG_READ_WRITE);
        }
    }
    else
    {
        for (int32_t i = 0; i < 10; i++)
        {
            memset(HKey, 0, sizeof(HKey));
            sprintf(HKey, PTZ_H_SPEED_KEY, i);
            memset(VKey, 0, sizeof(VKey));
            sprintf(VKey, PTZ_V_SPEED_KEY, i);
            Result = GMI_XmlRead(Handle, (const char_t*)PTZSpeedPath, (const char_t*)HKey, (const char_t*)Hs[i], HSpeed[i], GMI_CONFIG_READ_WRITE);
            Result = GMI_XmlRead(Handle, (const char_t*)PTZSpeedPath, (const char_t*)VKey, (const char_t*)Vs[i], VSpeed[i], GMI_CONFIG_READ_WRITE);
        }
    }

    if (FAILED(Result))
    {
        GMI_XmlFileSave(Handle);
        Unlock();
        return Result;
    }

    Result = GMI_XmlFileSave(Handle);
    if (FAILED(Result))
    {
        Unlock();
        return Result;
    }
    Unlock();

    return GMI_SUCCESS;
}


GMI_RESULT ConfigFileManager::FactoryDefault(void)
{
    int32_t Result;
    char_t  Cmd[128];
    struct stat FileInfo;

    Result = stat(GMI_SETTING_CONFIG_FILE_NAME, &FileInfo);
    if (0 == Result)
    {
        memset(Cmd, 0, sizeof(Cmd));
        sprintf(Cmd, "rm -f %s", GMI_SETTING_CONFIG_FILE_NAME);
        system(Cmd);
    }

    Result = stat(GMI_PTZ_PRESETS_PATROLS_FILE_NAME, &FileInfo);
    if (0 == Result)
    {
        memset(Cmd, 0, sizeof(Cmd));
        sprintf(Cmd, "rm -f %s", GMI_PTZ_PRESETS_PATROLS_FILE_NAME);
        system(Cmd);
    }

    Result = stat(GMI_RESOURCE_CONFIG_FILE_NAME, &FileInfo);
    if (0 == Result)
    {
        memset(Cmd, 0, sizeof(Cmd));
        sprintf(Cmd, "rm -f %s", GMI_RESOURCE_CONFIG_FILE_NAME);
        system(Cmd);
    }

    Result = stat(GMI_DAEMON_CONFIG_FILE, &FileInfo);
    if (0 == Result)
    {
        memset(Cmd, 0, sizeof(Cmd));
        sprintf(Cmd, "rm -f %s", GMI_DAEMON_CONFIG_FILE);
        system(Cmd);
    }

    return GMI_SUCCESS;
}
