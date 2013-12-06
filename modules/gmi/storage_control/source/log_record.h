#ifndef __LOG_RECORD_H__
#define __LOG_RECORD_H__
#ifdef __cplusplus
extern "C"
{
#endif

enum DebugLogLevel
{
	e_DebugLogLevel_Exception	  = 1,// program exception, including real exception case and error
	e_DebugLogLevel_Warning 	  = 2,// warning, for example, rare case happen, or data overwrite in ring buffer
	e_DebugLogLevel_Info		  = 3,// only print, for example, call trace, or I/O data print
	e_DebugLogLevel_Loop		  = 4,// frequent log info, for example, loop and multimedia frame trace
};

typedef int LogClient;
	
extern LogClient LogClientHd;
void ERR_PRINT_LOG(LogClient *LogClientHd, enum DebugLogLevel PrtLevel, char *FileName, const char *FuncName, int LineNum, const char *fmt, ...);
void ERR_PRINT_LOG_TEST(LogClient *LogClientHd, enum DebugLogLevel PrtLevel, char *FileName, const char *FuncName, int LineNum, const char *fmt, ...);

//#define DEBUG_LOG( Client, LogLevel, Format, ... )  (Client)->DebugLogV( LogLevel, __FILE__, __FUNCTION__, __LINE__, Format, ##__VA_ARGS__ )

#define DEBUG_LEVEL 4

#if 0
#define DEBUG_LOG(Client, LogLevel, Format, ... )
#define DEBUG_LOG_TEST(Client, LogLevel, Format, ... )

#else

#define DEBUG_LOG(Client, LogLevel, Format, ... )  do{ \
														   if(LogLevel <= DEBUG_LEVEL) {\
                                                           ERR_PRINT_LOG(Client, LogLevel, __FILE__, __FUNCTION__, __LINE__, Format, ##__VA_ARGS__); \
														   	}\
                                                       }while(0)

#define DEBUG_LOG_TEST(Client, LogLevel, Format, ... )  do{ \
															  if(LogLevel <= DEBUG_LEVEL) {\
															  ERR_PRINT_LOG_TEST(Client, LogLevel, __FILE__, __FUNCTION__, __LINE__, Format, ##__VA_ARGS__); \
															   }\
														  }while(0)

#endif



#ifdef __cplusplus
}; //end of extern "C" {
#endif

#endif //__LOG_RECORD_H__

