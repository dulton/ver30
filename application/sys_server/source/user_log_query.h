#ifndef __USER_LOG_H__
#define __USER_LOG_H__
#include <time.h>
#include "user_log_queryer.h"
#include "gmi_system_headers.h"
#include "sys_env_types.h"

class UserLogQuery
{
public:
	UserLogQuery(void);
	~UserLogQuery();
	GMI_RESULT Initialize();
	GMI_RESULT Deinitialize();
	GMI_RESULT Query(SysPkgLogInfoSearch *SysLogInfoSearch, SysPkgLogInfoInt *SysLogInfoInt, SysPkgLogInfo SysLogInfo[]);
private:
	GMI_RESULT SubQuery(int32_t MajorType, int32_t MinorType, char_t StartTim[32], char_t EndTim[32]);
	GMI_RESULT CheckTime(struct tm* ptTime);
private:
	UserLogQueryer m_UserLogQueryer; //object
	UserLogStorageInfo *m_UserLogPtr;//log
	uint32_t       m_TotalUserLogNum;  //total log match with query condition
	uint32_t       m_LeftUserLogNum;
	SysPkgLogInfoSearch SysLogInfoSearch_Last;//last user query condition
};


#endif
