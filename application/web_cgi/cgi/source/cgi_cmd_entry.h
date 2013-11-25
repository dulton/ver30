#ifndef __CGI_CMD_ENTRY_H__
#define __CGI_CMD_ENTRY_H__

#include "gmi_system_headers.h"

#define CONTENT_TYPE_JSON "json"
#define CONTENT_TYPE_XML  "xml"
#define CMD_KEY           "cgiFncCmd"
#define CONTENT_TYPE_KEY  "cgiContentType"
#define CONTENT_KEY       "Content"
#define SESSION_ID_KEY    "SessionId"
#define RETCODE_KEY       "RetCode"
#define CMD_STRING        "cgiFncCmd=%s&cgiContentType=%s&Content"

GMI_RESULT CgiCmdProcess(const char_t *FncCmd);

#endif
