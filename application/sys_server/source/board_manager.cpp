#include "board_manager.h"
#include "log.h"
#include "ipc_fw_v3.x_resource.h"


BoardManager::BoardManager()
    :m_NtpEnable(false)
    ,m_NtpInterval(0)
    ,m_NtpAjustTimeOnce(false)
    ,m_NtpServerIpIsChannged(false)
    ,m_NtpServerIp()
    ,m_ThreadExitFlag(false)
    ,m_TimeAdjustNtpThread()
{
}


BoardManager::~BoardManager()
{
}


GMI_RESULT BoardManager::CheckTime( struct tm* ptTime )
{
    struct tm tTime;

    if (ptTime == NULL)
    {
        SYS_ERROR("Input param is null.\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Input param is null.\n");
        return GMI_FAIL;
    }

    int32_t Day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    memcpy(&tTime,ptTime,sizeof(struct tm));
    tTime.tm_year += 1900;
    tTime.tm_mon  += 1;

    if (tTime.tm_year < 2000 || tTime.tm_year > 2037)
    {
        return GMI_FAIL;
    }

    if (tTime.tm_mon < 1 || tTime.tm_mon > 12)
    {
        return GMI_FAIL;
    }

    if (tTime.tm_mon == 2 && (tTime.tm_year & 3) == 0)
    {
        Day[tTime.tm_mon-1] = 29;
    }

    if (tTime.tm_mday < 1 || tTime.tm_mday > Day[tTime.tm_mon-1])
    {
        return GMI_FAIL;
    }

    if (tTime.tm_hour < 0 || tTime.tm_hour > 23)
    {
        return GMI_FAIL;
    }

    if (tTime.tm_min < 0 || tTime.tm_min > 59)
    {
        return GMI_FAIL;
    }

    if (tTime.tm_sec < 0 || tTime.tm_sec > 59)
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT BoardManager::SetZone(char_t *Zone)
{
    char_t CmdBuffer[255];

    if (NULL == Zone)
    {
        return GMI_FAIL;
    }

    if (strcmp(Zone,"Shanghai") == 0)
    {
        memset(CmdBuffer, 0 ,sizeof(CmdBuffer));
        snprintf(CmdBuffer, 255, "%s", GMI_ZONE_SHANGHAI);
        if (system(CmdBuffer) != 0)
        {
            return GMI_FAIL;
        }
    }
    else if (strcmp(Zone,"UTC") == 0)
    {
        memset(CmdBuffer, 0 ,sizeof(CmdBuffer));
        snprintf(CmdBuffer, 255, "%s", GMI_ZONE_UTC);
        if (system(CmdBuffer) != 0)
        {
            return GMI_FAIL;
        }
    }
    else
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;

}


GMI_RESULT BoardManager::GetTime( struct tm* ptTime)
{
    time_t tTime;
    struct tm *ptToday;


    tTime = time(NULL);
    ptToday = localtime(&tTime);
    memcpy(ptTime, ptToday, sizeof(struct tm));


    return GMI_SUCCESS;
}


GMI_RESULT BoardManager::SetTime( struct tm* ptTime )
{
    int32_t nRet;
    char_t Cmd[255];

    struct timeval tv;
    struct timezone tz;

    if (ptTime == NULL)
    {
        SYS_ERROR("ptTime is null\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "ptTime is null\n");
        return  GMI_INVALID_PARAMETER;
    }

    nRet = CheckTime(ptTime);
    if (FAILED(nRet))
    {
        SYS_ERROR("CheckTime fail, nRet = %d\n", nRet);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "CheckTime fail, nRet = %d\n", nRet);
        return GMI_INVALID_PARAMETER;
    }

    SYS_INFO("Time.tm_year = %d\n", ptTime->tm_year);
    SYS_INFO("Time.tm_mon  = %d\n", ptTime->tm_mon);
    SYS_INFO("Time.tm_mday = %d\n", ptTime->tm_mday);
    SYS_INFO("Time.tm_hour = %d\n", ptTime->tm_hour);
    SYS_INFO("Time.tm_min  = %d\n", ptTime->tm_min);
    SYS_INFO("Time.tm_sec  = %d\n", ptTime->tm_sec);

    gettimeofday(&tv, &tz);
    tv.tv_sec  = mktime(ptTime);
    tv.tv_usec = 0;

    settimeofday(&tv, &tz);

    memset(Cmd, 0,  255);
    snprintf(Cmd, 255, "hwclock -w");
    if (system(Cmd) < 0)
    {
        SYS_ERROR("hwclock -w fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "hwclock -w fail\n");
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT BoardManager::SetNtp(boolean_t NtpEnable, int32_t NtpInterval)
{
    if (NtpInterval < 0 || NtpInterval > 100000)
    {
        return GMI_INVALID_PARAMETER;
    }

    m_NtpEnable   = NtpEnable;
    m_NtpInterval = NtpInterval;
    if (0 == m_NtpInterval)
    {
        m_NtpAjustTimeOnce = true;
    }
    else
    {
        m_NtpAjustTimeOnce = false;
    }

    return GMI_SUCCESS;
}


GMI_RESULT BoardManager::SetNtpServer(char_t *NtpAddr)
{
    if (NULL == NtpAddr)
    {
        return GMI_INVALID_PARAMETER;
    }

    GMI_RESULT Result = CheckIPv4String(NtpAddr);
    if (FAILED(Result))
    {
        SYS_ERROR("NtpAddr %s incorrect, Result = 0x%lx\n", NtpAddr, Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "NtpAddr %s incorrect, Result = 0x%lx\n", NtpAddr, Result);
        return Result;
    }

    if (0 != strcmp(NtpAddr, m_NtpServerIp))
    {
        strcpy(m_NtpServerIp, NtpAddr);
        m_NtpServerIpIsChannged = true;
    }
    else
    {
        m_NtpServerIpIsChannged = false;
    }

    return GMI_SUCCESS;
}


GMI_RESULT BoardManager::CheckIPv4String(char_t *Ip)
{
    if (0 == strlen(Ip))
    {
        return GMI_INVALID_PARAMETER;
    }

    uint8_t t = 0;
    uint8_t i = 0;
    char_t  a;
    int32_t a0, a1, a2, a3;
    while ((a = Ip[i++]) != '\0')
    {
        if ((a == ' ')
                || (a == '.')
                || ((a >= '0') && (a <= '9')))
        {
            if (a == '.')
            {
                t++;
            }
        }
        else
        {
            return GMI_INVALID_PARAMETER;
        }
    }

    if (3 != t)
    {
        return GMI_INVALID_PARAMETER;
    }
    else
    {
        sscanf(Ip, "%d.%d.%d.%d", &a3, &a2, &a1, &a0);
        if (255 <= a0
                || 255 <= a1
                || 255 <= a2
                || 255 <= a3)
        {
            return GMI_INVALID_PARAMETER;
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT BoardManager::Initialize()
{
    m_ThreadExitFlag  = false;
    GMI_RESULT Result = m_TimeAdjustNtpThread.Create(NULL, 0, TimeAdjustNtpThread, this);
    if (FAILED(Result))
    {
        SYS_ERROR("m_TimeAdjustNtpThread.Create fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_TimeAdjustNtpThread.Create fail, Result = 0x%lx\n", Result);
        return Result;
    }

    Result = m_TimeAdjustNtpThread.Start();
    if (FAILED(Result))
    {
        m_TimeAdjustNtpThread.Destroy();
        SYS_ERROR("m_TimeAdjustNtpThread.Start fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_TimeAdjustNtpThread.Start fail, Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT BoardManager::Deinitialize()
{
    m_ThreadExitFlag = true;
    while (m_ThreadExitFlag)
    {
        GMI_Sleep(100);
    }

    GMI_RESULT Result = m_TimeAdjustNtpThread.Destroy();
    if (FAILED(Result))
    {
        SYS_ERROR("m_TimeAdjustNtpThread.Destroy() fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "m_TimeAdjustNtpThread.Destroy() fail, Result = 0x%lx\n", Result);
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


void_t* BoardManager::TimeAdjustNtpThread(void *Argument)
{
    BoardManager *Manager = reinterpret_cast<BoardManager*> (Argument);
    return Manager->TimeAdjustNtp();
}


void_t* BoardManager::TimeAdjustNtp(void)
{
    int ntp_fd = -1;
    struct timeval timeout;
    struct sockaddr_in ntp_addr;
    struct sockaddr_in local_addr;
    unsigned int tmpData[12];
    unsigned char curNtpIp[32];
    boolean_t isStartNtp;
    time_t now, curAdjustTime;

    if((ntp_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        goto errorext;
    }

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(ntp_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(ntp_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(GMI_NTP_S_PORT);
    if (bind(ntp_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0)
    {
        goto errorext;
    }

servLoop:
    if (m_ThreadExitFlag)
    {
        goto errorext;
    }
    isStartNtp = true;
    memset(curNtpIp, 0, sizeof(curNtpIp));
    memcpy(curNtpIp, m_NtpServerIp, strlen(m_NtpServerIp));
    if (strlen((const char_t*)curNtpIp) <= 0
            || !m_NtpEnable)
    {
        sleep(30);
        goto servLoop;
    }

    memset(&ntp_addr, 0, sizeof(ntp_addr));
    ntp_addr.sin_family = AF_INET;
    ntp_addr.sin_addr.s_addr = inet_addr((const char_t*)curNtpIp);
    ntp_addr.sin_port = htons(123);

    while (!m_ThreadExitFlag)
    {
        if (!isStartNtp)
        {
            sleep(30);
            goto servLoop;
        }

        if (m_NtpServerIpIsChannged)
        {
            m_NtpServerIpIsChannged = false;
            sleep(10);
            goto servLoop;
        }

        memset(tmpData, 0, sizeof(tmpData));
        PacketNtpTime(tmpData);
        if (sendto(ntp_fd, tmpData, sizeof(tmpData), 0, (struct sockaddr*)&ntp_addr, sizeof(ntp_addr)) != sizeof(tmpData))
        {
            sleep(10);
            continue;
        }
        memset(tmpData, 0, sizeof(tmpData));
        if (recvfrom(ntp_fd, tmpData, sizeof(tmpData), 0, 0, 0) < 0)
        {
            sleep(10);
            continue;
        }
        else
        {
            ParseNtpData(tmpData);
            curAdjustTime = time(NULL);
            SYS_INFO("NTP adjust time OK!\n");

        }

        while(1)
        {
            if (m_NtpAjustTimeOnce)
            {
                if(m_NtpServerIpIsChannged)
                {
                    break;
                }
                sleep(30);
            }
            else
            {
                now = time(NULL);
                if (((now - curAdjustTime) > (m_NtpInterval*60)) || ((now - curAdjustTime) < 0))
                {
                    break;
                }
                else
                {
                    if(m_NtpServerIpIsChannged)
                    {
                        break;
                    }
                    sleep(30);
                }
            }
        }
    }

errorext:
    if (ntp_fd > -1)
    {
        close(ntp_fd);
    }

    m_ThreadExitFlag = false;

    return (void *) GMI_SUCCESS;
}


GMI_RESULT  BoardManager::NtpUpdateTime(struct timeval *ptime)
{
    struct tm nowtime;

    if(NULL == ptime)
    {
        return GMI_INVALID_PARAMETER;
    }

    memcpy(&nowtime, localtime(&(ptime->tv_sec)), sizeof(struct tm));
    GMI_RESULT Result = SetTime(&nowtime);
    if (FAILED(Result))
    {
        SYS_ERROR("SetTime fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetTime fail, Result = 0x%lx\n", Result);
        return Result;
    }

    settimeofday(ptime, NULL);

    return GMI_SUCCESS;
}


GMI_RESULT BoardManager::PacketNtpTime(uint32_t *data)
{
    struct timeval now;

    memset((char*)data, 0, sizeof(data));
    data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24)
                    | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
    data[1] = htonl(1<<16);
    data[2] = htonl(1<<16);
    gettimeofday(&now, NULL);
    data[10] = htonl(now.tv_sec + JAN_1970);
    data[11] = htonl(NTPFRAC(now.tv_usec));

    return GMI_SUCCESS;
}


GMI_RESULT BoardManager::ParseNtpData(uint32_t *data)
{
    NTPTIME orgtime, rectime, xmttime;
    NTPTIME	localtime;
    struct timeval tv, now;
    long long time1, time2, time3, time4;
    long long intertime;
    long long interSec, interUsec;

    gettimeofday(&now, NULL);
    localtime.coarse = now.tv_sec + JAN_1970;
    localtime.fine = NTPFRAC(now.tv_usec);
 
    orgtime.coarse = Data(6);
    orgtime.fine   = Data(7);
    rectime.coarse = Data(8);
    rectime.fine   = Data(9);
    xmttime.coarse = Data(10);
    xmttime.fine   = Data(11);

    time1 = TTLUSEC(MKSEC(orgtime), MKUSEC(orgtime));
    time2 = TTLUSEC(MKSEC(rectime), MKUSEC(rectime));
    time3 = TTLUSEC(MKSEC(xmttime), MKUSEC(xmttime));
    time4 = TTLUSEC(MKSEC(localtime), MKUSEC(localtime));

    intertime = ((time2 - time1) + (time3 - time4))/2; 

    interSec  = GETSEC(intertime);
    interUsec = GETUSEC(intertime);

    tv.tv_sec  = MKSEC(localtime) + interSec;
    tv.tv_usec = MKUSEC(localtime) + interUsec;
    if ((tv.tv_usec) < 0 )
    {
        tv.tv_sec  = tv.tv_sec - 1;
        tv.tv_usec = tv.tv_usec + 1000000;
    }

    GMI_RESULT Result = NtpUpdateTime(&tv);
    if (FAILED(Result))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "adjust time fail, Result = 0x%lx\n", Result);
    }

    return GMI_SUCCESS;
}

