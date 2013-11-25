
#ifndef __USER_OPERATOR_H__
#define __USER_OPERATOR_H__

#include <sqlite3.h>
#include "sys_env_types.h"
#include "ipc_fw_v3.x_setting.h"
#include "gmi_system_headers.h"


class UserOperator
{
public:
    UserOperator();
    ~UserOperator();
    GMI_RESULT Initialize(const char_t *DatabaseName, const char_t *TableName);
    GMI_RESULT Deinitialize();
    GMI_RESULT GetUserNum(uint32_t *UserNumPtr);
    GMI_RESULT GetAllUsers(SysPkgUserInfo *UserInfoPtr, uint32_t UserInfoNum, uint32_t *RealUserNum);
    GMI_RESULT DeleteUser(SysPkgUserInfo *UserInfoPtr);
    GMI_RESULT DeleteAllUsers(void);
    GMI_RESULT SetUser(SysPkgUserInfo *UserInfoPtr);   
    GMI_RESULT GetUser(SysPkgUserInfo *UserInfoPtr);
private:
    sqlite3 *m_Database;
    char_t  **m_SqliteResult;
};

#endif