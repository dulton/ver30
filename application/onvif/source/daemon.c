#include "threads.h"
#include "gmi_system_headers.h"
#include "ipc_fw_v3.x_resource.h"
#include "gmi_daemon_heartbeat_api.h"
#include "daemon.h"
#include "log.h"

static DAEMON_DATA_CFG l_DaemonHandler;
static boolean_t l_DaemonKeepAliveStop = false;


GMI_RESULT DaemonRegister(void)
{
    ulong_t RegisterNum = 20;
    GMI_RESULT Result = GMI_SUCCESS; 
    
	ONVIF_INFO("%s In.........\n", __func__);  
    Result = GMI_DaemonInit(&l_DaemonHandler, ONVIF_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARDBEAT_ONVIF);
    if (FAILED(Result))
    {        
        ONVIF_ERROR("GMI_DaemonInit fail, Result = 0x%lx\n", Result);
        Result = GMI_DaemonInit(&l_DaemonHandler, ONVIF_SERVER_ID, GMI_DAEMON_HEARDBEAT_SERVER, GMI_DAEMON_HEARDBEAT_ONVIF);
        if (FAILED(Result))
        {
        	ONVIF_ERROR("GMI_DaemonInit again fail, Result = 0x%lx\n", Result);
        	return Result;
        }
    }

    do
    {
        Result = GMI_DaemonRegister(&l_DaemonHandler );
        if (SUCCEEDED(Result))
        {
            ONVIF_INFO("GMI_DaemonRegister Success\n");
            break;
        }
        sleep(1);
    }
    while (RegisterNum--);   
	ONVIF_INFO("%s Out.........\n", __func__);
    return Result;
}


static void* DaemonKeepAlive(void)
{
    GMI_RESULT Result;
    uint32_t   BootFlag = APPLICATION_RUNNING;

    pthread_detach(pthread_self());

    while (l_DaemonKeepAliveStop != true)
    {
        Result = GMI_DaemonReport(&l_DaemonHandler, &BootFlag);
        if (SUCCEEDED(Result))
        {
            if (APPLICATION_QUIT == BootFlag)
            {
                ONVIF_INFO("daemon inform system server normal exit!!!\n");
                break;
            }
        }

        sleep(1);
    }

    pthread_exit(NULL);
}


GMI_RESULT DaemonStart()
{
    THREAD_TYPE TidDaemon;

    l_DaemonKeepAliveStop = false;
    THREAD_CREATE(&TidDaemon, (void*(*)(void*))DaemonKeepAlive, (void*)NULL);

    return GMI_SUCCESS;
}


void DaemonStop()
{
    l_DaemonKeepAliveStop = true;
    return;
}


void DaemonUnregister(void)
{
    GMI_DaemonUnInit( &l_DaemonHandler );
    return;
}

