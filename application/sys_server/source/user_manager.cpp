#include "user_manager.h"
#include "log.h"

UserManager::UserManager()
    :UserOperator()
{
}


UserManager::~UserManager()
{
}


GMI_RESULT UserManager::Initialize(const char_t *DatabaseName, const char_t *TableName)
{
    GMI_RESULT Result = UserOperator::Initialize(DatabaseName, TableName);
    if (FAILED(Result))
    {
        SYS_ERROR("Initialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Initialize fail, Result = 0x%lx\n", Result);
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserManager::Deinitialize()
{
    GMI_RESULT Result = UserOperator::Deinitialize();
    if (FAILED(Result))
    {
        SYS_ERROR("Deinitialize fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Deinitialize fail, Result = 0x%lx\n", Result);
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserManager::AddDefaultAdminstrator(const char_t *UserName, const char_t* Password)
{
    SysPkgUserInfo UserInfo;

    memset(&UserInfo, 0, sizeof(SysPkgUserInfo));
    memcpy(UserInfo.s_UserName, UserName, sizeof(UserInfo.s_UserName));
    GMI_RESULT Result = UserOperator::GetUser(&UserInfo);
    if (GMI_NOT_SUPPORT == Result)
    {
        SYS_INFO("database default is null, no user '%s'\n", UserName);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "database default is null, no user '%s'\n", UserName);
        memcpy(UserInfo.s_UserPass, Password, sizeof(UserInfo.s_UserPass));
        UserInfo.s_UserFlag  = 1;
        UserInfo.s_UserLevel = GMI_ADMIN_AUTH_VALUE;
        SYS_INFO("%s %d 0x%x\n", UserInfo.s_UserName, UserInfo.s_UserFlag, UserInfo.s_UserLevel);
        Result = UserOperator::SetUser(&UserInfo);
        if (FAILED(Result))
        {
            SYS_ERROR("SetUser fail, Result = 0x%lx\n", Result);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetUser fail, Result = 0x%lx\n", Result);

            char_t Cmd[128];
            sprintf(Cmd, "rm -f %s", GMI_USERS_FILE_NAME);
            system(Cmd);

            return GMI_NOT_SUPPORT;
        }
    }
    else if (FAILED(Result))
    {
        SYS_ERROR("GetUser '%s' fail, Result = 0x%lx\n", UserName, Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetUser '%s' fail, Result = 0x%lx\n", UserName, Result);


        char_t Cmd[128];
        sprintf(Cmd, "rm -f %s", GMI_USERS_FILE_NAME);
        system(Cmd);

        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserManager::AddDefaultAdmin(void)
{
    GMI_RESULT Result;

    //if user database is null, add default adminstrator
    //add "root"
    Result = AddDefaultAdminstrator(GMI_SUPER_USER_NAME, GMI_SUPER_USER_PASSWD);
    if (FAILED(Result))
    {
        SYS_ERROR("AddDefaultAdminstrator %s fail, Result = 0x%lx\n", GMI_SUPER_USER_NAME, Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "AddDefaultAdminstrator %s fail, Result = 0x%lx\n", GMI_SUPER_USER_NAME, Result);
        return Result;
    }

    //add "admin"
    Result = AddDefaultAdminstrator(GMI_ADMIN_USER_NAME1, GMI_ADMIN_USER_NAME1_PASSWD);
    if (FAILED(Result))
    {
        SYS_ERROR("AddDefaultAdminstrator %s fail, Result = 0x%lx\n", GMI_ADMIN_USER_NAME1, Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "AddDefaultAdminstrator %s fail, Result = 0x%lx\n", GMI_ADMIN_USER_NAME1, Result);
        return Result;
    }

    //add "inc"
    Result = AddDefaultAdminstrator(GMI_ADMIN_USER_NAME2, GMI_ADMIN_USER_NAME2_PASSWD);
    if (FAILED(Result))
    {
        SYS_ERROR("AddDefaultAdminstrator %s fail, Result = 0x%lx\n", GMI_ADMIN_USER_NAME2, Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Warning, "AddDefaultAdminstrator %s fail, Result = 0x%lx\n", GMI_ADMIN_USER_NAME2, Result);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserManager::FactoryDefault()
{
    GMI_RESULT Result = UserOperator::DeleteAllUsers();
    if (FAILED(Result))
    {
        SYS_ERROR("DeleteAllUsers fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DeleteAllUsers fail, Result = 0x%lx\n", Result);
        return Result;
    }

    SysPkgUserInfo SysUserInfo;
    memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));
    memcpy(SysUserInfo.s_UserName, GMI_ADMIN_USER_NAME1, sizeof(SysUserInfo.s_UserName));
    memcpy(SysUserInfo.s_UserPass, GMI_ADMIN_USER_NAME1_PASSWD, sizeof(SysUserInfo.s_UserPass));
    SysUserInfo.s_UserFlag = 1;
    Result = UserOperator::SetUser(&SysUserInfo);
    if (FAILED(Result))
    {
        SYS_ERROR("SetUser '%s' fail, Result = 0x%lx\n", GMI_ADMIN_USER_NAME1, Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetUser fail, Result = 0x%lx\n", Result);
        return Result;
    }

    memset(&SysUserInfo, 0, sizeof(SysPkgUserInfo));
    memcpy(SysUserInfo.s_UserName, GMI_ADMIN_USER_NAME2, sizeof(SysUserInfo.s_UserName));
    memcpy(SysUserInfo.s_UserPass, GMI_ADMIN_USER_NAME2_PASSWD, sizeof(SysUserInfo.s_UserPass));
    SysUserInfo.s_UserFlag = 1;
    Result = UserOperator::SetUser(&SysUserInfo);
    if (FAILED(Result))
    {
        SYS_ERROR("SetUser '%s' fail, Result = 0x%lx\n", GMI_ADMIN_USER_NAME2, Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SetUser fail, Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}

