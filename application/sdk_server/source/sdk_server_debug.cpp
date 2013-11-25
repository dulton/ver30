
#include <sdk_server_debug.h>
#include <stdarg.h>

LogClient* g_pSdkLogClient=NULL;

#define INNER_DEBUG(...) do{fprintf(stderr,"%s:%d\t",__FILE__,__LINE__);fprintf(stderr,__VA_ARGS__);}while(0)

void SdkDebugBuffer(const char* file,int lineno,unsigned char* pBuffer,int buflen,const char* fmt,...)
{
    unsigned char* pCurPtr=pBuffer;
    int i;
    va_list ap;


    fprintf(stderr,"At [%s:%d] pointer 0x%p length(%d)\t",file,lineno,pBuffer,buflen);
    if(fmt)
    {
        va_start(ap,fmt);
        vfprintf(stderr,fmt,ap);
    }
    for(i=0,pCurPtr = pBuffer; i<buflen; i++,pCurPtr++)
    {
        if((i%16) == 0)
        {
            fprintf(stderr,"\n0x%08x\t",i);
        }
        fprintf(stderr,"0x%02x ",*pCurPtr);
    }

    fprintf(stderr,"\n");
    return ;
}


void Debug_CallBackTrace(const char * file,int lineno,const char* fmt,...)
{

    void *array[30];
    size_t size;
    int i;
    time_t curt;

    char **strings=NULL;

    size = backtrace(array,30);
    strings = backtrace_symbols(array,size);
	if (strings == NULL)
	{
		return ;
	}
    curt = time(NULL);
    fprintf(stderr,"[%s:%d](%ld)\n",file,lineno,curt);
    if(fmt)
    {
        va_list ap;
        va_start(ap,fmt);
        vfprintf(stderr,fmt,ap);
        fprintf(stderr,"\n");
    }
    for(i=0; i<(int)size; i++)
    {
        fprintf(stderr,"%s\n",strings[i]);
    }
	free(strings);
	strings = NULL;
    return ;
}


#ifndef USE_SDK_LOG_DEBUG
int InitializeSdkLog(void)
{
    int ret=0;
    if(g_pSdkLogClient==NULL)
    {
        GMI_RESULT gmiret;

        DEBUG_BUFFER_FMT(NULL,0,"\n");
        g_pSdkLogClient = new LogClient();

        DEBUG_BUFFER_FMT(NULL,0,"\n");

        gmiret = g_pSdkLogClient->Initialize(GMI_LOG_MODULE_SDK_ID,GMI_LOG_MODULE_SDK_NAME,LOG_SDK_DEFAULT_CLIENT_PIPE_NAME,LOG_SDK_DEFAULT_CLIENT_PIPE_MUTEX_ID,
                                             LOG_SDK_DEFAULT_PEER_PIPE_NAME,LOG_SDK_DEFAULT_PEER_PIPE_MUTEX_ID,LOG_SERVER_DEFAULT_PIPE_NAME,LOG_SERVER_DEFAULT_PIPE_MUTEX_ID);

        DEBUG_BUFFER_FMT(NULL,0,"gmiret 0x%08lx\n",gmiret);
        if(gmiret != GMI_SUCCESS)
        {
            ret = -errno ? -errno : -1;
            delete g_pSdkLogClient;
            g_pSdkLogClient = NULL;
        }
    }
    return ret;
}

void DeInitializeSdkLog(void)
{
    if(g_pSdkLogClient)
    {
        g_pSdkLogClient->Deinitialize();
        delete g_pSdkLogClient;
    }
    g_pSdkLogClient = NULL;
    return;
}

#else




typedef struct
{
    int m_LogLevel;
    char m_LogCotent[1000];
} log_buffer_t;
static std::vector<log_buffer_t*>  st_LogBufferVecs;

static int st_LogLevel=LOG_DEBUG_LEVEL;
/*this is the water mark to notify the thread*/
static unsigned int st_LogNotifyWaterMark=20;
static FILE* st_LogFp=NULL;
static int st_LogIdx=0;
static int st_LogFileSize=0;
static int st_LogFileMaxSize=0x100000;
static const char * st_LogFileBase="/opt/log/sdk_log";
static int st_LogThreadRunning=0;
static int st_LogThreadExited=1;

static pthread_cond_t st_LogCond;
static pthread_mutex_t st_LogCondMutex;
static int st_LogCondWaited=0;
static int st_LogInited=0;
static int st_LogMaxIdx = 2;
static GMI_Mutex st_LogMutex;
static GMI_Thread st_LogThread;

int LogWait(int timeout)
{
    int ret;
    struct timeval nowtime;
    struct timespec outtime;

    ret = pthread_mutex_lock(&st_LogCondMutex);
    if(ret != 0)
    {
        ret = -errno ? -errno : -1;
        fprintf(stderr,"Wait could not get LogCondMutex error(%d)\n",ret);
        return ret;
    }

    gettimeofday(&nowtime,NULL);
    outtime.tv_sec = nowtime.tv_sec + timeout;
    outtime.tv_nsec = nowtime.tv_usec * 1000;

    st_LogCondWaited = 1;
    ret = pthread_cond_timedwait(&st_LogCond,&st_LogCondMutex,&outtime);
    st_LogCondWaited = 0;
    if(ret == 0)
    {
        ret = 1;
    }
    else if(ret == ETIMEDOUT)
    {
        ret = 0;
    }
    else
    {
        ret = -errno ? -errno : -1;
    }

    pthread_mutex_unlock(&st_LogCondMutex);
    return ret;
}

int LogNotify()
{
    int ret=0;
    ret = pthread_mutex_lock(&st_LogCondMutex);
    if(ret != 0)
    {
        ret = -errno ? -errno : -1;
        fprintf(stderr,"Notify could not get LogCondMutex error(%d)\n",ret);
        return ret;
    }

    if(st_LogCondWaited)
    {

        ret = pthread_cond_signal(&st_LogCond);
        if(ret == 0)
        {
            ret = 1;
        }
        else
        {
            ret = 0;
        }
    }

    pthread_mutex_unlock(&st_LogCondMutex);
    return ret;
}

static void __FreeLogBuffer(log_buffer_t* pLogBuffer)
{
    if(pLogBuffer)
    {
        free(pLogBuffer);
    }
    return ;
}

static log_buffer_t* __AllocateLogBuffer(int level)
{
    log_buffer_t *pLogBuffer=NULL;

    pLogBuffer =(log_buffer_t*) calloc(sizeof(*pLogBuffer),1);
    if(pLogBuffer)
    {
        pLogBuffer->m_LogLevel = level;
    }
    return pLogBuffer;
}


static log_buffer_t* __PullLog()
{
    GMI_RESULT gmiret;
    log_buffer_t *pLogBuffer=NULL;

    gmiret = st_LogMutex.Lock(TIMEOUT_INFINITE);
    assert(gmiret == GMI_SUCCESS);
    if(st_LogBufferVecs.size() > 0)
    {
        pLogBuffer = st_LogBufferVecs[0];
        st_LogBufferVecs.erase(st_LogBufferVecs.begin());
    }

    st_LogMutex.Unlock();
    return pLogBuffer;
}

static int __PushLog(log_buffer_t* pLogBuffer)
{
    GMI_RESULT gmiret;
    int ret = 0;

    if(st_LogThreadExited > 0)
    {
        /*exited ,so we should not inserted */
        fprintf(stderr,"Exited!!!!!%s",pLogBuffer->m_LogCotent);
        __FreeLogBuffer(pLogBuffer);
        return 0;
    }
    gmiret = st_LogMutex.Lock(TIMEOUT_INFINITE);
    assert(gmiret == GMI_SUCCESS);
    st_LogBufferVecs.push_back(pLogBuffer);
    if(st_LogBufferVecs.size() > st_LogNotifyWaterMark)
    {
        ret = 1;
    }
    st_LogMutex.Unlock();

    if(ret > 0)
    {
        LogNotify();
    }
    return ret;
}


void CopyFileToOld(const char* fname)
{
	int ret;
	struct stat statbuf;
	char oldfname[128];

	ret = stat(fname,&statbuf);
	if (ret >= 0)
	{
		if (S_ISREG(statbuf.st_mode))
		{
			/*now we should move */
			snprintf(oldfname,sizeof(oldfname),"%s_old",fname);
			ret = stat(oldfname,&statbuf);
			if (ret >= 0)
			{
				/*we have file ,so remove it*/
				unlink(oldfname);
			}
			
			ret = rename(fname,oldfname);
			if (ret < 0)
			{
				fprintf(stderr,"could not rename (%s => %s) error(%d)\n",fname,oldfname,errno);
				unlink(fname);
			}
		}
		else
		{
			fprintf(stderr,"(%s) not regular file 0x%08x\n",fname,statbuf.st_mode);
			unlink(fname);
		}
	}

	return ;
}


int __ChangeLogFileFp()
{
    char logfile[32];
    int ret;
    if(st_LogFp)
    {
        fclose(st_LogFp);
    }
    st_LogFp = NULL;
    st_LogFileSize = 0;
    if(st_LogIdx < st_LogMaxIdx)
    {
        st_LogIdx ++;
    }
    else
    {
        st_LogIdx = 0;
    }

    snprintf(logfile,sizeof(logfile),"%s%d",st_LogFileBase,st_LogIdx);

	CopyFileToOld(logfile);

    st_LogFp = fopen(logfile,"w");
    if(st_LogFp == NULL)
    {
        ret = -errno ? -errno : -1;
        fprintf(stderr,"Change can not open (%s) %d\n",logfile,ret);
        return ret;
    }

    rewind(st_LogFp);
    return 0;
}

static void __WriteLog(log_buffer_t* pLogBuffer)
{
    int datalen = 0;
    int ret;

    if(st_LogFp == NULL)
    {
        fprintf(stderr,"LogFp NULL not write log (%s)\n",pLogBuffer->m_LogCotent);
        return ;
    }

    datalen = strlen(pLogBuffer->m_LogCotent);

    ret = fwrite(pLogBuffer->m_LogCotent,datalen,1,st_LogFp);
    if(ret != 1)
    {
        ret = -errno ? -errno : -1;
        fprintf(stderr,"Write File(%s%d) error(%d)\n",st_LogFileBase,st_LogIdx,ret);
        ret = __ChangeLogFileFp();
        if(ret < 0)
        {
            return ;
        }

        ret = fwrite(pLogBuffer->m_LogCotent,datalen,1,st_LogFp);
        if(ret != 1)
        {
            ret = -errno ? -errno : -1;
            fprintf(stderr,"Write doulble failed with (%s%d) error(%d)\n",st_LogFileBase,st_LogIdx,ret);
            return ;
        }
    }

    st_LogFileSize += datalen;
    if(st_LogFileSize >= st_LogFileMaxSize)
    {
        ret = __ChangeLogFileFp();
        if(ret < 0)
        {
            return ;
        }
    }
    return ;
}

static void __FlushLog()
{
    log_buffer_t* pLogBuffer=NULL;

    while(1)
    {
        assert(pLogBuffer == NULL);
        pLogBuffer = __PullLog();
        if(pLogBuffer == NULL)
        {
            break;
        }

        __WriteLog(pLogBuffer);
        __FreeLogBuffer(pLogBuffer);
        pLogBuffer = NULL;
    }

	if (st_LogFp)
	{
		fflush(st_LogFp);
	}

    return ;
}




void* FlushLogThread(void* arg)
{
    int ret;
    while(st_LogThreadRunning)
    {
        /*wait for 5 seconds*/
        ret = LogWait(5);
        if(ret < 0)
        {
            /*yes something wrong ,so we should sleep for a 100 millseconds*/
            usleep(100000);
        }
        if(st_LogThreadRunning == 0)
        {
            break;
        }

        __FlushLog();
    }

    st_LogThreadExited = 1;
    return (void*)0;
}


int __LogAppendv(log_buffer_t *pLogBuffer,const char* fmt,va_list ap)
{
    char* pCurPtr=NULL;
    int slen=0,leftlen;
    int ret;
    slen = strlen(pLogBuffer->m_LogCotent);

    pCurPtr = pLogBuffer->m_LogCotent + slen;
    leftlen = sizeof(pLogBuffer->m_LogCotent) - slen;

    ret = vsnprintf(pCurPtr,leftlen,fmt,ap);
    if(ret >= leftlen)
    {
        /*not let expand string,*/
        *pCurPtr = '\0';
        return 1;
    }

    return 0;

}

int __LogAppend(log_buffer_t * pLogBuffer,const char * fmt,...)
{
    va_list ap;

    va_start(ap,fmt);

    return __LogAppendv(pLogBuffer,fmt,ap);
}

#define LOG_BUFFER_CHANGED(loglevel,...)   \
do\
{\
	int __tries=0;\
	int __ret = 0;\
	while(__tries < 2 )\
	{\
		if (pLogBuffer == NULL)\
		{\
			break;\
		}\
		__ret = __LogAppend(pLogBuffer,__VA_ARGS__);\
		if (__ret > 0)\
		{\
			__PushLog(pLogBuffer);\
			pLogBuffer = NULL;\
			pLogBuffer = __AllocateLogBuffer(loglevel);\
		}\
		else\
		{\
			break;\
		}\
		__tries ++;\
	}\
}while(0)

#define LOG_BUFFER_CHANGED_AP(loglevel,fmt,ap)   \
do\
{\
	int __tries=0;\
	int __ret = 0;\
	while(__tries < 2 )\
	{\
		if (pLogBuffer == NULL)\
		{\
			break;\
		}\
		__ret = __LogAppendv(pLogBuffer,(fmt),(ap));\
		if (__ret > 0)\
		{\
			__PushLog(pLogBuffer);\
			pLogBuffer = NULL;\
			pLogBuffer = __AllocateLogBuffer(loglevel);\
		}\
		else\
		{\
			break;\
		}\
		__tries ++;\
	}\
}while(0)


int __LogPrintfv(log_buffer_t* pLogBuffer,const char* fmt,va_list ap)
{
    vsnprintf(pLogBuffer->m_LogCotent,sizeof(pLogBuffer->m_LogCotent),fmt,ap);
    __PushLog(pLogBuffer);
    return 0;
}




void __LogPrintf(log_buffer_t* pLogBuffer,const char* fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    __LogPrintfv(pLogBuffer,fmt,ap);
    return;
}



void SdkLogAssertError(const char* file,int lineno,const char* fmt)
{
    log_buffer_t* pLogBuffer=NULL;
    time_t curt;
    if(st_LogInited == 0)
    {
        return ;
    }

    /*to make things uniquely*/
    st_LogInited = 0;

    pLogBuffer = __AllocateLogBuffer(LOG_CRITICAL_LEVEL);
    if(pLogBuffer == NULL)
    {
        return ;
    }

    curt = time(NULL);
    __LogPrintf(pLogBuffer,"%s:%d[%ld]\t%s",file,lineno,curt,fmt);
    /*now to flush*/
    st_LogThreadRunning = 0;
    while(st_LogThreadExited == 0)
    {
        LogNotify();
        usleep(10000);
    }
    __FlushLog();
    assert(st_LogThreadExited == 1);
    if(st_LogFp)
    {
        fclose(st_LogFp);
    }
    st_LogFp = NULL;
    st_LogIdx = 0;
    st_LogCondWaited = 0;
    st_LogFileSize = 0;
    pthread_cond_destroy(&st_LogCond);
    pthread_mutex_destroy(&st_LogCondMutex);
    st_LogMutex.Destroy();

    return;
}



void SdkLogFmt(int loglevel ,const char* file,const char* func,int lineno,const char* fmt,...)
{
    log_buffer_t* pLogBuffer=NULL;
    va_list ap;
    time_t curt;
    int ret;

    if(st_LogInited == 0 || loglevel > st_LogLevel)
    {
        return ;
    }

    pLogBuffer = __AllocateLogBuffer(loglevel);
    if(pLogBuffer == NULL)
    {
        return;
    }

    va_start(ap,fmt);
    curt = time(NULL);
    ret = __LogAppend(pLogBuffer,"%s:%s:%d [%ld]\t",file,func,lineno,curt);
    /*we do not any thing to this and we should see the next one*/
    if(fmt)
    {
        ret = __LogAppendv(pLogBuffer,fmt,ap);
        if(ret > 0)
        {
            __PushLog(pLogBuffer);
            pLogBuffer = NULL;
            pLogBuffer = __AllocateLogBuffer(loglevel);
            if(pLogBuffer)
            {
                __LogAppendv(pLogBuffer,fmt,ap);
            }
        }
    }

    if(pLogBuffer)
    {
        __PushLog(pLogBuffer);
    }
	
    pLogBuffer = NULL;

    return ;


}

void SdkLogBufferFmt(int loglevel,const char* file,const char* func,int lineno,unsigned char* pBuffer,int buflen,const char* fmt,...)
{
    log_buffer_t* pLogBuffer=NULL;
    va_list ap;
    time_t curt;
    int i;
    int ret;

    if(st_LogInited == 0 || loglevel > st_LogLevel)
    {
        return ;
    }

    pLogBuffer = __AllocateLogBuffer(loglevel);
    if(pLogBuffer == NULL)
    {
        return;
    }

    curt = time(NULL);

    LOG_BUFFER_CHANGED(loglevel,"%s:%s:%d [%d] buffer 0x%p size (%d)\t",file,func,lineno,curt,pBuffer,buflen);
    /*we do not any thing to this and we should see the next one*/
    if(fmt)
    {
        va_start(ap,fmt);
        ret = __LogAppendv(pLogBuffer,fmt,ap);
        if(ret > 0)
        {
            __PushLog(pLogBuffer);
            pLogBuffer = NULL;
            pLogBuffer = __AllocateLogBuffer(loglevel);
            if(pLogBuffer)
            {
                ret = __LogAppendv(pLogBuffer,fmt,ap);
            }
        }
    }

    for(i=0; i<buflen; i++)
    {
        if((i%16)==0)
        {
            LOG_BUFFER_CHANGED(loglevel,"\n0x%08x\t",i);
        }

        LOG_BUFFER_CHANGED(loglevel," 0x%02x",pBuffer[i]);
    }
	LOG_BUFFER_CHANGED(loglevel,"\n");

    if(pLogBuffer)
    {
        __PushLog(pLogBuffer);
    }
    pLogBuffer = NULL;
    return ;

}

void SdkLogCallBackTrace(const char * file,int lineno,const char* fmt,...)
{

    void *array[30];
    size_t size;
    int i;
    time_t curt;
    char **strings=NULL;
    log_buffer_t *pLogBuffer=NULL;


    if(st_LogInited == 0 || st_LogLevel < LOG_DEBUG_LEVEL)
    {
        return;
    }

    pLogBuffer = __AllocateLogBuffer(LOG_DEBUG_LEVEL);
    if(pLogBuffer == NULL)
    {
        return ;
    }

    size = backtrace(array,30);
    strings = backtrace_symbols(array,size);
	if (strings == NULL)
	{
		return ;
	}
    curt = time(NULL);
    LOG_BUFFER_CHANGED(LOG_DEBUG_LEVEL,"[%s:%d](%ld)\n",file,lineno,curt);
    if(fmt)
    {
		va_list ap;
		va_start(ap,fmt);
		LOG_BUFFER_CHANGED_AP(LOG_DEBUG_LEVEL,fmt,ap);
		LOG_BUFFER_CHANGED(LOG_DEBUG_LEVEL,"\n");
    }
    for(i=0; i<(int)size; i++)
    {
        LOG_BUFFER_CHANGED(LOG_DEBUG_LEVEL,"%s\n",strings[i]);
    }

    if(pLogBuffer)
    {
        __PushLog(pLogBuffer);
    }
    pLogBuffer = NULL;
	free(strings);
	strings = NULL;
    return ;
}



int InitializeSdkLog(void)
{
    int gmimutexinit=0;
    int condmutexinit=0;
    int condinit =0;
    int ret;
    int threadinit=0;
    GMI_RESULT gmiret;
    char logfile[32];

	DEBUG_BUFFER_FMT(NULL,0,"\n");

    if(st_LogInited > 0)
    {
        return 0;
    }

	DEBUG_BUFFER_FMT(NULL,0,"\n");
    /*now first to init the gmimutex*/
    gmiret = st_LogMutex.Create(NULL);
    if(gmiret != GMI_SUCCESS)
    {
        ret = -errno ? -errno : -1;
        fprintf(stderr,"Init can not init logmutex error(%d)\n",ret);
        goto fail;
    }

	DEBUG_BUFFER_FMT(NULL,0,"\n");
    gmimutexinit = 1;

    /*then we initialize the logfp*/
    st_LogIdx = 0;
    snprintf(logfile,sizeof(logfile),"%s%d",st_LogFileBase,st_LogIdx);
	CopyFileToOld(logfile);
	
	DEBUG_BUFFER_FMT(NULL,0,"\n");

    st_LogFp = fopen(logfile,"w");
    if(st_LogFp == NULL)
    {
        ret = -errno ? -errno : -1;
        fprintf(stderr,"can not open (%s) error(%d)\n",logfile,ret);
        goto fail;
    }
	DEBUG_BUFFER_FMT(NULL,0,"\n");

    rewind(st_LogFp);
    st_LogFileSize = 0;

    /*now to init cond mutex and cond*/
    ret = pthread_mutex_init(&st_LogCondMutex,NULL);
    if(ret != 0)
    {
        ret = -errno ? -errno : -1;
        fprintf(stderr,"Init can not init logcondmutex error(%d)\n",ret);
        goto fail;
    }

    condmutexinit = 1;

    ret = pthread_cond_init(&st_LogCond,NULL);
    if(ret != 0)
    {
        ret = -errno ? -errno : -1;
        fprintf(stderr,"Init can not init logcond error(%d)\n",ret);
        goto fail;
    }
	DEBUG_BUFFER_FMT(NULL,0,"\n");

    condinit = 1;

    /*now to init thread*/
    st_LogThreadRunning = 1;
    st_LogThreadExited = 0;
    gmiret = st_LogThread.Create(NULL,0,FlushLogThread,NULL);
    if(gmiret != GMI_SUCCESS)
    {
        /*to pretend to be exited because the thread not running*/
        st_LogThreadRunning = 0;
        st_LogThreadExited = 1;
        ret = -errno ? -errno : -1;
        fprintf(stderr,"Init can not create thread error(%d)\n",ret);
        goto fail;
    }
    threadinit = 1;
	DEBUG_BUFFER_FMT(NULL,0,"\n");

    gmiret = st_LogThread.Start();
	DEBUG_BUFFER_FMT(NULL,0,"\n");
    if(gmiret != GMI_SUCCESS)
    {
        st_LogThreadExited = 1;
        ret = -errno ? -errno : -1;
        fprintf(stderr,"Init can not start thread error(%d)\n",ret);
        goto fail;
    }
	DEBUG_BUFFER_FMT(NULL,0,"\n");

    assert(st_LogBufferVecs.size() == 0);

    /*now to start ok*/
    st_LogInited = 1;

	DEBUG_BUFFER_FMT(NULL,0,"\n");

    return 0;
fail:

    if(threadinit)
    {
        st_LogThreadRunning = 0;
        while(st_LogThreadExited == 0)
        {
            LogNotify();
            usleep(10000);
        }
        /*we should flush all the vectors*/
        __FlushLog();
        st_LogThread.Destroy();
        threadinit = 0;
    }

    assert(st_LogBufferVecs.size() == 0);

    if(st_LogFp)
    {
        fclose(st_LogFp);
        st_LogFp = NULL;
    }

    if(condinit)
    {
        pthread_cond_destroy(&st_LogCond);
        condinit  =0;
    }

    if(condmutexinit)
    {
        pthread_mutex_destroy(&st_LogCondMutex);
        condmutexinit = 0;
    }

    if(gmimutexinit)
    {
        st_LogMutex.Destroy();
        gmimutexinit = 0;
    }
    return ret;
}

void DeInitializeSdkLog(void)
{
    if(st_LogInited)
    {
        /*to make this ,will give no enter for other threads*/
        st_LogInited = 0;
        st_LogThreadRunning = 0;
        while(st_LogThreadExited==0)
        {
            LogNotify();
            usleep(10000);
        }

        assert(st_LogThreadExited == 1);
        __FlushLog();
        if(st_LogFp)
        {
            fclose(st_LogFp);
        }
        st_LogFp = NULL;
        st_LogCondWaited = 0;
        st_LogFileSize = 0;
        pthread_cond_destroy(&st_LogCond);
        pthread_mutex_destroy(&st_LogCondMutex);
        st_LogMutex.Destroy();
    }
    st_LogInited = 0;
    return;
}


#endif

