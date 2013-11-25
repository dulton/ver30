/******************************************************************************
modules		:    Daemon
name		:    gmi_debug.c
function	:    daemon server debug function, for example, debug message
author		:    minchao.wang
version		:    1.0.0.0.0
---------------------------------------------------------------
modification	:
	date		version		 author 	modification
	1/28/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
#include "gmi_includes.h"
#include "gmi_debug.h"
#include "gmi_system.h"
#include "gmi_struct.h"
#include "gmi_config.h"
#include "gmi_daemon_thread.h"
#include "gmi_update.h"
#include "gmi_system_headers.h"

static FILE  *l_AppFLogFile1;
static int32_t    l_iLogCount;

/*==============================================================
name				:	GMI_DeBugPrint
function			:  Write debug log to file
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     format.
return				:    no
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   2/5/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
void GMI_DeBugPrint(const char_t *format, ...)
{
    va_list   	arg;
    const char_t *p, *sval;
    char_t localfmt[LOCALFMT] = {0};
    int32_t i,ival;
    uint32_t uval;
    double 		dval;
    struct tm	 *pAppTime;
    time_t 		nAppNow;

    time(&nAppNow);
    pAppTime = localtime(&nAppNow);

    printf("%04d-%02d-%02d %02d:%02d:%02d ",
           pAppTime->tm_year+1900, pAppTime->tm_mon+1, pAppTime->tm_mday, pAppTime->tm_hour,
           pAppTime->tm_min, pAppTime->tm_sec);

    fprintf(l_AppFLogFile1, "%04d: %04d-%02d-%02d %02d:%02d:%02d  ",
            l_iLogCount,
            pAppTime->tm_year+1900, pAppTime->tm_mon+1, pAppTime->tm_mday, pAppTime->tm_hour,
            pAppTime->tm_min, pAppTime->tm_sec);

    va_start(arg, format);  /*make arg point to 1st unnamed arg*/
    for (p = format; *p; p++)
    {
        if (*p != '%')
        {
            putchar(*p);
            fprintf(l_AppFLogFile1, "%c", *p);
            continue;
        }

        i = 0;
        localfmt[i++] = '%';
        while (*(p+1) && !isalpha(*(p+1)))
            localfmt[i++] = *++p;
        localfmt[i++] = *(p + 1);
        localfmt[i] = '\0';
        switch (*++p)
        {
        case 'd':
        case 'i':
            ival = va_arg(arg, int);
            printf(localfmt,ival);
            fprintf(l_AppFLogFile1, localfmt, ival);
            break;
        case 'x':
        case 'X':
        case 'u':
        case 'o':
            uval = va_arg(arg, unsigned);
            printf(localfmt,uval);
            fprintf(l_AppFLogFile1, localfmt, uval);
            break;
        case 'f':
            dval = va_arg(arg, double);
            printf(localfmt, dval);
            fprintf(l_AppFLogFile1, localfmt, dval);
            break;
        case 's':
            sval = va_arg(arg, char_t *);
            printf(localfmt, sval);
            fprintf(l_AppFLogFile1, localfmt, sval);
            break;
        default:
            printf("%s",localfmt);
            fprintf(l_AppFLogFile1, "%s", localfmt);
            break;
        }
    }
    va_end(arg);

    printf("\n");
    fprintf(l_AppFLogFile1, "%s", "\n");
    fflush(l_AppFLogFile1);

    l_iLogCount++;
    if (l_iLogCount > MAX_LOG_LINE)
    {
        l_iLogCount = 0;
        fseek(l_AppFLogFile1, 0, SEEK_SET);
    }
}

/*==============================================================
name				:	GMI_DebugLog2FileInitial
function			:  Debug log file to file
algorithm implementation	:	no
global variable			:	no
parameter declaration		:     no
return				:    no
---------------------------------------------------------------
modification	:
   date		version		 author 	modification
   2/5/2013	1.0.0.0.0     minchao.wang         establish
******************************************************************************/
void GMI_DebugLog2FileInitial()
{
    char_t CmdBuffer[MAX_BUFFER_LENGTH];

    memset(CmdBuffer, 0, sizeof(CmdBuffer));
    snprintf(CmdBuffer, 255, "cp %s %s", LOG_FILE1, LOG_FILE1_OLD);
    if (system(CmdBuffer) < 0)
    {
        GMI_DeBugPrint("System cp exe error!\n");
    }

    l_AppFLogFile1 = fopen(LOG_FILE1, "w");

    if (l_AppFLogFile1 == NULL)
    {
        GMI_DeBugPrint("open logfile1 failed\n");
    }

    return ;
}

