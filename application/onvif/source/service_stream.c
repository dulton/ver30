#include "log.h"
#include "service_utilitly.h"
#include "service_stream.h"
#include "service_device_management.h"
#include "service_media_configuration.h"
#include "service_utilitly.h"
#include "soapH.h"
#include "sys_client.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"

extern uint16_t g_ONVIF_Port;
extern uint16_t g_RTSP_Port;

SOAP_FMAC5 int SOAP_FMAC6 __trt__GetStreamUri(struct soap *soap_ptr, struct _trt__GetStreamUri *trt__GetStreamUri, struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    uint16_t   SessionId = 0;
    uint32_t   AuthValue = 0;
    int32_t    RtspPort = g_RTSP_Port;
    GMI_RESULT Result = GMI_SUCCESS;
    char_t     Url[100] = {0};
    char_t     IP[NETWORK_INFO_LENGTH] = {0};
    char_t     InterfaceName[NETWORK_INFO_LENGTH] = {0};

    do
    {
        //user auth
        Result = Soap_WSSE_Authentication(soap_ptr);
        if (FAILED(Result))
        {
            ONVIF_ERROR("user auth failed\n");
            break;
        }

        //
        strcpy(InterfaceName, DEFAULT_NETWORK_NAME);
        Result = NET_GetIpInfo(InterfaceName, IP);
        if (FAILED(Result))
        {
            break;
        }      
        
        if (0 == strcmp(trt__GetStreamUri->ProfileToken, PROFILE_TOKEN_1))
        {
            Result = SysForceIdr(SessionId, AuthValue, 0);
            if (FAILED(Result))
            {
                ONVIF_ERROR("\n");
            }
            sprintf( Url, "rtsp://%s:%d/stream1", IP, RtspPort);
        }
        else if (0 == strcmp(trt__GetStreamUri->ProfileToken, PROFILE_TOKEN_2))
        {
            Result = SysForceIdr(SessionId, AuthValue, 1);
            if (FAILED(Result))
            {
                ONVIF_ERROR("\n");
            }
            sprintf( Url, "rtsp://%s:%d/stream2", IP, RtspPort);
        }
        else if (0 == strcmp(trt__GetStreamUri->ProfileToken, PROFILE_TOKEN_3))
        {
            Result = SysForceIdr(SessionId, AuthValue, 2);
            if (FAILED(Result))
            {
                ONVIF_ERROR("\n");
            }
            sprintf( Url, "rtsp://%s:%d/stream3", IP, RtspPort);
        }
        else if (0 == strcmp(trt__GetStreamUri->ProfileToken, PROFILE_TOKEN_4))
        {
            Result = SysForceIdr(SessionId, AuthValue, 3);
            if (FAILED(Result))
            {
                ONVIF_ERROR("\n");
            }
            sprintf( Url, "rtsp://%s:%d/stream4", IP, RtspPort);
        }
        else
        {
            ONVIF_ERROR("not support profiletoken %s\n", trt__GetStreamUri->ProfileToken);
            break;
        }

        ONVIF_INFO("%s\n", Url);

        trt__GetStreamUriResponse->MediaUri = (struct tt__MediaUri*)soap_malloc_zero(soap_ptr, sizeof(struct tt__MediaUri));
        trt__GetStreamUriResponse->MediaUri->Uri =  soap_strdup(soap_ptr, Url);
        trt__GetStreamUriResponse->MediaUri->InvalidAfterConnect = xsd__boolean__false_;
        trt__GetStreamUriResponse->MediaUri->InvalidAfterReboot = xsd__boolean__false_;
        trt__GetStreamUriResponse->MediaUri->Timeout = 0;

        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
        return SOAP_OK;
    }
    while (0);

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Abnormal Out.........\n", __func__);
    return SOAP_SVR_FAULT;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__StartMulticastStreaming(struct soap *soap_ptr, struct _trt__StartMulticastStreaming *trt__StartMulticastStreaming, struct _trt__StartMulticastStreamingResponse *trt__StartMulticastStreamingResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__StopMulticastStreaming(struct soap *soap_ptr, struct _trt__StopMulticastStreaming *trt__StopMulticastStreaming, struct _trt__StopMulticastStreamingResponse *trt__StopMulticastStreamingResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__SetSynchronizationPoint(struct soap *soap_ptr, struct _trt__SetSynchronizationPoint *trt__SetSynchronizationPoint, struct _trt__SetSynchronizationPointResponse *trt__SetSynchronizationPointResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out, Not Implement.........\n", __func__);
    return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 __trt__GetSnapshotUri(struct soap *soap_ptr, struct _trt__GetSnapshotUri *trt__GetSnapshotUri, struct _trt__GetSnapshotUriResponse *trt__GetSnapshotUriResponse)
{
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);
    trt__GetSnapshotUriResponse->MediaUri = (struct tt__MediaUri*)soap_malloc_zero(soap_ptr, sizeof(struct tt__MediaUri));

    trt__GetSnapshotUriResponse->MediaUri->Uri = NULL;
    trt__GetSnapshotUriResponse->MediaUri->InvalidAfterConnect = xsd__boolean__false_;
    trt__GetSnapshotUriResponse->MediaUri->InvalidAfterReboot = xsd__boolean__false_;
    trt__GetSnapshotUriResponse->MediaUri->Timeout = 0;

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s Out.........\n", __func__);
    return SOAP_OK;
}






