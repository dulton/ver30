#ifndef __SYS_ALARM_INFO_REPORT_H__
#define __SYS_ALARM_INFO_REPORT_H__
#include "alarm_session.h"
#include "gmi_system_headers.h"
#include "sys_env_types.h"


class SysAlarmInfoReport
{
public:
	SysAlarmInfoReport();
	~SysAlarmInfoReport();
	GMI_RESULT Initialize();
	GMI_RESULT Deinitialize();
	void_t Report(SysPkgAlarmInfor *SysAlarmInfor);
	inline unsigned long long htonll(unsigned long long val)
	{
	    return (((unsigned long long )htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));    
	}
private:
	GMI_RESULT GSMT_Pakage(SysPkgAlarmInfor *SysAlarmInfor, uint8_t *Buffer, size_t BufferSize, size_t *TotalLen);
private:
	ReferrencePtr<AlarmSession>    m_AlarmSession;
	ReferrencePtr<uint8_t, DefaultObjectsDeleter> m_Package;
	size_t m_PackageSize;
};

#endif


