#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include<ctype.h>


#include "gmi_debug.h"


static FILE  *l_AppFLogFile1 = NULL;
static int32_t    l_iLogCount =0;

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
    struct tm	 *AppTime;
    time_t 		AppNow;

    time(&AppNow);
    AppTime = localtime(&AppNow);

    printf("%04d-%02d-%02d %02d:%02d:%02d ",
           AppTime->tm_year+1900, AppTime->tm_mon+1, AppTime->tm_mday, AppTime->tm_hour,
           AppTime->tm_min, AppTime->tm_sec);

    fprintf(l_AppFLogFile1, "%04d: %04d-%02d-%02d %02d:%02d:%02d  ",
            l_iLogCount,
            AppTime->tm_year+1900, AppTime->tm_mon+1, AppTime->tm_mday, AppTime->tm_hour,
            AppTime->tm_min, AppTime->tm_sec);

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


