

#ifndef  __SDK_SERVER_DEBUG_H__
#define  __SDK_SERVER_DEBUG_H__

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <execinfo.h>
#include <ipc_fw_v3.x_resource.h>
#include <ipc_fw_v3.x_setting.h>
#include <log_utils/log_client.h>

#define USE_SDK_LOG_DEBUG
//#undef USE_SDK_LOG_DEBUG

#define SETERRNO(ret)  (errno = (ret))
#define GETERRNO() ( errno ? errno : 1)

#ifdef USE_SDK_LOG_DEBUG

#define  LOG_CRITICAL_LEVEL    0
#define  LOG_ERROR_LEVEL       1
#define  LOG_WARNING_LEVEL     2
#define  LOG_INFO_LEVEL        3
#define  LOG_DEBUG_LEVEL       4

#define ERROR_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);SdkLogFmt(LOG_ERROR_LEVEL,__FILE__,__ASSERT_FUNCTION,__LINE__,__VA_ARGS__);}while(0)
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);SdkLogFmt(LOG_DEBUG_LEVEL,__FILE__,__ASSERT_FUNCTION,__LINE__,__VA_ARGS__);}while(0)
#define BACK_TRACE()  do{Debug_CallBackTrace(__FILE__,__LINE__,NULL);SdkLogCallBackTrace(__FILE__,__LINE__,NULL);}while(0)
#define BACK_TRACE_FMT(...) do{Debug_CallBackTrace(__FILE__,__LINE__,__VA_ARGS__);SdkLogCallBackTrace(__FILE__,__LINE__,__VA_ARGS__);}while(0)

#define SDK_ASSERT(expr) \
do\
{\
    if (!(expr))\
    {\
        BACK_TRACE();\
        fprintf(stderr,"%s:%d\t%s asserted\n",__FILE__,__LINE__,#expr);\
        SdkLogAssertError(__FILE__,__LINE__,#expr);\
        abort();\
    }\
}\
while(0)

#define  DEBUG_BUFFER(buf,len) do{SdkDebugBuffer(__FILE__,__LINE__,(unsigned char*)(buf),(len),NULL);SdkLogBufferFmt(LOG_DEBUG_LEVEL,__FILE__,__ASSERT_FUNCTION,__LINE__,(unsigned char*)(buf),(len),NULL);}while(0)
#define  DEBUG_BUFFER_FMT(buf,len,...) do{SdkDebugBuffer(__FILE__,__LINE__,(unsigned char*)(buf),(len),__VA_ARGS__);SdkLogBufferFmt(LOG_DEBUG_LEVEL,__FILE__,__ASSERT_FUNCTION,__LINE__,(unsigned char*)(buf),(len),__VA_ARGS__);}while(0)



#else


#define ERROR_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);DEBUG_LOG(g_pSdkLogClient,e_DebugLogLevel_Exception,__VA_ARGS__);}while(0)
#define DEBUG_INFO(...) do{fprintf(stderr,"%s:%s:%d[%ld]\t",__FILE__,__ASSERT_FUNCTION,__LINE__,time(NULL));fprintf(stderr,__VA_ARGS__);DEBUG_LOG(g_pSdkLogClient,e_DebugLogLevel_Info,__VA_ARGS__);}while(0)
#define BACK_TRACE()  Debug_CallBackTrace(__FILE__,__LINE__,NULL)
#define BACK_TRACE_FMT(...) do{Debug_CallBackTrace(__FILE__,__LINE__,__VA_ARGS__);}while(0)

#define SDK_ASSERT(expr) \
do\
{\
    if (!(expr))\
    {\
        BACK_TRACE();\
        fprintf(stderr,"%s:%d\t%s asserted\n",__FILE__,__LINE__,#expr);\
        DEBUG_LOG(g_pSdkLogClient,e_DebugLogLevel_Exception,"%s:%d\t%s asserted\n",__FILE__,__LINE__,#expr);\
        abort();\
    }\
}\
while(0)

#define  DEBUG_BUFFER(buf,len) SdkDebugBuffer(__FILE__,__LINE__,(unsigned char*)(buf),(len),NULL)
#define  DEBUG_BUFFER_FMT(buf,len,...) SdkDebugBuffer(__FILE__,__LINE__,(unsigned char*)(buf),(len),__VA_ARGS__)


#endif


#ifdef __cplusplus
extern "C" {
#endif
void SdkDebugBuffer(const char* file,int lineno,unsigned char* pBuffer,int buflen,const char* fmt,...);
void Debug_CallBackTrace(const char* file,int lineno,const char* fmt,...);

#ifdef USE_SDK_LOG_DEBUG

void SdkLogCallBackTrace(const char * file,int lineno,const char* fmt,...);
void SdkLogAssertError(const char* file,int lineno,const char* fmt);
void SdkLogFmt(int loglevel ,const char* file,const char* func,int lineno,const char* fmt,...);
void SdkLogBufferFmt(int loglevel,const char* file,const char* func,int lineno,unsigned char* pBuffer,int buflen,const char* fmt,...);




#endif  /*USE_SDK_LOG_DEBUG*/


#ifdef __cplusplus
}
#endif





extern  LogClient* g_pSdkLogClient;

int InitializeSdkLog(void);
void DeInitializeSdkLog(void);

#endif /*__SDK_SERVER_DEBUG_H__*/

