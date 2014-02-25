#ifndef __SYSTEM_SERVICE_H__
#define __SYSTEM_SERVICE_H__

#include "alarm.h"
#include "net_manager.h"
#include "ptz_control.h"
#include "sys_env_types.h"
#include "stream_center_client.h"
#include "config_file_manager.h"
#include "board_manager.h"
#include "user_manager.h"
#include "sdk_stream_control.h"
#include "rtsp_stream_control.h"
#include "user_log_query.h"
#include "gmi_media_ctrl.h"
#include "gmi_system_headers.h"

#define FACTORY_DEFAULT_REBOOT_DELAY_TIMES  (5)
#define REBOOT_DELAY_TIMES                  (10)
#define VIDIN_BLOCKED_TIMES                 (45)

class SystemServiceManager :public UserLogQuery
{
public:
    SystemServiceManager();
    ~SystemServiceManager();
    GMI_RESULT Initialize();
    GMI_RESULT Deinitialize();
    //debug
    GMI_RESULT SvrStop3A(void);
    //ptz
    GMI_RESULT SvrPtzControl(SysPkgPtzCtrl *PtzCtrl);
    GMI_RESULT SvrGetAutoFocus(SysPkgAutoFocus *SysAutoFocusPtr);
    GMI_RESULT SvrSetAutoFocus(SysPkgAutoFocus *SysAutoFocusPtr);
    GMI_RESULT SvrGetPresetInfo(int32_t *PresetCnt, SysPkgPtzPresetInfo *SysPresetInfoPtr);
    GMI_RESULT SvrGetPresetInfo(int32_t Index, SysPkgPtzPresetInfo *SysPresetInfoPtr);
    GMI_RESULT SvrSetPresetInfo(SysPkgPtzPresetInfo *SysPresetInfoPtr);
    //net
    GMI_RESULT SvrNetReadIpInfo(SysPkgIpInfo *SysPkgIpInfoPtr);
    GMI_RESULT SvrNetReadMacInfo(char_t *MacPtr);
    GMI_RESULT SvrNetWriteIpInfo(SysPkgIpInfo *SysPkgIpInfoPtr);
    //video
    GMI_RESULT SvrGetVideoStreamNum(int32_t *StreamNum);
    GMI_RESULT SvrGetVideoEncodeSettings(int StreamId, SysPkgEncodeCfg *SysEncodeCfgPt); //StreamId = 0xff, get all video encode configurations
    GMI_RESULT SvrGetVideoEncodeSettingOptions();
    GMI_RESULT SvrGetVideoSourceSettings(int32_t SourceId, SysPkgVideoSource *SysVidSourcePtr);
    GMI_RESULT SvrSetVideoSourceSettings(int32_t SourceId, SysPkgVideoSource *SysVidSourcePtr);
    GMI_RESULT SvrForceIdrFrame(int32_t StreamId);
    GMI_RESULT SvrSetVideoEncodeSetting(int StreamId, SysPkgEncodeCfg *SysEncodeCfgPtr);//StreamId = 0xff, get all video encode configurations
    GMI_RESULT SvrGetVideoEncStreamCombine(SysPkgEncStreamCombine *SysEncStreamCombinePtr);
    GMI_RESULT SvrSetVideoEncStreamCombine(SysPkgEncStreamCombine *SysEncStreamCombinePtr);

    //audio
    GMI_RESULT SvrStartAudioDecode(int32_t AudioId, SysPkgAudioDecParam *SysAudioDecParamPtr);
    GMI_RESULT SvrStopAudioDecode(int32_t AudioId);
    GMI_RESULT SvrGetAudioEncodeSetting(int32_t AudioId, SysPkgAudioEncodeCfg *SysEncodeCfgPtr);
    GMI_RESULT SvrSetAudioEncodeSetting(int32_t AudioId, SysPkgAudioEncodeCfg *SysEncodeCfgPtr);
    //image
    GMI_RESULT SvrGetVideoSourceImageSettings(int32_t SourceId, SysPkgImaging *SysPkgImagingPtr);
    GMI_RESULT SvrSetVideoSourceImageSettings(int32_t SourceId, SysPkgImaging *SysPkgImagingPtr);
    GMI_RESULT SvrGetAdvancedImageSettings(int32_t SourceId, SysPkgAdvancedImaging *SysAdvancedImagingPtr);
    GMI_RESULT SvrSetAdvancedImagingSettings(int32_t SourceId, SysPkgAdvancedImaging *SysAdvancedImagingPtr);
    GMI_RESULT SvrGetWhiteBalanceSettings(int32_t SourceId, SysPkgWhiteBalance *SysWhiteBalancePtr);
    GMI_RESULT SvrSetWhiteBalanceSettings(int32_t SourceId, SysPkgWhiteBalance *SysWhiteBalancePtr);

    GMI_RESULT SvrSetDaynightSettings(int32_t SourceId, SysPkgDaynight *SysDaynightPr);
    GMI_RESULT SvrGetDaynightSettings(int32_t SourceId, SysPkgDaynight *SysDaynightPr);

    //time
    GMI_RESULT SvrSetTime(SysPkgSysTime *SysPkgTimePtr);
    GMI_RESULT SvrGetTime(SysPkgSysTime *SysPkgTimePtr);
    GMI_RESULT SvrGetTimezone(SysPkgTimeZone *SysTimezonePtr);
    GMI_RESULT SvrSetTimezone(SysPkgTimeZone *SysTimezonePtr);
    GMI_RESULT SvrGetNtpServerInfo(SysPkgNtpServerInfo *SysNtpServerInfoPtr);
    GMI_RESULT SvrSetNtpServerInfo(SysPkgNtpServerInfo *SysNtpServerInfoPtr);
    GMI_RESULT SvrSetTimeType(SysPkgDateTimeType *SysPkgDateTimePtr);
    GMI_RESULT SvrGetTimeType(SysPkgDateTimeType *SysPkgDateTimePtr);
    //device
    GMI_RESULT SvrGetDevinfo(SysPkgDeviceInfo *SysDeviceInfoPtr);
    GMI_RESULT SvrSetDeviceInfo(SysPkgDeviceInfo *SysDeviceInfoPtr);
    //showcfg
    GMI_RESULT SvrGetShowCfg(int StreamId, SysPkgShowCfg *SysShowCfgPtr);
    GMI_RESULT SvrSetShowCfg(int StreamId, SysPkgShowCfg *SysShowCfgPtr);
    //users
    GMI_RESULT SvrGetAllUsers(SysPkgUserInfo * UserInfoPtr, uint32_t UserInfoNum, uint32_t *RealUserNum);
    GMI_RESULT SvrSetUser(SysPkgUserInfo * UserInfoPtr);
    GMI_RESULT SvrDelUser(SysPkgUserInfo * UserInfoPtr);
    GMI_RESULT SvrGetUserNum(uint32_t *UserNumPtr);
    GMI_RESULT SvrSetNetworkPort(SysPkgNetworkPort *SysNetworkPort);
    GMI_RESULT SvrGetNetworkPort(SysPkgNetworkPort *SysNetworkPort);
    //system
    GMI_RESULT SvrSetSystemDefault(int32_t SysCtrlCmd, int32_t ConfigModule);
    GMI_RESULT GetCapabilities(int32_t CapabilityCategory, int32_t CapabilityBufferLength, char_t* Capability, SysPkgXml *SysCapabilities);
    GMI_RESULT GetWorkState(int32_t WorkStateBufferLength, char_t* WorkState, SysPkgXml *SysWorkState);
    GMI_RESULT ExcuteImportConfigFile(SysPkgConfigFileInfo *SysConfigFilePtr);

    //alarm
    GMI_RESULT SvrGetAlarmConfig(int32_t AlarmId, void_t *Parameter, size_t ParameterLength);
    GMI_RESULT SvrSetAlarmConfig(int32_t AlarmId, const void_t *Parameter, size_t ParameterLength);
    GMI_RESULT SvrGetAlmScheduleTime(SysPkgGetAlarmScheduleTime *SysGetAlarmScheduleTime, void_t *Parameter, size_t ParameterLength);
    GMI_RESULT SvrSetAlmScheduleTime(int32_t ScheduleId, const void_t *Parameter, size_t ParameterLength);
private:
    inline uint32_t TimeStamp(void)
    {
        uint32_t Stamp;
        struct timeval Now;
        gettimeofday(&Now, NULL);
        Stamp = Now.tv_sec * 1000 + Now.tv_usec/1000;
        return Stamp;
    }
private:
	//get version
    GMI_RESULT GetVersion(char_t FwVer[64]);
    //media param load
    GMI_RESULT MediaParamLoad(void);
    GMI_RESULT MiscInitial(void);
    GMI_RESULT MiscDeinitial(void);
    GMI_RESULT MediaParamUnLoad(void);
    GMI_RESULT MediaInitial(void);
    GMI_RESULT MediaDeinitial(void);
    GMI_RESULT PTZ_Initial();
    GMI_RESULT PTZ_Deinitial();
    //rtsp & sdk stream server monitor
    static void* ServerMonitorThread(void *Argument);
    void_t *ServerMonitor(void);
    GMI_RESULT StartStreamMonitor();
    GMI_RESULT StopStreamMonitor();
    //record zoom position
    static void* RecordZoomPosThread(void *Argument);
    void_t *RecordZoomPos(void);
    GMI_RESULT StopRecordZoomPos();
    GMI_RESULT StartRecordZoomPos();
    //maintain system
    static void* MaintainSystemThread(void *Argument);
    void_t *MaintainSystem(void);     
    GMI_RESULT OsalResourceDeinitial(void);
    GMI_RESULT OsalResourceInitial(void);
    int32_t FloatToInt(float_t Value);
    GMI_RESULT FactoryResetImaging(void);
    GMI_RESULT FactoryResetNetInfo(void);    
    GMI_RESULT RecreateVideoCodec(int32_t StreamId, VideoEncodeParam *EncParamPtr);
    void AudVidStreamIsExis(boolean_t *Exit);
    GMI_RESULT StartAudioEncode(void);
    GMI_RESULT StopAudioEncode(void);
    GMI_RESULT SvrSysAudioEncodeTransferMediaAudio(SysPkgAudioEncodeCfg *SysEncodeCfgPtr, AudioEncParam *AudioEncodePtr, AudioDecParam *AudioDecodePtr);
    GMI_RESULT SvrMeidaAudioTransferSysAudioEncode(AudioEncParam *AudioEncodePtr, AudioDecParam *AudioDecodePtr, SysPkgAudioEncodeCfg *SysEncodeCfgPtr);
    GMI_RESULT DestroyVideoCodec();
    GMI_RESULT RtspServerMonitor(boolean_t *Start);
    GMI_RESULT SdkServerMonitor(boolean_t *Start);
    GMI_RESULT FactoryResetStreamCombine();
    GMI_RESULT FactoryResetVideoEncode();
    GMI_RESULT OsdsInit(int32_t VideoCnt, VideoEncodeParam VideoEncodeParam[], VideoOSDParam VideoOsdParam[]);
    GMI_RESULT CheckShutterAndChange(int32_t VidInFps);
    GMI_RESULT GetCompileNum(int32_t *CompilesPtr);
private:
    //read & write lock
    pthread_rwlock_t                   m_Lock;
    boolean_t                          m_ThreadExitFlag;
    GMI_Thread                         m_ServerMonitorThread;
    boolean_t                          m_RecordZoomPosThreadExitFlag;
    GMI_Thread                         m_RecordZoomPosThread;
    GMI_Event                          m_RecordZoomNotify;
    GMI_Thread                         m_MaintainSystemThread;

    //objects
    ReferrencePtr<ConfigFileManager>   m_ConfigFileManagerPtr;
    ReferrencePtr<PtzControl>          m_PtzControlPtr;
    ReferrencePtr<StreamCenterClient>  m_StreamCenterClientPtr;
    ReferrencePtr<BoardManager>        m_BoardManagerPtr;
    ReferrencePtr<UserManager>         m_UserManagerPtr;
    ReferrencePtr<SdkStreamControl>    m_SdkStreamCtlPtr;
    ReferrencePtr<RtspStreamControl>   m_RtspStreamCtlPtr;
    ReferrencePtr<Alarm>               m_AlarmPtr;

    //stream center communication port
    uint16_t                           m_MediaLocalClientPort;
    uint16_t                           m_MediaLocalServerPort;
    uint16_t                           m_RTSP_ServerPort;
    size_t                             m_UDPSessionBufferSize;

    //stream center encode&decode parameter
    int32_t                                              m_VideoStreamNum;
    ReferrencePtr<int32_t, DefaultObjectsDeleter>        m_VideoStreamTypePtr;
    ReferrencePtr<VideoEncodeParam, DefaultObjectsDeleter>  m_VideoEncParamPtr;
    ReferrencePtr<FD_HANDLE, DefaultObjectsDeleter>      m_VideoCodecHandle;    
    ReferrencePtr<AudioEncParam>                         m_AudioEncParamPtr;
    ReferrencePtr<AudioDecParam>                         m_AudioDecParamPtr;    
    FD_HANDLE                                            m_VideoInOutHandle;
    FD_HANDLE                                            m_ImageHandle;
    FD_HANDLE                                            m_AutoFocusHandle;
    FD_HANDLE                                            m_ZoomHandle;
    FD_HANDLE                                            m_AudioEncHandle;
    FD_HANDLE                                            m_AudioDecHandle;

    //osd
    ReferrencePtr<VideoOSDParam, DefaultObjectsDeleter>  m_VideoOsdParamPtr;
    //image
    ReferrencePtr<ImageBaseParam>      m_ImageParamPtr;
    ReferrencePtr<ImageAdvanceParam>   m_ImageAdvanceParamPtr;
    ReferrencePtr<ImageWbParam>        m_ImageWbParamPtr;
    ReferrencePtr<ImageDnParam>        m_ImageDnParamPtr;
    //video source
    ReferrencePtr<SysPkgVideoSource>   m_VideoSourcePtr;
    //suppport ptz
    boolean_t                          m_SupportPtz;
    //ptz control  parameter
    UartCtrlCfg                        m_PtzUartCfg;
    //presets
    ReferrencePtr<SysPkgPresetInfo_Inner, DefaultObjectsDeleter>  m_PresetsInfo_InnerPtr;
    int32_t                            m_FocusMode;
    int32_t                            m_ZoomPos;
    //network port
    SysPkgNetworkPort                  m_SysNetWorkPort;
    //device info
    SysPkgDeviceInfo                   m_SysDeviceInfo;
    //capabilities
    ReferrencePtr<char_t, DefaultObjectsDeleter> m_CapabilitiesMessagePtr;
    SysPkgXml                          m_SysCapability;
    //ntp server info
    SysPkgNtpServerInfo                m_SysNtpServerInfo;
	//set vidin blocked flag
	boolean_t                          m_VidInBlocked;
    
};


#endif

