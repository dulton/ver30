#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilitly.h"
#include "log.h"
#include "cgi_cmd_entry.h"
#include "gmi_system_headers.h"
#include "sys_client.h"
#include "ipc_fw_v3.x_resource.h"


int32_t main(int32_t argc, char_t *argv[])
{
    char_t *Method     = getenv("REQUEST_METHOD");
    char_t *ContentLen = getenv("CONTENT_LENGTH");
    char_t *ContentType= getenv("CONTENT_TYPE");
    char_t *QueryString= getenv("QUERY_STRING");
    char_t *Query  = NULL;
    char_t *FncCmd = NULL;
    GMI_RESULT Result;

    LogInitial();

    do
    {
        if (NULL == Method)
        {
            CGI_ERROR("Method null!!!\n");
            break;
        }

        if (0 == strcasecmp(Method, "GET"))
        {
            Query = (QueryString) ? strdup(QueryString) : NULL;
        }
        else if (0 == strcasecmp(Method, "POST"))
        {
            if (0 == (strncasecmp(ContentType, "multipart/form-data", strlen("multipart/form-data"))))
            {
                Query = (QueryString) ? strdup(QueryString) : NULL;
            }
            else
            {
                int32_t ConLen = 0;

                if (NULL == ContentLen)
                {
                    CGI_ERROR("ContentLen null!!!\n");
                    break;
                }

                ConLen = atoi(ContentLen);
                if (0 == ConLen)
                {
                    CGI_ERROR("ConLen is 0!!!\n");
                    break;
                }

                Query = (char_t*)malloc((ConLen+1) * sizeof(char_t));
                if (NULL == Query)
                {
                    CGI_ERROR("malloc query error!!!\n");
                    break;
                }

                memset(Query, 0, ConLen+1);

                int32_t Size = fread(Query, 1, ConLen, stdin);
                if (Size != ConLen)  
                {
                    CGI_ERROR("ConLen %d, Size %d, not equal!!!\n", ConLen, Size);
                    break;
                }
            }
        }
        else
        {
            CGI_ERROR("Method %s not matching excuting function!!!\n", Method);
            break;
        }

        fprintf(stdout,"Content-type:text/html\n\n");
        if (NULL != Query)
        {
            CGI_INFO("Query: %s\n", Query);

            Result = InitCgi(Query);
            if (FAILED(Result))
            {
                CGI_ERROR("InitCgi fail\n");
                break;
            }

            FncCmd = WEB_GET_VAR("cgiFncCmd");
            if (NULL == FncCmd)
            {
                CGI_ERROR("WEB_GET_VAR fail\n");
                break;
            }

            Result =  SysInitialize(GMI_CGI_AUTH_PORT);
	   // Result = SysInitialize(GMI_CGI_C_PORT_START, GMI_CGI_C_PORT_END,GMI_CGI_S_PORT_START, GMI_CGI_S_PORT_END,GMI_CONTROL_S_PORT, GMI_CGI_AUTH_PORT);
            if (FAILED(Result))
            {
                InitCgi(NULL);
                break;
            }

            struct timeval ComTimeout;
            ComTimeout.tv_sec  = 60;
            ComTimeout.tv_usec = 0;
            Result = SysInitializeExt(&ComTimeout, 1);	 
            if (FAILED(Result))
            {
            	InitCgi(NULL);
            	break;
            }

            Result = CgiCmdProcess(FncCmd);
            if (FAILED(Result))
            {
                InitCgi(NULL);
//                CGI_ERROR("CgiCmdProcess  Error Result = 0x%0x\n", Result);
                break;
            }

            InitCgi(NULL);
        }

        if (NULL != Query)
        {
            free(Query);
            Query = NULL;
        }

        LogUninitial();
        SysDeinitialize();

        return 0;
    } while(0);

    if (NULL != Query)
    {
        free(Query);
        Query = NULL;
    }

    LogUninitial();
    SysDeinitialize();

    return -1;
}
