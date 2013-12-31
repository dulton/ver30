
#include <libev/ev.h>
#include <sdk_server_main.h>
#include <signal.h>
#include <execinfo.h>
#include <gmi_config_api.h>
#include <sdk_server_debug.h>

#define  GMI_SETTING_XML  "/opt/config/gmi_setting.xml"
#define  SDK_SERVER_PORT_PATH "/Config/NetworkPort/Extern/"
#define  SDK_SERVER_PORT_ITEM "SDK_ServerPort"
#define  SDK_SERVER_PORT_DEFAULT  30000

static int st_RunLoop = 1;




void SigStop(int signo)
{
    if(signo == SIGINT ||
            signo == SIGTERM)
    {
        st_RunLoop = 0;
    }
    else if(signo == SIGSEGV)
    {
        SDK_ASSERT(0!=0);
        exit(3);
    }
    return ;
}

int GetListenPort()
{
    GMI_RESULT gmiret;
    FD_HANDLE xmlhd=NULL;
    int serverport;

    DEBUG_INFO("search<%s> <%s>\n",SDK_SERVER_PORT_PATH,SDK_SERVER_PORT_ITEM);
    gmiret = GMI_XmlOpen(GMI_SETTING_XML,&xmlhd);
    if(gmiret != GMI_SUCCESS)
    {
        DEBUG_INFO("\n");
        goto set_default;
    }
    gmiret = GMI_XmlRead(xmlhd,SDK_SERVER_PORT_PATH,
                         SDK_SERVER_PORT_ITEM,
                         SDK_SERVER_PORT_DEFAULT,
                         &serverport,GMI_CONFIG_READ_ONLY);
    if(gmiret != GMI_SUCCESS)
    {
        DEBUG_INFO("\n");
        goto set_default;
    }
    GMI_XmlFileSave(xmlhd);
    xmlhd = NULL;

    DEBUG_INFO("\n");
    return serverport;


set_default:
    if(xmlhd)
    {
        GMI_XmlFileSave(xmlhd);
    }
    xmlhd= NULL;
    return SDK_SERVER_PORT_DEFAULT;
}

int main(int argc,char* argv[])
{
    int ret;
    int port;
    sighandler_t sigret;
    SdkServerMain *pMain=NULL;

    DEBUG_BUFFER_FMT(NULL,0,"\n");
    ret = InitializeSdkLog();
    if(ret < 0)
    {
        ERROR_INFO("could not initialize sdk log\n");
    }
    DEBUG_BUFFER_FMT(NULL,0,"\n");

    port = GetListenPort();
    if(port >= (1<<16))
    {
        port = SDK_SERVER_PORT_DEFAULT;
    }
    DEBUG_INFO("port %d\n",port);
    sigret = signal(SIGINT,SigStop);
    if(sigret == SIG_ERR)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    sigret = signal(SIGTERM,SigStop);
    if(sigret == SIG_ERR)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    /*SIGPIPE when the peer close socket and we send the packets*/
    sigret = signal(SIGPIPE,SIG_IGN);
    if(sigret == SIG_ERR)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    sigret = signal(SIGSEGV,SigStop);
    if(sigret == SIG_ERR)
    {
        ret = -errno ? -errno : -1;
        goto out;
    }

    pMain = new SdkServerMain(port,&st_RunLoop,20);
    SDK_ASSERT(pMain);

    ret = pMain->RunLoop();
out:
    if(pMain)
    {
        delete pMain;
    }
    pMain = NULL;
    DeInitializeSdkLog();
    return ret;
}
