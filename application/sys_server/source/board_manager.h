#ifndef __BOARD_MANAGER_H__
#define __BOARD_MANAGER_H__

#include <time.h>
#include "gmi_system_headers.h"

#define GMI_ZONE_SHANGHAI  "cat /usr/share/zoneinfo/Shanghai > /etc/localtime"
#define GMI_ZONE_UTC  "cat /usr/share/zoneinfo/UTC > /etc/localtime"

//ntp 
#define MAX_NTP_INTERVAL  (100000)

#define JAN_1970		0x83aa7e80      					
#define NTPFRAC(x)		(4294 * (x) + ((1981 * (x))>>11))					
#define USEC(x)			(((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))	

#define LI		0			
#define VN		3					
#define MODE	3					
#define STRATUM 0					
#define POLL 	4					
#define PREC 	-6		

#define		Data(i) 			(ntohl(((unsigned int *)data)[i]))
#define		MKSEC(ntpt)			((ntpt).coarse - JAN_1970) 
#define		MKUSEC(ntpt)		(USEC((ntpt).fine)) 
#define		TTLUSEC(sec,usec)	(( long   long )(sec)*1000000 + (usec)) 
#define		GETSEC(us)			((us)/1000000)  
#define		GETUSEC(us)			((us)%1000000) 

typedef struct
{
    uint32_t coarse;		
    uint32_t fine;			
}NTPTIME ;


class BoardManager
{
public:
    BoardManager();
    ~BoardManager();
    GMI_RESULT Initialize();
    GMI_RESULT Deinitialize();
    GMI_RESULT GetTime(struct tm *Time);
    GMI_RESULT SetTime(struct tm *Time);
    GMI_RESULT SetZone(char_t *Zone);
    GMI_RESULT SetNtpServer(char_t *NtpAddr);
    GMI_RESULT SetNtp(boolean_t NtpEnable, int32_t NtpInterval);
    
private:
    GMI_RESULT CheckTime( struct tm* ptTime );
    GMI_RESULT CheckIPv4String(char_t *Ip);
    static void* TimeAdjustNtpThread(void *);
    void_t* TimeAdjustNtp(void);
    GMI_RESULT NtpUpdateTime(struct timeval *ptime);
    GMI_RESULT PacketNtpTime(uint32_t *data);
    GMI_RESULT ParseNtpData(uint32_t *data);
private:
	boolean_t   m_NtpEnable;
	int32_t     m_NtpInterval;
	boolean_t   m_NtpAjustTimeOnce;
	boolean_t   m_NtpServerIpIsChannged;
	char_t      m_NtpServerIp[32];
	boolean_t   m_ThreadExitFlag;
	GMI_Thread  m_TimeAdjustNtpThread;
};
#endif

