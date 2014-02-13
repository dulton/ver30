#include "log.h"
#include "threads.h"
#include "service_utilitly.h"
#include "service_ptz.h"
#include "soapH.h"
#include "sys_client.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"

//ptz 0 continous move stop flag
boolean_t  l_Ptz0ContinousMoveStop      = false;
int32_t    l_Ptz0ContinousMoveTimeout   = 0;
boolean_t  l_PtzContinousMoveThreadExit = false;

//ptz num
uint8_t    l_PTZ_Num  = 1;

//PTZ Configuration
PTZ_Configuration g_PTZ_Coniguration[DEFAULT_PTZ_NUM] =
{
    //PTZ_0
    {
        PTZ_NAME_0,        //name
        2,                 //use count
        PTZ_TOKEN_0,       //token
        PTZ_NODE_TOKEN_0,  //node token
        ABSOLUTE_PANTILT_POSITION_SPACE,    //absolute pantilt space
        ABSOLUTE_ZOOM_POSITION_SPACE,       //absolute zoom space
        RELATIVE_PANTILT_TRANSLATION_SPACE, //relative pantilt space
        RELATIVE_ZOOM_TRANSLATION_SPACE,    //relateive zoom space
        CONTINUOUS_PANTILT_VELOCITY_SPACE,  //continous pantilt space
        CONTINUOUS_ZOOM_VELOCITY_SPACE,     //coninous zoom space
        //default ptz speed
        {
            { PANTILT_XRAGE, PANTILT_YRAGE, GENERIC_SPEED_SPACE }, //PanTilt
            { ZOOM_XRAGE, GENERIC_ZOOM_SPEED_SPACE}   //Zoom
        },
        DEFAULT_PTZ_TIMEOUT,                         //default ptz timeout
        {{ ABSOLUTE_PANTILT_POSITION_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX,  PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}}, //PanTilt limits
        {{ ABSOLUTE_ZOOM_POSITION_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}} //zoom limits
    }
};


//PTZ Configuration options
static PTZ_ConfigurationOptions l_PTZ_ConfigurationOptions[DEFAULT_PTZ_NUM] =
{
    //PTZ0
    {
        //AbsolutePanTiltPositionSpace
        {{ABSOLUTE_PANTILT_POSITION_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX, PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}, {ABSOLUTE_PANTILT_DIGITAL_POSITION_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX, PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}}
        ,
        //AbsoluteZoomPositionSpace
        {{ABSOLUTE_ZOOM_POSITION_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}, {ABSOLUTE_ZOOM_DIGITAL_POSITON_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}}
        ,
        //RelativePanTiltTranslationSpace
        {{RELATIVE_PANTILT_TRANSLATION_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX, PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}, {RELATIVE_PANTILT_DIGITAL_TRANSLATION_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX, PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}}
        ,
        //RelativeZoomTranslationSpace
        {{RELATIVE_ZOOM_TRANSLATION_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}, {RELATIVE_ZOOM_DIGITAL_TRANSLATION_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}}
        ,
        //ContinuousPanTiltVelocitySpace
        {{CONTINUOUS_PANTILT_VELOCITY_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX, PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}, {CONTINUOUS_PANTILT_DIGITAL_SPACE, CONTINOUS_DIGITIAL_PANTILT_XRANGE_MIN, CONTINOUS_DIGITIAL_PANTILT_XRANGE_MAX, CONTINOUS_DIGITIAL_PANTILT_YRANGE_MIN, CONTINOUS_DIGITIAL_PANTILT_YRANGE_MAX}}
        ,
        //ContinuousZoomVelocitySpace
        {{CONTINUOUS_ZOOM_VELOCITY_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}, {CONTINUOUS_ZOOM_DIGITAL_SPACE, CONTINOUS_DIGITIAL_ZOOM_XRANGE_MIN, CONTINOUS_DIGITIAL_ZOOM_XRANGE_MAX}}
        ,
        //PanTiltSpeedSpace
        {GENERIC_SPEED_SPACE, PANTILT_XRAGE_MIN, PANTILT_YRAGE_MAX}
        ,
        //s_ZoomSpeedSpace
        {GENERIC_ZOOM_SPEED_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}
        ,
        {DEFAULT_MIN_PTZ_TIMEOUT, DEFAULT_MAX_PTZ_TIMEOUT}
    }
};


//ptz node
static PTZ_Node  l_PTZ_Node[DEFAULT_PTZ_NUM] =
{
    {
        PTZ_NODE_TOKEN_0,
        PTZ_NODE_NAME_0,
        //AbsolutePanTiltPositionSpace
        {{ABSOLUTE_PANTILT_POSITION_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX, PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}, {ABSOLUTE_PANTILT_DIGITAL_POSITION_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX, PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}}
        ,
        //AbsoluteZoomPositionSpace
        {{ABSOLUTE_ZOOM_POSITION_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}, {ABSOLUTE_ZOOM_DIGITAL_POSITON_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}}
        ,
        //RelativePanTiltTranslationSpace
        {{RELATIVE_PANTILT_TRANSLATION_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX, PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}, {RELATIVE_PANTILT_DIGITAL_TRANSLATION_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX, PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}}
        ,
        //RelativeZoomTranslationSpace
        {{RELATIVE_ZOOM_TRANSLATION_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}, {RELATIVE_ZOOM_DIGITAL_TRANSLATION_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}}
        ,
        //ContinuousPanTiltVelocitySpace
        {{CONTINUOUS_PANTILT_VELOCITY_SPACE, PANTILT_XRAGE_MIN, PANTILT_XRAGE_MAX, PANTILT_YRAGE_MIN, PANTILT_YRAGE_MAX}, {CONTINUOUS_PANTILT_DIGITAL_SPACE, CONTINOUS_DIGITIAL_PANTILT_XRANGE_MIN, CONTINOUS_DIGITIAL_PANTILT_XRANGE_MAX, CONTINOUS_DIGITIAL_PANTILT_YRANGE_MIN, CONTINOUS_DIGITIAL_PANTILT_YRANGE_MAX}}
        ,
        //ContinuousZoomVelocitySpace
        {{CONTINUOUS_ZOOM_VELOCITY_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}, {CONTINUOUS_ZOOM_DIGITAL_SPACE, CONTINOUS_DIGITIAL_ZOOM_XRANGE_MIN, CONTINOUS_DIGITIAL_ZOOM_XRANGE_MAX}}
        ,
        //PanTiltSpeedSpace
        {GENERIC_SPEED_SPACE, PANTILT_XRAGE_MIN, PANTILT_YRAGE_MAX}
        ,
        //s_ZoomSpeedSpace
        {GENERIC_ZOOM_SPEED_SPACE, ZOOM_XRAGE_MIN, ZOOM_XRAGE_MAX}
        ,//preset num
        MAX_PTZ_PRESET_NUM
        ,//true
        xsd__boolean__true_
    }
};

uint32_t             l_PtzPresetTotal;
PTZ_Preset          *l_PtzPreset;

//func declaration
void *Ptz_ContinousMoveStopThread(void);


GMI_RESULT PtzTimeoutProcessStart(void)
{
    THREAD_TYPE Tid;

    l_PtzContinousMoveThreadExit = false;
    THREAD_CREATE(&Tid, (void*(*)(void*))Ptz_ContinousMoveStopThread, (void*)NULL);

    return GMI_SUCCESS;
}


GMI_RESULT PtzTimeoutProcessStop(void)
{
    l_PtzContinousMoveThreadExit = true;

    while (l_PtzContinousMoveThreadExit != false)
    {
        usleep(200*1000);
    }

    return GMI_SUCCESS;
}


GMI_RESULT __tptz__Initialize()
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t SessionId  = 0;
    uint32_t AuthValue  = 0;
    int32_t  MaxPresetNum;
    boolean_t Fail = false;
    GMI_RESULT Result = GMI_SUCCESS;
    //SysPkgPtzPresetInfo *SysPtzPresetInfoPtr = NULL;

    do
    {
        PtzTimeoutProcessStart();

        Result = SysGetMaxPresetNum(SessionId, AuthValue, &MaxPresetNum);
        if (FAILED(Result))
        {
            ONVIF_ERROR("get max preset num fail, Result = 0x%lx\n", Result);
            return Result;
        }

#if 0
        SysPtzPresetInfoPtr = (SysPkgPtzPresetInfo*)malloc(sizeof(SysPkgPtzPresetInfo)*MaxPresetNum);
        if (NULL == SysPtzPresetInfoPtr)
        {
            ONVIF_ERROR("malloc fail\n");
            return GMI_OUT_OF_MEMORY;
        }

        int32_t RspCnt = 0;
        Result = SysGetPtzPresetInfo(SessionId, AuthValue, MaxPresetNum, SysPtzPresetInfoPtr, &RspCnt);
        if (FAILED(Result))
        {
            ONVIF_ERROR("get ptz presetinfo fail, Result = 0x%lx\n", Result);
            if (NULL != SysPtzPresetInfoPtr)
            {
                free(SysPtzPresetInfoPtr);
                SysPtzPresetInfoPtr = NULL;
            }
            return Result;
        }
#endif
        l_PtzPreset = (PTZ_Preset*)malloc(sizeof(PTZ_Preset)*MaxPresetNum);
        if (FAILED(Result))
        {
            return GMI_OUT_OF_MEMORY;
        }
        memset(l_PtzPreset, 0, sizeof(PTZ_Preset)*MaxPresetNum);

        l_PtzPresetTotal = MaxPresetNum;
        for (int32_t i = 0; i < MaxPresetNum; i++)
        {
            boolean_t Setted = false;
            char_t    PresetName[128];
            Result = SysSearchPtzPresetInfo(SessionId, AuthValue, i+1, &Setted, PresetName);
            if (FAILED(Result))
            {
                Fail = true;
                ONVIF_ERROR("SysSearchPtzPresetInfo %d fail\n", i+1);
                break;
            }
            memcpy(l_PtzPreset[i].s_Token, PresetName, sizeof(l_PtzPreset[i].s_Token));
            l_PtzPreset[i].s_Index = i+1;
            l_PtzPreset[i].s_Setted = Setted;
        }

        if (Fail)
        {
            break;
        }
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return GMI_SUCCESS;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return Result;
}


void __tptz__Deinitialize()
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);


    if (NULL != l_PtzPreset)
    {
        free(l_PtzPreset);
        l_PtzPreset = NULL;
    }

    PtzTimeoutProcessStop();

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s normal Out.........\n", __func__);
    ONVIF_INFO("%s normal out.........\n", __func__);
    return;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetServiceCapabilities(struct soap *soap_ptr, struct _tptz__GetServiceCapabilities *tptz__GetServiceCapabilities, struct _tptz__GetServiceCapabilitiesResponse *tptz__GetServiceCapabilitiesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurations(struct soap *soap_ptr, struct _tptz__GetConfigurations *tptz__GetConfigurations, struct _tptz__GetConfigurationsResponse *tptz__GetConfigurationsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint8_t             Id                   = 0;
    uint8_t             PTZ_Num              = 0;
    PTZ_Configuration  *PTZ_ConfigurationPtr = NULL;
    GMI_RESULT          Result               = GMI_SUCCESS;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        //get device capabilitis from sys_control_server, acquire PTZ Num etc.
        PTZ_Num   = DEFAULT_PTZ_NUM;
        l_PTZ_Num = PTZ_Num;

        tptz__GetConfigurationsResponse->__sizePTZConfiguration = PTZ_Num;
        tptz__GetConfigurationsResponse->PTZConfiguration = (struct tt__PTZConfiguration *)soap_malloc_zero( soap_ptr, tptz__GetConfigurationsResponse->__sizePTZConfiguration * sizeof(struct tt__PTZConfiguration) );
        if (NULL == tptz__GetConfigurationsResponse->PTZConfiguration)
        {
            ONVIF_ERROR("PTZConfiguration soap_malloc_zero failed\n");
            break;
        }

        //soap_malloc
        for (Id = 0; Id < PTZ_Num; Id++)
        {
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed              = (struct tt__PTZSpeed *)soap_malloc_zero( soap_ptr, sizeof(struct tt__PTZSpeed) );
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed->PanTilt     = (struct tt__Vector2D *)soap_malloc_zero( soap_ptr, sizeof(struct tt__Vector2D) );
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed->Zoom        = (struct tt__Vector1D *)soap_malloc_zero( soap_ptr, sizeof(struct tt__Vector1D) );

            tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits                = (struct tt__PanTiltLimits*)soap_malloc_zero(soap_ptr, sizeof(struct tt__PanTiltLimits));
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range         = (struct tt__Space2DDescription*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Space2DDescription));
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range->XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range->YRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].ZoomLimits                   = (struct tt__ZoomLimits*)soap_malloc_zero(soap_ptr, sizeof(struct tt__ZoomLimits));
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].ZoomLimits->Range            = (struct tt__Space1DDescription*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Space1DDescription));
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].ZoomLimits->Range->XRange    = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZTimeout            = (LONG64*)soap_malloc_zero(soap_ptr, sizeof(LONG64));
            if (NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed
                    || NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed->PanTilt
                    || NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed->Zoom
                    || NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZTimeout
                    || NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits
                    || NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range
                    || NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range->XRange
                    || NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range->YRange
                    || NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].ZoomLimits
                    || NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].ZoomLimits->Range
                    || NULL == tptz__GetConfigurationsResponse->PTZConfiguration[Id].ZoomLimits->Range->XRange)
            {
                ONVIF_ERROR("PTZConfiguration[%d] members soap_malloc_zero failed\n", Id);
                break;
            }
        }

        //fillout the struct of tptz__GetConfigurationsResponse
        PTZ_ConfigurationPtr = g_PTZ_Coniguration;
        for (Id = 0; Id < PTZ_Num; Id++)
        {
            //name & token
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].Name      = soap_strdup( soap_ptr, PTZ_ConfigurationPtr->s_Name);
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].UseCount  = PTZ_ConfigurationPtr->s_UseCount;
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].token     = soap_strdup( soap_ptr, PTZ_ConfigurationPtr->s_Token);
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].NodeToken = soap_strdup( soap_ptr, PTZ_ConfigurationPtr->s_NodeToken);

            //absolute & relative & continous spaces
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultAbsolutePantTiltPositionSpace   = soap_strdup(soap_ptr, PTZ_ConfigurationPtr->s_DefaultAbsolutePantTiltPositionSpace);
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultAbsoluteZoomPositionSpace       = soap_strdup(soap_ptr, PTZ_ConfigurationPtr->s_DefaultAbsoluteZoomPositionSpace);
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultRelativePanTiltTranslationSpace = soap_strdup(soap_ptr, PTZ_ConfigurationPtr->s_DefaultRelativePanTiltTranslationSpace);
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultRelativeZoomTranslationSpace    = soap_strdup(soap_ptr, PTZ_ConfigurationPtr->s_DefaultRelativeZoomTranslationSpace);
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultContinuousPanTiltVelocitySpace  = soap_strdup(soap_ptr, PTZ_ConfigurationPtr->s_DefaultContinuousPanTiltVelocitySpace);
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultAbsoluteZoomPositionSpace       = soap_strdup(soap_ptr, PTZ_ConfigurationPtr->s_DefaultContinuousZoomVelocitySpace);

            //default PTZ speed
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed->PanTilt->x       = PTZ_ConfigurationPtr->s_PTZ_Speed.s_PanTilt.x;
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed->PanTilt->y       = PTZ_ConfigurationPtr->s_PTZ_Speed.s_PanTilt.y;
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed->Zoom->x          = PTZ_ConfigurationPtr->s_PTZ_Speed.s_Zoom.x;
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed->PanTilt->space   = soap_strdup(soap_ptr, PTZ_ConfigurationPtr->s_PTZ_Speed.s_PanTilt.space);
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZSpeed->Zoom->space      = soap_strdup(soap_ptr, PTZ_ConfigurationPtr->s_PTZ_Speed.s_Zoom.space);

            //PTZ Timeout
            *(tptz__GetConfigurationsResponse->PTZConfiguration[Id].DefaultPTZTimeout)              = PTZ_ConfigurationPtr->s_DefaultTimeout;

            //PanTilt & Zoom limits
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range->URI         = soap_strdup(soap_ptr, PTZ_ConfigurationPtr->s_PanTiltLimits.s_Range.s_URI);
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range->XRange->Min = PTZ_ConfigurationPtr->s_PanTiltLimits.s_Range.s_XRangeMin;
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range->XRange->Max = PTZ_ConfigurationPtr->s_PanTiltLimits.s_Range.s_XRangeMax;
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range->YRange->Min = PTZ_ConfigurationPtr->s_PanTiltLimits.s_Range.s_YRangeMin;
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].PanTiltLimits->Range->YRange->Max = PTZ_ConfigurationPtr->s_PanTiltLimits.s_Range.s_YRangeMax;
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].ZoomLimits->Range->URI            = soap_strdup(soap_ptr, PTZ_ConfigurationPtr->s_ZoomLimits.s_Range.s_URI);
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].ZoomLimits->Range->XRange->Min    = PTZ_ConfigurationPtr->s_ZoomLimits.s_Range.s_XRangeMin;
            tptz__GetConfigurationsResponse->PTZConfiguration[Id].ZoomLimits->Range->XRange->Max    = PTZ_ConfigurationPtr->s_ZoomLimits.s_Range.s_XRangeMax;

            PTZ_ConfigurationPtr++;
        }
        PTZ_ConfigurationPtr = NULL;
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresets(struct soap *soap_ptr, struct _tptz__GetPresets *tptz__GetPresets, struct _tptz__GetPresetsResponse *tptz__GetPresetsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);

    do
    {
        //user auth
        GMI_RESULT Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        if (NULL == tptz__GetPresets->ProfileToken)
        {
            ONVIF_ERROR("ProfileToken %s not exist\n", tptz__GetPresets->ProfileToken);
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "NoPTZProfile", "The requested profile token does not reference a PTZ configuration");
            break;
        }
        ONVIF_INFO("ProfileToken      = %s\n", tptz__GetPresets->ProfileToken);

        int32_t Count = 0;
        uint32_t Id = 0;
        PTZ_Preset *PtzPreset = l_PtzPreset;
        for (Id = 0; Id < l_PtzPresetTotal; Id++)
        {
            if (PtzPreset[Id].s_Setted)
            {
                printf("Id%d %s\n", Id, PtzPreset[Id].s_Token);
                Count++;
            }
        }

        if (0 < Count)
        {
            tptz__GetPresetsResponse->__sizePreset = Count;
            tptz__GetPresetsResponse->Preset       = (struct tt__PTZPreset*)soap_malloc_zero(soap_ptr, tptz__GetPresetsResponse->__sizePreset * sizeof(struct tt__PTZPreset));
            for (Id = 0; Id < (uint32_t)tptz__GetPresetsResponse->__sizePreset; Id++)
            {
                tptz__GetPresetsResponse->Preset[Id].token = (char_t*)soap_malloc_zero(soap_ptr, TOKEN_LENGTH);
                tptz__GetPresetsResponse->Preset[Id].Name  = (char_t*)soap_malloc_zero(soap_ptr, TOKEN_LENGTH);
            }

            int32_t i;
            for (Id = 0, i = 0; Id < l_PtzPresetTotal && i < Count; Id++)
            {
                if (PtzPreset[Id].s_Setted
                	&& 0 < strlen(PtzPreset[Id].s_Token))
                {
                    strcpy(tptz__GetPresetsResponse->Preset[i].token, PtzPreset[Id].s_Token);                 
                    strcpy(tptz__GetPresetsResponse->Preset[i].Name, PtzPreset[Id].s_Token);                
                    i++;
                }
            }
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Normal Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetPreset(struct soap *soap_ptr, struct _tptz__SetPreset *tptz__SetPreset, struct _tptz__SetPresetResponse *tptz__SetPresetResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);

    do
    {
        //user auth
        GMI_RESULT Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        if (NULL == tptz__SetPreset->ProfileToken)
        {
            ONVIF_ERROR("PresetToken %s not exist\n", tptz__SetPreset->PresetToken);
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "NoPTZProfile", "The requested profile token does not reference a PTZ configuration");
            break;
        }
        ONVIF_INFO("ProfileToken      = %s\n", tptz__SetPreset->ProfileToken);

        uint16_t      SessionId  = 0;
        uint32_t      AuthValue  = 0;
        boolean_t     TokenExist = false;
        boolean_t     NameExist  = false;
        boolean_t     Fail       = false;
        SysPkgPtzPresetInfo SysPtzPresetInfo;

        if (NULL != tptz__SetPreset->PresetToken)
        {
            ONVIF_INFO("PresetToken       = %s\n", tptz__SetPreset->PresetToken);
            TokenExist = true;
        }
        if (NULL != tptz__SetPreset->PresetName)
        {
            ONVIF_INFO("PresetName        = %s\n", tptz__SetPreset->PresetName);
            NameExist = true;
        }

        //overwrite
        if (TokenExist)
        {
            uint32_t Id;
            PTZ_Preset *PtzPresetPtr = l_PtzPreset;

            for (Id = 0; Id < l_PtzPresetTotal; Id++)
            {
                if (strlen(tptz__SetPreset->PresetToken) == strlen(PtzPresetPtr[Id].s_Token)
                	&& 0 == strcmp(tptz__SetPreset->PresetToken, PtzPresetPtr[Id].s_Token))
                {
                    //overwrite
                    SysPtzPresetInfo.s_PtzId = 1;
                    SysPtzPresetInfo.s_PresetIndex = PtzPresetPtr[Id].s_Index;
                    strcpy(SysPtzPresetInfo.s_PresetName, PtzPresetPtr[Id].s_Token);
                    Result = SysSetPtzPresetInfo(SessionId, AuthValue, &SysPtzPresetInfo);
                    if (FAILED(Result))
                    {

                        ONVIF_ERROR("set preset to system fail, Result = 0x%lx\n", Result);
                        Fail = true;
                        break;
                    }
                    break;
                }
            }
            if (Fail)
            {
                break;
            }

            if (Id >= l_PtzPresetTotal)
            {
                ONVIF_ERROR("PresetToken %s not exist\n", tptz__SetPreset->PresetToken);
                ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:NoToken", "The requested preset token does not exist");
                break;
            }

            tptz__SetPresetResponse->PresetToken = soap_strdup(soap_ptr, tptz__SetPreset->PresetToken);
        }
        //create
        else if (NameExist)
        {
            if (strlen(tptz__SetPreset->PresetName) >= TOKEN_LENGTH)
            {
                ONVIF_ERROR("PresetName %s too long\n", tptz__SetPreset->PresetName);
                ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:InvalidPresetName", "The PresetName is either too long or contains invalid characters");
                break;
            }

            uint32_t Id;
            PTZ_Preset *PtzPresetPtr = l_PtzPreset;
            for (Id = 0; Id < l_PtzPresetTotal; Id++)
            {
                if (strlen(tptz__SetPreset->PresetName) == strlen(PtzPresetPtr[Id].s_Name)
                        && 0 == strcmp(tptz__SetPreset->PresetName, PtzPresetPtr[Id].s_Name))
                {
                    ONVIF_ERROR("PresetName %s have exist\n", tptz__SetPreset->PresetName);
                    //ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:PresetExist", "The requested name already exist for another preset");
                    //Fail = true;
                    //no empty presets been used when this device have been interfaced with many NVR . this method solve this problem.guoqiang.lu 14/1/23
                    //overwrite
                    PtzPresetPtr[Id].s_Setted = true;                    
                    strcpy(PtzPresetPtr[Id].s_Token, tptz__SetPreset->PresetName);
                    SysPtzPresetInfo.s_PtzId = 1;
                    SysPtzPresetInfo.s_PresetIndex = PtzPresetPtr[Id].s_Index;
                    strcpy(SysPtzPresetInfo.s_PresetName, PtzPresetPtr[Id].s_Token);
                    Result = SysSetPtzPresetInfo(SessionId, AuthValue, &SysPtzPresetInfo);
                    if (FAILED(Result))
                    {
                        ONVIF_ERROR("set preset to system fail, Result = 0x%lx\n", Result);
                        Fail = true;
                        break;
                    }
                    break;
                }
            }
            if (Fail)
            {
                break;
            }

            if (Id >= l_PtzPresetTotal)
            {
                uint32_t i;
                PTZ_Preset *PtzPresetPtr = l_PtzPreset;
                for (i = 0; i < l_PtzPresetTotal; i++)
                {
                    boolean_t Setted = 	PtzPresetPtr[i].s_Setted;
                    if (!Setted)
                    {
                        ONVIF_INFO("index %d, name %s, set preset\n", PtzPresetPtr[i].s_Index, tptz__SetPreset->PresetName);
                        SysPtzPresetInfo.s_PtzId = 1;
                        SysPtzPresetInfo.s_PresetIndex = PtzPresetPtr[i].s_Index;
                        strcpy(SysPtzPresetInfo.s_PresetName, tptz__SetPreset->PresetName);
                        Result = SysSetPtzPresetInfo(SessionId, AuthValue, &SysPtzPresetInfo);
                        if (FAILED(Result))
                        {

                            ONVIF_ERROR("set preset to system fail, Result = 0x%lx\n", Result);
                            Fail = true;
                            break;
                        }
                        strcpy(PtzPresetPtr[i].s_Name, tptz__SetPreset->PresetName);
                        strcpy(PtzPresetPtr[i].s_Token, tptz__SetPreset->PresetName);
                        PtzPresetPtr[i].s_Setted = true;
                        break;
                    }
                }
                if (Fail)
                {
                    break;
                }
                if (i >= l_PtzPresetTotal)
                {
                    ONVIF_ERROR("no free spaces to create new preset %s\n", tptz__SetPreset->PresetName);
                    break;
                }
            }           

            tptz__SetPresetResponse->PresetToken = soap_strdup(soap_ptr, tptz__SetPreset->PresetName);
        }
        else
        {
            ONVIF_ERROR("no token, no name, support\n");
            uint32_t i;
            PTZ_Preset *PtzPresetPtr = l_PtzPreset;
            for (i = 0; i < l_PtzPresetTotal; i++)
            {
                boolean_t Setted = PtzPresetPtr[i].s_Setted;
                if (!Setted)
                {
                    SysPtzPresetInfo.s_PtzId = 1;
                    SysPtzPresetInfo.s_PresetIndex = PtzPresetPtr[i].s_Index;
                    sprintf(SysPtzPresetInfo.s_PresetName, "%d", PtzPresetPtr[i].s_Index);
                    Result = SysSetPtzPresetInfo(SessionId, AuthValue, &SysPtzPresetInfo);
                    if (FAILED(Result))
                    {
                        ONVIF_ERROR("set preset to system fail, Result = 0x%lx\n", Result);
                        Fail = true;
                        break;
                    }
                    strcpy(PtzPresetPtr[i].s_Name, SysPtzPresetInfo.s_PresetName);
                    strcpy(PtzPresetPtr[i].s_Token, SysPtzPresetInfo.s_PresetName);
                    PtzPresetPtr[i].s_Setted = true;
                    break;
                }
            }
            if (Fail)
            {
                break;
            }
            tptz__SetPresetResponse->PresetToken = soap_strdup(soap_ptr, PtzPresetPtr[i].s_Token);
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Normal Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePreset(struct soap *soap_ptr, struct _tptz__RemovePreset *tptz__RemovePreset, struct _tptz__RemovePresetResponse *tptz__RemovePresetResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    do
    {
        uint16_t   SessionId = 0;
        uint32_t   AuthValue = 0;

        //user auth
        GMI_RESULT Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        if (NULL == tptz__RemovePreset->ProfileToken)
        {
            ONVIF_ERROR("ProfileToken %s not exist\n", tptz__RemovePreset->ProfileToken);
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "NoPTZProfile", "The requested profile token does not reference a PTZ configuration");
            break;
        }
        ONVIF_INFO("ProfileToken      = %s\n", tptz__RemovePreset->ProfileToken);

        if (NULL == tptz__RemovePreset->PresetToken)
        {
            ONVIF_ERROR("PresetToken %s not exist\n", tptz__RemovePreset->PresetToken);
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:NoToken", "The requested preset token does not exist");
            break;
        }

        uint32_t      Id;
        boolean_t     Fail = false;
        SysPkgPtzCtrl PtzCtrlCmd = {0};
        PTZ_Preset   *PtzPresetPtr = l_PtzPreset;
        for (Id = 0; Id < l_PtzPresetTotal; Id++)
        {
            if (strlen(tptz__RemovePreset->PresetToken) == strlen(PtzPresetPtr[Id].s_Token)
            	&& 0 == strcmp(tptz__RemovePreset->PresetToken, PtzPresetPtr[Id].s_Token))
            {
                PtzCtrlCmd.s_PtzCmd   = 1;
                PtzCtrlCmd.s_PtzCmd   = SYS_PTZCMD_CLEARPRESET;
                PtzCtrlCmd.s_Param[0] = PtzPresetPtr[Id].s_Index;
                Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd);
                if (FAILED(Result))
                {
                    ONVIF_ERROR("SYS_PTZCMD_CLEARPRESET failed\n");
                    Fail = true;
                    break;
                }
                PtzPresetPtr[Id].s_Setted = false;
                memset(PtzPresetPtr[Id].s_Name, 0, sizeof(PtzPresetPtr[Id].s_Name));
                memset(PtzPresetPtr[Id].s_Token, 0, sizeof(PtzPresetPtr[Id].s_Token));
                break;
            }
        }

        if (Fail)
        {
            break;
        }

        if (Id >= l_PtzPresetTotal)
        {
            ONVIF_ERROR("PresetToken %s not exist\n", tptz__RemovePreset->PresetToken);
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:NoToken", "The requested preset token does not exist");
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s Out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoPreset(struct soap *soap_ptr, struct _tptz__GotoPreset *tptz__GotoPreset, struct _tptz__GotoPresetResponse *tptz__GotoPresetResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    do
    {
        uint16_t   SessionId = 0;
        uint32_t   AuthValue = 0;

        //user auth
        GMI_RESULT Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        if (NULL == tptz__GotoPreset->ProfileToken)
        {
            ONVIF_ERROR("ProfileToken %s not exist\n", tptz__GotoPreset->ProfileToken);
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "NoPTZProfile", "The requested profile token does not reference a PTZ configuration");
            break;
        }
        ONVIF_INFO("ProfileToken      = %s\n", tptz__GotoPreset->ProfileToken);

        if (NULL == tptz__GotoPreset->PresetToken)
        {
            ONVIF_ERROR("PresetToken %s not exist\n", tptz__GotoPreset->PresetToken);
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:NoToken", "The requested preset token does not exist");
            break;
        }

        uint32_t   Id;
        boolean_t  Fail = false;
        SysPkgPtzCtrl PtzCtrlCmd = {0};
        PTZ_Preset *PtzPresetPtr = l_PtzPreset;
        for (Id = 0; Id < l_PtzPresetTotal; Id++)
        {
            if (strlen(tptz__GotoPreset->PresetToken) == strlen(PtzPresetPtr[Id].s_Token)
            	&& 0 == strcmp(tptz__GotoPreset->PresetToken, PtzPresetPtr[Id].s_Token))
            {
                ONVIF_INFO("Token %s\n", PtzPresetPtr[Id].s_Token);
                PtzCtrlCmd.s_PtzCmd   = 1;
                PtzCtrlCmd.s_PtzCmd   = SYS_PTZCMD_GOTOPRESET;
                PtzCtrlCmd.s_Param[0] = PtzPresetPtr[Id].s_Index;
                Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd);
                if (FAILED(Result))
                {
                    ONVIF_ERROR("goto preset fail, Result = 0x%lx\n", Result);
                    Fail = true;
                    break;
                }
                break;
            }
        }
        if (Fail)
        {
            break;
        }

        if (Id >= l_PtzPresetTotal)
        {
            ONVIF_ERROR("PresetToken %s not exist\n", tptz__GotoPreset->PresetToken);
            ONVIF_Fault(soap_ptr, "ter:InvalidArgVal", "ter:NoToken", "The requested preset token does not exist");
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s Out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetStatus(struct soap *soap_ptr, struct _tptz__GetStatus *tptz__GetStatus, struct _tptz__GetStatusResponse *tptz__GetStatusResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfiguration(struct soap *soap_ptr, struct _tptz__GetConfiguration *tptz__GetConfiguration, struct _tptz__GetConfigurationResponse *tptz__GetConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNodes(struct soap *soap_ptr, struct _tptz__GetNodes *tptz__GetNodes, struct _tptz__GetNodesResponse *tptz__GetNodesResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    int32_t                    Id                          = 0;
    int32_t                    Sp                          = 0;
    int32_t                    PTZNum                      = 0;
    int32_t                    SpaceNum                    = 0;
    GMI_RESULT				   Result					   = GMI_SUCCESS;
    PTZ_Node                  *PTZ_NodePtr                 = NULL;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }
        //should get from sys control server.
        //get ptz num
        PTZNum      = DEFAULT_PTZ_NUM;
        SpaceNum    =  DEFAULT_PTZ_NUM;//DEFAULT_PTZ_SPACE_NUM;
        PTZ_NodePtr = l_PTZ_Node;

        tptz__GetNodesResponse->__sizePTZNode = PTZNum;
        tptz__GetNodesResponse->PTZNode       = (struct tt__PTZNode*)soap_malloc_zero(soap_ptr, tptz__GetNodesResponse->__sizePTZNode*sizeof(struct tt__PTZNode));

        for (Id = 0; Id < PTZNum; Id++)
        {
            tptz__GetNodesResponse->PTZNode[Id].Name                   = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_Name);
            tptz__GetNodesResponse->PTZNode[Id].token                  = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_token);
            tptz__GetNodesResponse->PTZNode[Id].MaximumNumberOfPresets = PTZ_NodePtr[Id].s_MaximumNumberOfPresets;
            tptz__GetNodesResponse->PTZNode[Id].HomeSupported          = PTZ_NodePtr[Id].s_HomeSupported;

            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces =(struct tt__PTZSpaces*)soap_malloc_zero(soap_ptr, sizeof(struct tt__PTZSpaces));
            //AbsolutePanTiltPositionSpace
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeAbsolutePanTiltPositionSpace = SpaceNum;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsolutePanTiltPositionSpace =(struct tt__Space2DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeAbsolutePanTiltPositionSpace*sizeof(struct tt__Space2DDescription));
            for (Sp = 0; Sp < tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeAbsolutePanTiltPositionSpace; Sp++)
            {
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_AbsolutePanTiltPositionSpace[Sp].s_URI);
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].YRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_AbsolutePanTiltPositionSpace[Sp].s_XRangeMin;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_AbsolutePanTiltPositionSpace[Sp].s_XRangeMax;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].YRange->Min = PTZ_NodePtr[Id].s_AbsolutePanTiltPositionSpace[Sp].s_YRangeMin;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].YRange->Max = PTZ_NodePtr[Id].s_AbsolutePanTiltPositionSpace[Sp].s_YRangeMax;
            }
            //AbsoluteZoomPositionSpace
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeAbsoluteZoomPositionSpace = SpaceNum;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsoluteZoomPositionSpace =(struct tt__Space1DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeAbsoluteZoomPositionSpace*sizeof(struct tt__Space2DDescription));
            for (Sp = 0; Sp < tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeAbsoluteZoomPositionSpace; Sp++)
            {
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsoluteZoomPositionSpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_AbsoluteZoomPositionSpace[Sp].s_URI);
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsoluteZoomPositionSpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsoluteZoomPositionSpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_AbsoluteZoomPositionSpace[Sp].s_XRangeMin;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->AbsoluteZoomPositionSpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_AbsoluteZoomPositionSpace[Sp].s_XRangeMax;
            }

            //RelativePanTiltTranslationSpace
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeRelativePanTiltTranslationSpace = SpaceNum;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativePanTiltTranslationSpace =(struct tt__Space2DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeRelativePanTiltTranslationSpace*sizeof(struct tt__Space2DDescription));
            for (Sp = 0; Sp < tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeRelativePanTiltTranslationSpace; Sp++)
            {
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_URI);
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].YRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_XRangeMin;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_XRangeMax;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].YRange->Min = PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_YRangeMin;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].YRange->Max = PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_YRangeMax;
            }
            //RelativeZoomTranslationSpace
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeRelativeZoomTranslationSpace = SpaceNum;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativeZoomTranslationSpace =(struct tt__Space1DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeRelativeZoomTranslationSpace*sizeof(struct tt__Space2DDescription));
            for (Sp = 0; Sp < tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeRelativeZoomTranslationSpace; Sp++)
            {
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativeZoomTranslationSpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_URI);
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativeZoomTranslationSpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativeZoomTranslationSpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_RelativeZoomTranslationSpace[Sp].s_XRangeMin;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->RelativeZoomTranslationSpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_RelativeZoomTranslationSpace[Sp].s_XRangeMax;
            }

            //ContinuousPanTiltVelocitySpace
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace = SpaceNum;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace =(struct tt__Space2DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace*sizeof(struct tt__Space2DDescription));
            for (Sp = 0; Sp < tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace; Sp++)
            {
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_ContinuousPanTiltVelocitySpace[Sp].s_URI);
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].YRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_ContinuousPanTiltVelocitySpace[Sp].s_XRangeMin;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_ContinuousPanTiltVelocitySpace[Sp].s_XRangeMax;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].YRange->Min = PTZ_NodePtr[Id].s_ContinuousPanTiltVelocitySpace[Sp].s_YRangeMin;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].YRange->Max = PTZ_NodePtr[Id].s_ContinuousPanTiltVelocitySpace[Sp].s_YRangeMax;
            }

            //ContinuousZoomVelocitySpace
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeContinuousZoomVelocitySpace = SpaceNum;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousZoomVelocitySpace =(struct tt__Space1DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace*sizeof(struct tt__Space2DDescription));
            for (Sp = 0; Sp < tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeContinuousZoomVelocitySpace; Sp++)
            {
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousZoomVelocitySpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_ContinuousZoomVelocitySpace[Sp].s_URI);
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousZoomVelocitySpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousZoomVelocitySpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_ContinuousZoomVelocitySpace[Sp].s_XRangeMin;
                tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ContinuousZoomVelocitySpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_ContinuousZoomVelocitySpace[Sp].s_XRangeMax;
            }

            //speed space
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizePanTiltSpeedSpace   = 1;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->PanTiltSpeedSpace         = (struct tt__Space1DDescription* )soap_malloc_zero(soap_ptr, tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizePanTiltSpeedSpace*sizeof(struct tt__Space1DDescription));
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->PanTiltSpeedSpace->XRange = (struct tt__FloatRange *)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeZoomSpeedSpace   = 1;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ZoomSpeedSpace            = (struct tt__Space1DDescription*)soap_malloc_zero(soap_ptr, tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->__sizeZoomSpeedSpace*sizeof(struct tt__Space1DDescription));
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ZoomSpeedSpace->XRange    = (struct tt__FloatRange *)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));

            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->PanTiltSpeedSpace->URI         = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_PanTiltSpeedSpace.s_URI);
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->PanTiltSpeedSpace->XRange->Max = PTZ_NodePtr[Id].s_PanTiltSpeedSpace.s_XRangeMax;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->PanTiltSpeedSpace->XRange->Min = PTZ_NodePtr[Id].s_PanTiltSpeedSpace.s_XRangeMin;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ZoomSpeedSpace->URI            = (char_t*)soap_strdup(soap_ptr,PTZ_NodePtr[Id].s_ZoomSpeedSpace.s_URI);
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ZoomSpeedSpace->XRange->Max    = PTZ_NodePtr[Id].s_ZoomSpeedSpace.s_XRangeMax;
            tptz__GetNodesResponse->PTZNode[Id].SupportedPTZSpaces->ZoomSpeedSpace->XRange->Min    = PTZ_NodePtr[Id].s_ZoomSpeedSpace.s_XRangeMin;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        ONVIF_INFO("%s normal out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetNode(struct soap *soap_ptr, struct _tptz__GetNode *tptz__GetNode, struct _tptz__GetNodeResponse *tptz__GetNodeResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    int32_t 				   Id						   = 0;
    int32_t 				   Sp						   = 0;
    int32_t 				   PTZNum					   = 0;
    int32_t 				   SpaceNum 				   = 0;
    GMI_RESULT				   Result					   = GMI_SUCCESS;
    PTZ_Node				  *PTZ_NodePtr				   = NULL;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }
        //should get from sys control server.
        //get ptz num
        PTZNum		= DEFAULT_PTZ_NUM;
        SpaceNum	=  DEFAULT_PTZ_NUM;//DEFAULT_PTZ_SPACE_NUM;
        PTZ_NodePtr = l_PTZ_Node;

        tptz__GetNodeResponse->PTZNode 	  = (struct tt__PTZNode*)soap_malloc_zero(soap_ptr, sizeof(struct tt__PTZNode));

        for (Id = 0; Id < PTZNum; Id++)
        {
            if (0 == strcmp(tptz__GetNode->NodeToken, PTZ_NodePtr[Id].s_token))
            {
                tptz__GetNodeResponse->PTZNode->Name				   = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_Name);
                tptz__GetNodeResponse->PTZNode->token				   = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_token);
                tptz__GetNodeResponse->PTZNode->MaximumNumberOfPresets = PTZ_NodePtr[Id].s_MaximumNumberOfPresets;
                tptz__GetNodeResponse->PTZNode->HomeSupported		   = PTZ_NodePtr[Id].s_HomeSupported;

                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces =(struct tt__PTZSpaces*)soap_malloc_zero(soap_ptr, sizeof(struct tt__PTZSpaces));
                //AbsolutePanTiltPositionSpace
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeAbsolutePanTiltPositionSpace = SpaceNum;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace =(struct tt__Space2DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeAbsolutePanTiltPositionSpace*sizeof(struct tt__Space2DDescription));
                for (Sp = 0; Sp < tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeAbsolutePanTiltPositionSpace; Sp++)
                {
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_AbsolutePanTiltPositionSpace[Sp].s_URI);
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].YRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_AbsolutePanTiltPositionSpace[Sp].s_XRangeMin;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_AbsolutePanTiltPositionSpace[Sp].s_XRangeMax;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].YRange->Min = PTZ_NodePtr[Id].s_AbsolutePanTiltPositionSpace[Sp].s_YRangeMin;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsolutePanTiltPositionSpace[Sp].YRange->Max = PTZ_NodePtr[Id].s_AbsolutePanTiltPositionSpace[Sp].s_YRangeMax;
                }
                //AbsoluteZoomPositionSpace
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeAbsoluteZoomPositionSpace = SpaceNum;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace =(struct tt__Space1DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeAbsoluteZoomPositionSpace*sizeof(struct tt__Space2DDescription));
                for (Sp = 0; Sp < tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeAbsoluteZoomPositionSpace; Sp++)
                {
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_AbsoluteZoomPositionSpace[Sp].s_URI);
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_AbsoluteZoomPositionSpace[Sp].s_XRangeMin;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->AbsoluteZoomPositionSpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_AbsoluteZoomPositionSpace[Sp].s_XRangeMax;
                }

                //RelativePanTiltTranslationSpace
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeRelativePanTiltTranslationSpace = SpaceNum;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace =(struct tt__Space2DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeRelativePanTiltTranslationSpace*sizeof(struct tt__Space2DDescription));
                for (Sp = 0; Sp < tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeRelativePanTiltTranslationSpace; Sp++)
                {
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_URI);
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].YRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_XRangeMin;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_XRangeMax;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].YRange->Min = PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_YRangeMin;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativePanTiltTranslationSpace[Sp].YRange->Max = PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_YRangeMax;
                }
                //RelativeZoomTranslationSpace
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeRelativeZoomTranslationSpace = SpaceNum;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace =(struct tt__Space1DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeRelativeZoomTranslationSpace*sizeof(struct tt__Space2DDescription));
                for (Sp = 0; Sp < tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeRelativeZoomTranslationSpace; Sp++)
                {
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_RelativePanTiltTranslationSpace[Sp].s_URI);
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_RelativeZoomTranslationSpace[Sp].s_XRangeMin;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->RelativeZoomTranslationSpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_RelativeZoomTranslationSpace[Sp].s_XRangeMax;
                }

                //ContinuousPanTiltVelocitySpace
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace = SpaceNum;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace =(struct tt__Space2DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace*sizeof(struct tt__Space2DDescription));
                for (Sp = 0; Sp < tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace; Sp++)
                {
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_ContinuousPanTiltVelocitySpace[Sp].s_URI);
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].YRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_ContinuousPanTiltVelocitySpace[Sp].s_XRangeMin;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_ContinuousPanTiltVelocitySpace[Sp].s_XRangeMax;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].YRange->Min = PTZ_NodePtr[Id].s_ContinuousPanTiltVelocitySpace[Sp].s_YRangeMin;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousPanTiltVelocitySpace[Sp].YRange->Max = PTZ_NodePtr[Id].s_ContinuousPanTiltVelocitySpace[Sp].s_YRangeMax;
                }

                //ContinuousZoomVelocitySpace
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeContinuousZoomVelocitySpace = SpaceNum;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace =(struct tt__Space1DDescription *)soap_malloc_zero(soap_ptr, tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeContinuousPanTiltVelocitySpace*sizeof(struct tt__Space2DDescription));
                for (Sp = 0; Sp < tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeContinuousZoomVelocitySpace; Sp++)
                {
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace[Sp].URI = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_ContinuousZoomVelocitySpace[Sp].s_URI);
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace[Sp].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace[Sp].XRange->Min = PTZ_NodePtr[Id].s_ContinuousZoomVelocitySpace[Sp].s_XRangeMin;
                    tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ContinuousZoomVelocitySpace[Sp].XRange->Max = PTZ_NodePtr[Id].s_ContinuousZoomVelocitySpace[Sp].s_XRangeMax;
                }

                //speed space
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizePanTiltSpeedSpace   = 1;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->PanTiltSpeedSpace		  = (struct tt__Space1DDescription* )soap_malloc_zero(soap_ptr, tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizePanTiltSpeedSpace*sizeof(struct tt__Space1DDescription));
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->PanTiltSpeedSpace->XRange = (struct tt__FloatRange *)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeZoomSpeedSpace   = 1;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ZoomSpeedSpace			  = (struct tt__Space1DDescription*)soap_malloc_zero(soap_ptr, tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->__sizeZoomSpeedSpace*sizeof(struct tt__Space1DDescription));
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ZoomSpeedSpace->XRange	  = (struct tt__FloatRange *)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));

                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->PanTiltSpeedSpace->URI		   = (char_t*)soap_strdup(soap_ptr, PTZ_NodePtr[Id].s_PanTiltSpeedSpace.s_URI);
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->PanTiltSpeedSpace->XRange->Max = PTZ_NodePtr[Id].s_PanTiltSpeedSpace.s_XRangeMax;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->PanTiltSpeedSpace->XRange->Min = PTZ_NodePtr[Id].s_PanTiltSpeedSpace.s_XRangeMin;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ZoomSpeedSpace->URI 		   = (char_t*)soap_strdup(soap_ptr,PTZ_NodePtr[Id].s_ZoomSpeedSpace.s_URI);
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ZoomSpeedSpace->XRange->Max    = PTZ_NodePtr[Id].s_ZoomSpeedSpace.s_XRangeMax;
                tptz__GetNodeResponse->PTZNode->SupportedPTZSpaces->ZoomSpeedSpace->XRange->Min    = PTZ_NodePtr[Id].s_ZoomSpeedSpace.s_XRangeMin;
                break;
            }
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetConfiguration(struct soap *soap_ptr, struct _tptz__SetConfiguration *tptz__SetConfiguration, struct _tptz__SetConfigurationResponse *tptz__SetConfigurationResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetConfigurationOptions(struct soap *soap_ptr, struct _tptz__GetConfigurationOptions *tptz__GetConfigurationOptions, struct _tptz__GetConfigurationOptionsResponse *tptz__GetConfigurationOptionsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint8_t                    Id                          = 0;
    uint8_t                    PTZ_Num                     = 0;
    int32_t                    SpaceNum                    = 0;
    boolean_t                  SupportDigitalPTZ           = false;
    PTZ_ConfigurationOptions  *PTZ_ConfigurationOptionsPtr = NULL;
    GMI_RESULT                 Result                      = GMI_SUCCESS;

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        /*get device capabilitis from sys_control_server, acquire
              1. PTZ Num;
              2. support digitial PTZ or not;
              3. absolute PanTilt X range, absolute  PanTilt Y range,
                  absolute Zoom X range;
              4. relative PanTilt X range, relative PanTilt Y range,
                  relative Zoom X range;
              5. continous PanTilt X range, continous PanTilt Y range,
                  continous Zoom X range;
              6. ptz timeout range.
           */
        PTZ_Num           = DEFAULT_PTZ_NUM;
        SpaceNum          = DEFAULT_PTZ_NUM;//DEFAULT_PTZ_SPACE_NUM;
        SupportDigitalPTZ = true;

        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions         = (struct tt__PTZConfigurationOptions *)soap_malloc_zero( soap_ptr,sizeof(struct tt__PTZConfigurationOptions) );
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->__size = PTZ_Num;

        //soap_malloc
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces = (struct tt__PTZSpaces *)soap_malloc_zero( soap_ptr, sizeof(struct tt__PTZSpaces) );
        if (NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces)
        {
            ONVIF_ERROR("PTZConfigurationOptions->Spaces soap_malloc_zero failed\n");
            break;
        }

        //digital ptz support or not
        if (SupportDigitalPTZ)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsolutePanTiltPositionSpace    = SpaceNum;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsoluteZoomPositionSpace       = SpaceNum;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativePanTiltTranslationSpace = SpaceNum;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativeZoomTranslationSpace    = SpaceNum;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousPanTiltVelocitySpace  = SpaceNum;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousZoomVelocitySpace     = SpaceNum;
        }
        else
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsolutePanTiltPositionSpace    = DEFAULT_PTZ_NUM;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsoluteZoomPositionSpace       = DEFAULT_PTZ_NUM;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativePanTiltTranslationSpace = DEFAULT_PTZ_NUM;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativeZoomTranslationSpace    = DEFAULT_PTZ_NUM;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousPanTiltVelocitySpace  = DEFAULT_PTZ_NUM;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousZoomVelocitySpace     = DEFAULT_PTZ_NUM;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizePanTiltSpeedSpace               = DEFAULT_PTZ_NUM;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeZoomSpeedSpace                  = DEFAULT_PTZ_NUM;
        }
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizePanTiltSpeedSpace                   = 1;
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeZoomSpeedSpace                      = 1;

        //soap malloc space
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace   = (struct tt__Space2DDescription *)soap_malloc_zero( soap_ptr, tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsolutePanTiltPositionSpace*sizeof(struct tt__Space2DDescription));
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace      = (struct tt__Space1DDescription *)soap_malloc_zero( soap_ptr, tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsoluteZoomPositionSpace*sizeof(struct tt__Space1DDescription));
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace= (struct tt__Space2DDescription *)soap_malloc_zero( soap_ptr, tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativePanTiltTranslationSpace*sizeof(struct tt__Space2DDescription));
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace   = (struct tt__Space1DDescription *)soap_malloc_zero( soap_ptr, tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativeZoomTranslationSpace*sizeof(struct tt__Space1DDescription));
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace = (struct tt__Space2DDescription *)soap_malloc_zero( soap_ptr, tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousPanTiltVelocitySpace*sizeof(struct tt__Space2DDescription) );
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace    = (struct tt__Space1DDescription *)soap_malloc_zero( soap_ptr, tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousZoomVelocitySpace*sizeof(struct tt__Space1DDescription) );
        if (  NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace
                || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace
                || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace
                || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace
                || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace
                || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace )
        {
            ONVIF_ERROR("spaces soap_malloc_zero failed\n");
            break;
        }

        //soap malloc absolute pantilt xrange yrange
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsolutePanTiltPositionSpace; Id++ )
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[Id].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[Id].YRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            if (NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[Id].XRange
                    || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[Id].YRange)
            {
                ONVIF_ERROR("XRange YRange soap_malloc_zero failed\n");
                break;
            }
        }
        //soap malloc absolute zoom xrange
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsoluteZoomPositionSpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace[Id].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            if (NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace[Id].XRange)
            {
                ONVIF_ERROR("XRange YRange soap_malloc_zero failed\n");
                break;
            }
        }

        //soap malloc relative pantilt xrange yrange
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativePanTiltTranslationSpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace[Id].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace[Id].YRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            if (NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace[Id].XRange
                    || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace[Id].YRange)
            {
                ONVIF_ERROR("XRange YRange soap_malloc_zero failed\n");
                break;
            }
        }
        //soap malloc relative zoom xrange
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativeZoomTranslationSpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace[Id].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            if (NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace[Id].XRange)
            {
                ONVIF_ERROR("XRange YRange soap_malloc_zero failed\n");
                break;
            }
        }

        //soap malloc continous pantilt xrange yrange
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousPanTiltVelocitySpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace[Id].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace[Id].YRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            if (NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace[Id].XRange
                    || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace[Id].YRange)
            {
                ONVIF_ERROR("XRange YRange soap_malloc_zero failed\n");
                break;
            }
        }
        //soap malloc continous zoom xrange
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousZoomVelocitySpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace[Id].XRange = (struct tt__FloatRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
            if (NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace[Id].XRange)
            {
                ONVIF_ERROR("XRange YRange soap_malloc_zero failed\n");
                break;
            }
        }

        //soap malloc PTZ timeout
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->PTZTimeout = (struct tt__DurationRange*)soap_malloc_zero(soap_ptr, sizeof(struct tt__DurationRange));
        if (NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->PTZTimeout)
        {
            ONVIF_ERROR("PTZTimeout soap_malloc_zero failed\n");
            break;
        }

        //soap malloc PanTiltSpeedSpace
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->PanTiltSpeedSpace         = (struct tt__Space1DDescription* )soap_malloc_zero(soap_ptr, sizeof(struct tt__Space1DDescription));
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->PanTiltSpeedSpace->XRange = (struct tt__FloatRange *)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
        //soap_malloc Zoom
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ZoomSpeedSpace            = (struct tt__Space1DDescription*)soap_malloc_zero(soap_ptr, sizeof(struct tt__Space1DDescription));
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ZoomSpeedSpace->XRange    = (struct tt__FloatRange *)soap_malloc_zero(soap_ptr, sizeof(struct tt__FloatRange));
        if ( NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->PanTiltSpeedSpace
                || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->PanTiltSpeedSpace->XRange
                || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ZoomSpeedSpace
                || NULL == tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ZoomSpeedSpace->XRange)
        {
            ONVIF_ERROR("SpeedSpace soap_malloc_zero failed\n");
            break;
        }


        PTZ_ConfigurationOptionsPtr = l_PTZ_ConfigurationOptions;
        //fillout the struct of tptz__GetConfigurationOptionsResponse
        //AbsolutePanTiltPositionSpace
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsolutePanTiltPositionSpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[Id].URI         = (char_t*)soap_strdup(soap_ptr, PTZ_ConfigurationOptionsPtr->s_AbsolutePanTiltPositionSpace[Id].s_URI);
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[Id].XRange->Max = PTZ_ConfigurationOptionsPtr->s_AbsolutePanTiltPositionSpace[Id].s_XRangeMax;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[Id].XRange->Min = PTZ_ConfigurationOptionsPtr->s_AbsolutePanTiltPositionSpace[Id].s_XRangeMin;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[Id].YRange->Max = PTZ_ConfigurationOptionsPtr->s_AbsolutePanTiltPositionSpace[Id].s_YRangeMax;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsolutePanTiltPositionSpace[Id].YRange->Min = PTZ_ConfigurationOptionsPtr->s_AbsolutePanTiltPositionSpace[Id].s_YRangeMin;
        }
        //AbsoluteZoomPositionSpace
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeAbsoluteZoomPositionSpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace[Id].URI             = (char_t*)soap_strdup(soap_ptr, PTZ_ConfigurationOptionsPtr->s_AbsoluteZoomPositionSpace[Id].s_URI);
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace[Id].XRange->Max     = PTZ_ConfigurationOptionsPtr->s_AbsoluteZoomPositionSpace[Id].s_XRangeMax;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->AbsoluteZoomPositionSpace[Id].XRange->Min     = PTZ_ConfigurationOptionsPtr->s_AbsoluteZoomPositionSpace[Id].s_XRangeMin;
        }

        //RelativePanTiltTranslationSpace
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativePanTiltTranslationSpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace[Id].URI         = (char*)soap_strdup(soap_ptr, PTZ_ConfigurationOptionsPtr->s_RelativePanTiltTranslationSpace[Id].s_URI);
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace[Id].XRange->Max = PTZ_ConfigurationOptionsPtr->s_RelativePanTiltTranslationSpace[Id].s_XRangeMax;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace[Id].XRange->Min = PTZ_ConfigurationOptionsPtr->s_RelativePanTiltTranslationSpace[Id].s_XRangeMin;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace[Id].YRange->Max = PTZ_ConfigurationOptionsPtr->s_RelativePanTiltTranslationSpace[Id].s_YRangeMax;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativePanTiltTranslationSpace[Id].YRange->Min = PTZ_ConfigurationOptionsPtr->s_RelativePanTiltTranslationSpace[Id].s_YRangeMin;
        }
        //RelativeZoomTranslationSpace
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeRelativeZoomTranslationSpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace[Id].URI            = (char_t*)soap_strdup(soap_ptr, PTZ_ConfigurationOptionsPtr->s_RelativeZoomTranslationSpace[Id].s_URI);
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace[Id].XRange->Max    = PTZ_ConfigurationOptionsPtr->s_RelativeZoomTranslationSpace[Id].s_XRangeMax;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->RelativeZoomTranslationSpace[Id].XRange->Min    = PTZ_ConfigurationOptionsPtr->s_RelativeZoomTranslationSpace[Id].s_XRangeMin;
        }

        //ContinuousPanTiltVelocitySpace
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousPanTiltVelocitySpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace[Id].URI         = (char *)soap_strdup(soap_ptr, PTZ_ConfigurationOptionsPtr->s_ContinuousPanTiltVelocitySpace[Id].s_URI);
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace[Id].XRange->Max = PTZ_ConfigurationOptionsPtr->s_ContinuousPanTiltVelocitySpace[Id].s_XRangeMax;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace[Id].XRange->Min = PTZ_ConfigurationOptionsPtr->s_ContinuousPanTiltVelocitySpace[Id].s_XRangeMin;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace[Id].YRange->Max = PTZ_ConfigurationOptionsPtr->s_ContinuousPanTiltVelocitySpace[Id].s_YRangeMax;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousPanTiltVelocitySpace[Id].YRange->Min = PTZ_ConfigurationOptionsPtr->s_ContinuousPanTiltVelocitySpace[Id].s_YRangeMin;
        }
        //ContinuousZoomVelocitySpace
        for (Id = 0; Id < tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->__sizeContinuousZoomVelocitySpace; Id++)
        {
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace[Id].URI            = (char_t*)soap_strdup(soap_ptr, PTZ_ConfigurationOptionsPtr->s_ContinuousZoomVelocitySpace[Id].s_URI);
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace[Id].XRange->Max    = PTZ_ConfigurationOptionsPtr->s_ContinuousZoomVelocitySpace[Id].s_XRangeMax;
            tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ContinuousZoomVelocitySpace[Id].XRange->Min    = PTZ_ConfigurationOptionsPtr->s_ContinuousZoomVelocitySpace[Id].s_XRangeMin;
        }

        //PanTiltSpeedSpace
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->PanTiltSpeedSpace->URI         = (char_t*)soap_strdup(soap_ptr, PTZ_ConfigurationOptionsPtr->s_PanTiltSpeedSpace.s_URI);
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->PanTiltSpeedSpace->XRange->Max = PTZ_ConfigurationOptionsPtr->s_PanTiltSpeedSpace.s_XRangeMax;
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->PanTiltSpeedSpace->XRange->Min = PTZ_ConfigurationOptionsPtr->s_PanTiltSpeedSpace.s_XRangeMin;

        //ZoomSpeedSpace
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ZoomSpeedSpace->URI         = (char_t*)soap_strdup(soap_ptr, PTZ_ConfigurationOptionsPtr->s_ZoomSpeedSpace.s_URI);
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ZoomSpeedSpace->XRange->Max = PTZ_ConfigurationOptionsPtr->s_ZoomSpeedSpace.s_XRangeMax;
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->Spaces->ZoomSpeedSpace->XRange->Min = PTZ_ConfigurationOptionsPtr->s_ZoomSpeedSpace.s_XRangeMin;

        //ptz timeout
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->PTZTimeout->Max = PTZ_ConfigurationOptionsPtr->s_PTZTimeoutRange.Max;
        tptz__GetConfigurationOptionsResponse->PTZConfigurationOptions->PTZTimeout->Min = PTZ_ConfigurationOptionsPtr->s_PTZTimeoutRange.Min;

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GotoHomePosition(struct soap *soap_ptr, struct _tptz__GotoHomePosition *tptz__GotoHomePosition, struct _tptz__GotoHomePositionResponse *tptz__GotoHomePositionResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t   SessionId = 0;
    uint32_t   AuthValue = 0;

    do
    {
        //user auth
        GMI_RESULT Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        SysPkgPtzCtrl PtzCtrlCmd;
        memset(&PtzCtrlCmd, 0, sizeof(SysPkgPtzCtrl));
        PtzCtrlCmd.s_PtzCmd   = 1;
        PtzCtrlCmd.s_PtzCmd   = SYS_PTZCMD_GOTOPRESET;
        PtzCtrlCmd.s_Param[0] = 1;
        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SYS_PTZCMD_GOTOPRESET failed\n");
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__SetHomePosition(struct soap *soap_ptr, struct _tptz__SetHomePosition *tptz__SetHomePosition, struct _tptz__SetHomePositionResponse *tptz__SetHomePositionResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    uint16_t   SessionId = 0;
    uint32_t   AuthValue = 0;

    do
    {
        //user auth
        GMI_RESULT Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "tptz__SetHomePosition->ProfileToken %s\n", tptz__SetHomePosition->ProfileToken);
        ONVIF_INFO("tptz__SetHomePosition->ProfileToken %s\n", tptz__SetHomePosition->ProfileToken);

        SysPkgPtzCtrl PtzCtrlCmd;
        memset(&PtzCtrlCmd, 0, sizeof(SysPkgPtzCtrl));
        PtzCtrlCmd.s_PtzCmd   = 1;
        PtzCtrlCmd.s_PtzCmd   = SYS_PTZCMD_SETPRESET;
        PtzCtrlCmd.s_Param[0] = 1;
        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd);
        if (FAILED(Result))
        {
            ONVIF_ERROR("SYS_PTZCMD_SETPRESET failed\n");
            break;
        }

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__ContinuousMove(struct soap *soap_ptr, struct _tptz__ContinuousMove *tptz__ContinuousMove, struct _tptz__ContinuousMoveResponse *tptz__ContinuousMoveResponse)
{
    uint16_t      SessionId  = 0;
    uint32_t      AuthValue  = 0;
    uint8_t       Id         = 0;
    uint8_t       PTZ_Num    = DEFAULT_PTZ_NUM;
    GMI_RESULT    Result     = GMI_SUCCESS;
    uint32_t      LocalXRange= 255;
    uint32_t      LocalYRange= 255;
    uint32_t      ZoomXRange = 255;
    SysPkgPtzCtrl PtzCtrlCmd = {0};

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        if (tptz__ContinuousMove->Velocity == NULL)
        {
            break;
        }

        for (Id = 0; Id < PTZ_Num; Id++)
        {
            //if (memcmp(tptz__ContinuousMove->ProfileToken, ))
            {
                PtzCtrlCmd.s_PtzId = Id + 1;

                if (tptz__ContinuousMove->Velocity->PanTilt != NULL)
                {
                    if (tptz__ContinuousMove->Velocity->PanTilt->x > 0
                            && tptz__ContinuousMove->Velocity->PanTilt->y > 0)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_RIGHTUP;
                        PtzCtrlCmd.s_Param[0] =  abs((int32_t)(LocalXRange*(tptz__ContinuousMove->Velocity->PanTilt->x))); /*rightup*/
                        PtzCtrlCmd.s_Param[1] =  abs((int32_t)(LocalYRange*(tptz__ContinuousMove->Velocity->PanTilt->y)));
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            ONVIF_ERROR("SYS_PTZCMD_RIGHTUP fail, Result = 0x%lx\n", Result);
                            break;
                        }
                    }
                    else if (tptz__ContinuousMove->Velocity->PanTilt->x > 0
                             && tptz__ContinuousMove->Velocity->PanTilt->y < 0)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_RIGHTDOWN;
                        PtzCtrlCmd.s_Param[0] =  abs((int32_t)(LocalXRange*(tptz__ContinuousMove->Velocity->PanTilt->x))); /*rightdown*/
                        PtzCtrlCmd.s_Param[1] =  abs((int32_t)(LocalYRange*(tptz__ContinuousMove->Velocity->PanTilt->y)));
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            ONVIF_ERROR("SYS_PTZCMD_RIGHTDOWN fail, Result = 0x%lx\n", Result);
                            break;
                        }
                    }
                    else if (tptz__ContinuousMove->Velocity->PanTilt->x < 0
                             && tptz__ContinuousMove->Velocity->PanTilt->y > 0)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_LEFTUP;
                        PtzCtrlCmd.s_Param[0] =  abs((int32_t)(LocalXRange*(tptz__ContinuousMove->Velocity->PanTilt->x))); /*leftup*/
                        PtzCtrlCmd.s_Param[1] =  abs((int32_t)(LocalYRange*(tptz__ContinuousMove->Velocity->PanTilt->y)));
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            ONVIF_ERROR("SYS_PTZCMD_LEFTUP fail, Result = 0x%lx\n", Result);
                            break;
                        }
                    }
                    else if (tptz__ContinuousMove->Velocity->PanTilt->x < 0
                             && tptz__ContinuousMove->Velocity->PanTilt->y < 0)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_LEFTDOWN;
                        PtzCtrlCmd.s_Param[0] =  abs((int32_t)(LocalXRange*(tptz__ContinuousMove->Velocity->PanTilt->x))); /*leftdown*/
                        PtzCtrlCmd.s_Param[1] =  abs((int32_t)(LocalYRange*(tptz__ContinuousMove->Velocity->PanTilt->y)));
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            ONVIF_ERROR("SYS_PTZCMD_LEFTDOWN fail, Result = 0x%lx\n", Result);
                            break;
                        }
                    }
                    else if (tptz__ContinuousMove->Velocity->PanTilt->x > 0)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_RIGHT;
                        PtzCtrlCmd.s_Param[0] =  abs((int32_t)(LocalXRange*(tptz__ContinuousMove->Velocity->PanTilt->x))); /*right*/
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            ONVIF_ERROR("SYS_PTZCMD_RIGHT fail, Result = 0x%lx\n", Result);
                            break;
                        }
                    }
                    else if (tptz__ContinuousMove->Velocity->PanTilt->x < 0)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_LEFT;
                        PtzCtrlCmd.s_Param[0] = abs((int32_t)(LocalXRange*(tptz__ContinuousMove->Velocity->PanTilt->x))); /*left*/
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            ONVIF_ERROR("SYS_PTZCMD_LEFT fail, Result = 0x%lx\n", Result);
                            break;
                        }
                    }
                    else if (tptz__ContinuousMove->Velocity->PanTilt->y > 0)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_UP;
                        PtzCtrlCmd.s_Param[0] =  abs((int32_t)(LocalYRange*(tptz__ContinuousMove->Velocity->PanTilt->y))); /*up*/
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            ONVIF_ERROR("SYS_PTZCMD_UP fail, Result = 0x%lx\n", Result);
                            break;
                        }
                    }
                    else if (tptz__ContinuousMove->Velocity->PanTilt->y < 0)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_DOWN;
                        PtzCtrlCmd.s_Param[0] = abs((int32_t)(LocalYRange*(tptz__ContinuousMove->Velocity->PanTilt->y))); /*down*/
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            ONVIF_ERROR("SYS_PTZCMD_DOWN fail, Result = 0x%lx\n", Result);
                            break;
                        }
                    }
                }

                if (tptz__ContinuousMove->Velocity->Zoom != NULL)
                {
                    if (tptz__ContinuousMove->Velocity->Zoom->x > 0)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_ZOOM_TELE;
                        PtzCtrlCmd.s_Param[0] =  abs((int32_t)(ZoomXRange*(tptz__ContinuousMove->Velocity->Zoom->x))); /*zoom wide*/
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            ONVIF_ERROR("SYS_PTZCMD_ZOOM_TELE fail, Result = 0x%lx\n", Result);
                            break;
                        }
                    }
                    else  if (tptz__ContinuousMove->Velocity->Zoom->x < 0)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_ZOOM_WIDE;
                        PtzCtrlCmd.s_Param[0] = abs((int32_t)(ZoomXRange*(tptz__ContinuousMove->Velocity->Zoom->x))); /*zoom tele*/
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            ONVIF_ERROR("SYS_PTZCMD_ZOOM_WIDE fail, Result = 0x%lx\n", Result);
                            break;
                        }
                    }
                }

                if (tptz__ContinuousMove->Timeout != NULL)
                {
                    l_Ptz0ContinousMoveStop = true;
                    l_Ptz0ContinousMoveTimeout = *tptz__ContinuousMove->Timeout / 1000;
                }
            }
        }
        return SOAP_OK;
    }
    while (0);
    ONVIF_INFO("%s Abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}



void *Ptz_ContinousMoveStopThread(void)
{
    uint16_t   SessionId = 0;
    uint32_t   AuthValue = 0;
    GMI_RESULT     Result = GMI_SUCCESS;    
    SysPkgPtzCtrl  PtzCtrlCmd = {0};

    pthread_detach(pthread_self());

    while (l_PtzContinousMoveThreadExit != true)
    {
        if (l_Ptz0ContinousMoveStop)
        {
        	if (l_Ptz0ContinousMoveTimeout <= 0)
        	{
        		PtzCtrlCmd.s_PtzId = PTZ_0 + 1;
	            PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_STOP;
	            Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
	            if ( FAILED(Result) )
	            {
	                ONVIF_ERROR("SysPtzControl fail, Result = 0x%lx\n", Result);
	            }

	            l_Ptz0ContinousMoveStop = false;
	            l_Ptz0ContinousMoveTimeout = 0;
        	}
        	else
        	{
        		l_Ptz0ContinousMoveTimeout--;
        	}                       
        }
        
        sleep(1);
    }

    l_PtzContinousMoveThreadExit = false;

    return (void*)NULL;
}



SOAP_FMAC5 int SOAP_FMAC6 __tptz__RelativeMove(struct soap *soap_ptr, struct _tptz__RelativeMove *tptz__RelativeMove, struct _tptz__RelativeMoveResponse *tptz__RelativeMoveResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__SendAuxiliaryCommand(struct soap *soap_ptr, struct _tptz__SendAuxiliaryCommand *tptz__SendAuxiliaryCommand, struct _tptz__SendAuxiliaryCommandResponse *tptz__SendAuxiliaryCommandResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__AbsoluteMove(struct soap *soap_ptr, struct _tptz__AbsoluteMove *tptz__AbsoluteMove, struct _tptz__AbsoluteMoveResponse *tptz__AbsoluteMoveResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__Stop(struct soap *soap_ptr, struct _tptz__Stop *tptz__Stop, struct _tptz__StopResponse *tptz__StopResponse)
{
    uint16_t   SessionId = 0;
    uint32_t   AuthValue = 0;
    uint8_t       Id         = PTZ_0;
    uint8_t       PTZ_Num    = DEFAULT_PTZ_NUM;
    GMI_RESULT    Result     = GMI_SUCCESS;
    SysPkgPtzCtrl PtzCtrlCmd = {0};

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            ONVIF_Fault(soap_ptr, "ter:NotAuthorized", NULL, "The action requested requires authorization and the sender is not authorized");
            break;
        }

        for (Id = 0; Id < PTZ_Num; Id++)
        {
            //if (0 == strcmp(tptz__Stop->ProfileToken, ))
            {
                PtzCtrlCmd.s_PtzId = Id + 1;

                if (tptz__Stop->PanTilt != NULL)
                {
                    if (*(tptz__Stop->PanTilt) == xsd__boolean__true_)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_STOP;
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            break;
                        }
                        l_Ptz0ContinousMoveStop = false;
                    }
                }

                if (tptz__Stop->Zoom != NULL)
                {
                    if (*(tptz__Stop->Zoom) == xsd__boolean__true_)
                    {
                        PtzCtrlCmd.s_PtzCmd = SYS_PTZCMD_STOP;
                        Result = SysPtzControl(SessionId, AuthValue, &PtzCtrlCmd );
                        if (FAILED(Result))
                        {
                            break;
                        }
                        l_Ptz0ContinousMoveStop = false;
                    }
                }
            }
        }

        return SOAP_OK;
    }
    while (0);

    ONVIF_INFO("%s abnormal out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTours(struct soap *soap_ptr, struct _tptz__GetPresetTours *tptz__GetPresetTours, struct _tptz__GetPresetToursResponse *tptz__GetPresetToursResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTour(struct soap *soap_ptr, struct _tptz__GetPresetTour *tptz__GetPresetTour, struct _tptz__GetPresetTourResponse *tptz__GetPresetTourResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__GetPresetTourOptions(struct soap *soap_ptr, struct _tptz__GetPresetTourOptions *tptz__GetPresetTourOptions, struct _tptz__GetPresetTourOptionsResponse *tptz__GetPresetTourOptionsResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__CreatePresetTour(struct soap *soap_ptr, struct _tptz__CreatePresetTour *tptz__CreatePresetTour, struct _tptz__CreatePresetTourResponse *tptz__CreatePresetTourResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__ModifyPresetTour(struct soap *soap_ptr, struct _tptz__ModifyPresetTour *tptz__ModifyPresetTour, struct _tptz__ModifyPresetTourResponse *tptz__ModifyPresetTourResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__OperatePresetTour(struct soap *soap_ptr, struct _tptz__OperatePresetTour *tptz__OperatePresetTour, struct _tptz__OperatePresetTourResponse *tptz__OperatePresetTourResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __tptz__RemovePresetTour(struct soap *soap_ptr, struct _tptz__RemovePresetTour *tptz__RemovePresetTour, struct _tptz__RemovePresetTourResponse *tptz__RemovePresetTourResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    ONVIF_INFO("%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}

