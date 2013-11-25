
#include <signal.h>
#include "discovery.h"
#include "daemon.h"
#include "log.h"
#include "soapH.h"
#include "stdsoap2.h"
#include "threads.h"
#include "service_utilitly.h"
#include "service_ptz.h"
#include "sys_client.h"
#include "ipc_fw_v3.x_resource.h"
#include "sys_env_types.h"
#include "gmi_system_headers.h"


static boolean_t l_ServerStopFlag = false;

static void *ProcessRequest(void *soap);
static GMI_RESULT ServerLoop(struct soap *soap);


void SignalHandler(int signum)
{
    if (signum == SIGINT)
    {
        printf("receive SIGINT (Ctrl-C) signal\n");
        exit(0);
    }
    else if (signum == SIGIO)
    {
        printf("receive SIGIO signal\n");
    }
    else if (signum == SIGPIPE)
    {
        printf("receive SIGPIPE signal\n");
    }
    else if (signum == SIGHUP)
    {
        printf("receive SIGHUP signal\n");
        exit(0);
    }
    else if (signum == SIGQUIT)
    {
        printf("receive SIGQUIT signal\n");
        exit(0);
    }
    else if (signum == SIGSEGV)
    {
        sleep(1);
    }
    else
    {
        printf("receive unknown signal. signal = %d\n", signum);
    }
    return;
}


int main(int argc, const char *argv[])
{
    long_t master;
    GMI_RESULT Result = GMI_SUCCESS;
    struct soap soap;

    int32_t Signal;
    sigset_t NewMask;
    sigset_t OldMask;
    struct sigaction Sa;

    //signal
    Sa.sa_handler = SignalHandler;
    sigfillset(&Sa.sa_mask);
    Sa.sa_flags = SA_NOMASK;
    sigemptyset(&NewMask);
    for (Signal = 1; Signal <= _NSIG; ++Signal)
    {
        if ( ( Signal == SIGIO )
                || ( Signal == SIGPOLL )
                || ( Signal == SIGINT )
                || ( Signal == SIGQUIT )
                || ( Signal == SIGHUP )
                || ( Signal == SIGPIPE )
                || ( Signal == SIGSEGV )
           )
        {
            sigaction(Signal, &Sa, NULL);
        }
        else
        {
            sigaddset(&NewMask, Signal);
        }
    }
    sigprocmask(SIG_BLOCK, &NewMask, &OldMask);

    //log init
	ONVIF_INFO("LogInitial start\n");
    Result = LogInitial();
    if (FAILED(Result))
    {
        ONVIF_ERROR("LogInitial fail, Result = 0x%lx\n", Result);
        return Result;
    }
	ONVIF_INFO("LogInitial end\n");

	ONVIF_INFO("DaemonRegister start\n");
    //daemon register to daemon server
    Result = DaemonRegister();
    if (FAILED(Result))
    {
    	ONVIF_ERROR("DaemonRegister fail, Result = 0x%lx\n", Result);
    	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, " DaemonRegister fail, Result = 0x%lx\n", Result);
    	return Result;
    }
	ONVIF_INFO("DaemonRegister end\n");

    //system initial
    Result = SysInitialize(GMI_ONVIF_AUTH_PORT);
    if (FAILED(Result))
    {    	
    	DaemonUnregister();
    	ONVIF_ERROR("SysInitialize fail, Result = 0x%lx\n", Result);
    	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, " SysInitialize fail, Result = 0x%lx\n", Result);
    	return Result;
    }   
    
    //soap server init
    soap_init1(&soap, SOAP_ENC_MTOM);
    //do not set send & recv timeout, promise recv complete.
    //11/6/2013 guoqiang.lu
    #if 1
    soap.send_timeout    = 5;
    soap.recv_timeout    = 5;    
    #else
    soap.socket_flags    = MSG_NOSIGNAL;
    soap.accept_flags   |= SO_LINGER;
    soap.connect_flags  |= SO_LINGER;
    soap.linger_time     = 2;
    soap.bind_flags      = SO_REUSEADDR;
    soap.send_timeout    = 2;
    soap.recv_timeout    = 2;
    soap.accept_timeout  = 10;
    soap.connect_timeout = 10;
    soap.keep_alive      = 5;
    #endif
    soap_set_mode(&soap, SOAP_C_UTFSTRING);
    master = soap_bind(&soap, NULL, DEFAULT_SERVER_PORT, 30);
    if (!soap_valid_socket(master))
    {      
    	ONVIF_ERROR("soap_bind fail, Result = 0x%lx\n", Result);
    	DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, " soap_bind fail, Result = 0x%lx\n", Result);
        soap_print_fault(&soap, stderr);
        SysDeinitialize();
        //daemon unregister
        DaemonUnregister();
        exit(1);
    }

    //main server start
    ServerLoop(&soap);

    ONVIF_INFO("soap_end start\n");
    //soap release
    soap_end(&soap);
    soap_done(&soap);
    ONVIF_INFO("soap_end stop\n");

   	ONVIF_INFO("SysDeinitialize start\n");
    SysDeinitialize();
    ONVIF_INFO("SysDeinitialize end\n");
    ONVIF_INFO("DaemonUnregister start\n");
    //daemon unregister
    DaemonUnregister();
    ONVIF_INFO("DaemonUnregister end\n");

    return 0;
}


int l_ThreadCnt;

static GMI_RESULT ServerLoop(struct soap *soap)
{
    long_t Request;
    long_t Ret;
    SOAP_SOCKET SoapSock;
    struct linger TcpLinger;

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s In.........\n", __func__);

	ONVIF_INFO("%s In.........\n", __func__);
    //onvif probe service start
    OnvifDevProbeServiceStart();
    PtzTimeoutProcessStart();

    ONVIF_INFO("DaemonStart start\n");
    //daemon start
    DaemonStart();
    ONVIF_INFO("DaemonStart end\n");
    
    //accept
    for (Request = 1; l_ServerStopFlag != true; Request++)
    {
        soap->accept_flags  |= SO_LINGER;
        soap->connect_flags |= SO_LINGER;
        soap->linger_time    = 1;

        SoapSock = soap_accept(soap);
        if (!soap_valid_socket(SoapSock))
        {
            continue;
        }

        TcpLinger.l_onoff  = 1;
        TcpLinger.l_linger = 1;
        Ret = setsockopt(SoapSock, SOL_SOCKET, SO_LINGER, (char*)&TcpLinger, sizeof(struct linger));
        if (Ret == -1)
        {
            printf("set socket LINGER failed !! errno = %d\n", errno);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "set socket LINGER failed !! errno = %d\n", errno);
            close(SoapSock);
            SoapSock = -1;
            continue;
        }

        struct soap *tsoap = NULL;
        THREAD_TYPE TidSoap;
        void *ThreadRet;

		printf("=======>Thread count %d\n", l_ThreadCnt);
        tsoap = soap_copy(soap);
        if (tsoap)
        {
            tsoap->user = (void*)(SOAP_SOCKET)SoapSock;
            //THREAD_CREATE(&TidSoap, ProcessRequest, (void*)tsoap); //guoqiang.lu mask, 11/20/2013
            pthread_create(&TidSoap, NULL, ProcessRequest, (void*)tsoap);
            pthread_join(TidSoap, &ThreadRet);
        }
        else
        {
            continue;
        }
    }

    //daemon stop
    DaemonStop();
    //probe service stop
    OnvifDevProbeServiceStop();
	ONVIF_INFO("%s out.........\n", __func__);
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "%s out.........\n", __func__);
    return GMI_SUCCESS;
}


static void *ProcessRequest(void *soap)
{
    int32_t Ret;
    struct soap *tsoap = (struct soap*)soap;

    //pthread_detach(pthread_self()); //guoqiang.lu mask,11/20/2013
    
	l_ThreadCnt++;
	
    Ret = soap_serve(tsoap);   
    if (SOAP_OK != Ret
            && (tsoap->error != SOAP_EOF
                || (tsoap->errnum != 0 && !(tsoap->omode & SOAP_IO_KEEPALIVE))))
    {
        fprintf(stderr, "Thread %d completed with failure %d, Ret %d\n", \
        	(int)(SOAP_SOCKET)tsoap->user, tsoap->error, Ret);        
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Thread %d completed with failure %d\n", (int)(SOAP_SOCKET)tsoap->user, tsoap->error);
        soap_print_fault(tsoap, stderr);
    }

    soap_destroy(tsoap);
    soap_end(tsoap);
    soap_done(tsoap);
    soap_free(tsoap); 
    close((SOAP_SOCKET)tsoap->user);

	l_ThreadCnt--;
	
	printf("======>%s %d exit, Total ThreadCnt %d\n", __func__, __LINE__, l_ThreadCnt);
    pthread_exit(NULL);
}

