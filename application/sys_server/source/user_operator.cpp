
#include "user_operator.h"
#include "log.h"


UserOperator::UserOperator()
    :m_Database(NULL)
    ,m_SqliteResult(NULL)
{
}


UserOperator::~UserOperator()
{
}


GMI_RESULT UserOperator::Initialize(const char_t *DatabaseName, const char_t *TableName)
{
    char_t *ErrMsg;
    const char_t *SqlCreatePrefix = "CREATE TABLE IF NOT EXISTS";

    if (NULL == DatabaseName || NULL == TableName)
    {
        SYS_ERROR("DatabaseName or TableName is null\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "DatabaseName or TableName is null\n");
        return GMI_INVALID_PARAMETER;
    }

    if (SQLITE_OK != sqlite3_open(DatabaseName, &m_Database))
    {
        SYS_ERROR("sqlite3_open %s fail\n", DatabaseName);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "sqlite3_open %s fail\n", DatabaseName);
        return GMI_DEVICE_NOT_OPENED;
    }

    ReferrencePtr<char_t, DefaultObjectsDeleter> SqlCreate(BaseMemoryManager::Instance().News<char_t>(strlen(SqlCreatePrefix) + strlen(GMI_USERS_TABLE_NAME) + strlen(GMI_USERS_TABLE_VALUES) + 10));
    if (NULL == SqlCreate.GetPtr())
    {
        SYS_ERROR("SqlCreate new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "SqlCreate new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    sprintf(SqlCreate.GetPtr(), "%s %s%s;", SqlCreatePrefix, GMI_USERS_TABLE_NAME, GMI_USERS_TABLE_VALUES);

    if (SQLITE_OK != sqlite3_exec(m_Database, SqlCreate.GetPtr(), NULL, NULL, &ErrMsg))
    {
        sqlite3_close(m_Database);
        SqlCreate = NULL;
        SYS_ERROR("sqlite3_exec create %s fail, ErrMsg %s\n", GMI_USERS_TABLE_NAME, ErrMsg);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "sqlite3_exec create %s fail, ErrMsg %s\n", GMI_USERS_TABLE_NAME, ErrMsg);
        sqlite3_free(ErrMsg);
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserOperator::Deinitialize()
{
    if (m_Database)
    {
        sqlite3_close(m_Database);
        m_Database = NULL;
    }
    else
    {
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserOperator::GetUserNum(uint32_t * UserNumPtr)
{
    const char_t *Sql = "SELECT * FROM %s";
    char_t *ErrMsg;
    char_t SqlSearch[128];
    int32_t Row;
    int32_t Column;

    if (NULL == UserNumPtr)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "UserNumPtr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    sprintf(SqlSearch, Sql, GMI_USERS_TABLE_NAME);
    if (SQLITE_OK != sqlite3_get_table(m_Database, SqlSearch, &m_SqliteResult, &Row ,&Column , &ErrMsg))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "sqlite3_exec get_table %s fail, ErrMsg %s\n", GMI_USERS_TABLE_NAME, ErrMsg);
        sqlite3_free(ErrMsg);
        return GMI_FAIL;
    }
    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Row = %d, Column = %d\n", Row, Column);

    *UserNumPtr = Row;

    sqlite3_free_table(m_SqliteResult);

    return GMI_SUCCESS;
}


GMI_RESULT UserOperator::GetAllUsers(SysPkgUserInfo * UserInfoPtr, uint32_t UserInfoNum, uint32_t *RealUserNum)
{
    const char_t *Sql = "SELECT * FROM %s";
    char_t *ErrMsg;
    char_t  SqlSearch[128];
    int32_t Row;
    int32_t Column;
    int32_t UserCnt;
    boolean_t ExistRoot = false;

    if (NULL == UserInfoPtr || NULL == RealUserNum)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "UserInfoPtr or RealUserNum is null\n");
        return GMI_INVALID_PARAMETER;
    }

    sprintf(SqlSearch, Sql, GMI_USERS_TABLE_NAME);
    if (SQLITE_OK != sqlite3_get_table(m_Database, SqlSearch, &m_SqliteResult, &Row, &Column , &ErrMsg))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "sqlite3_exec get_table %s fail, ErrMsg %s\n", GMI_USERS_TABLE_NAME, ErrMsg);
        sqlite3_free(ErrMsg);
        return GMI_FAIL;
    }

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Row = %d, Column = %d\n", Row, Column);

    UserCnt = Row > (int32_t)UserInfoNum ? (int32_t)UserInfoNum : Row;

    int32_t i = 0;
    int32_t j = 0;
    for (j = 1; j <= UserCnt; j++)
    {
        if (0 == strcmp(m_SqliteResult[1+j*Column], GMI_SUPER_USER_NAME))
        {
            ExistRoot = true;
        }
        else
        {
            memcpy(UserInfoPtr[i].s_UserName, m_SqliteResult[1+j*Column], sizeof(UserInfoPtr[i].s_UserName));
            memcpy(UserInfoPtr[i].s_UserPass, m_SqliteResult[2+j*Column], sizeof(UserInfoPtr[i].s_UserPass));
            UserInfoPtr[i].s_UserFlag  = atoi(m_SqliteResult[3+j*Column]);
            UserInfoPtr[i].s_UserLevel = atoi(m_SqliteResult[4+j*Column]);
            i++;
        }
    }

    if (ExistRoot)
    {
        *RealUserNum = UserCnt - 1;
    }
    else
    {
        *RealUserNum = UserCnt;
    }

    sqlite3_free_table(m_SqliteResult);

    return GMI_SUCCESS;
}


GMI_RESULT UserOperator::GetUser(SysPkgUserInfo *UserInfoPtr)
{
    char_t  Sql[128];
    char_t *ErrMsg;
    int32_t Row;
    int32_t Column;

    if (NULL == UserInfoPtr)
    {
        SYS_ERROR("UserInfoPtr is null\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "UserInfoPtr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    sprintf(Sql, "SELECT * FROM %s WHERE Name='%s'", GMI_USERS_TABLE_NAME, UserInfoPtr->s_UserName);
    if (SQLITE_OK != sqlite3_get_table(m_Database, Sql, &m_SqliteResult, &Row, &Column , &ErrMsg))
    {
        SYS_ERROR("sqlite3_exec SELECT fail, ErrMsg = %s\n", ErrMsg);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "sqlite3_exec SELECT fail, ErrMsg = %s\n", ErrMsg);
        sqlite3_free(ErrMsg);
        return GMI_FAIL;
    }

    DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "Row = %d, Column = %d\n", Row, Column);

    if (1 < Row)
    {
        SYS_ERROR("Row = %d, error\n", Row);
        sqlite3_free_table(m_SqliteResult);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Row = %d, error\n", Row);
        return GMI_FAIL;
    }

    if (0 == Row)
    {
        SYS_ERROR("Row = %d, no user %s\n", Row, UserInfoPtr->s_UserName);
        sqlite3_free_table(m_SqliteResult);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "Row = %d, no user %s\n", Row, UserInfoPtr->s_UserName);
        return GMI_NOT_SUPPORT;
    }

    int32_t i;
    int32_t j;
    for (i = 0, j = 1; i < Row; i++, j++)
    {
        memcpy(UserInfoPtr[i].s_UserPass, m_SqliteResult[1+j*Column], sizeof(UserInfoPtr[i].s_UserPass));
        UserInfoPtr[i].s_UserFlag  = atoi(m_SqliteResult[2+j*Column]);
        UserInfoPtr[i].s_UserLevel = atoi(m_SqliteResult[3+j*Column]);
    }

    sqlite3_free_table(m_SqliteResult);

    return GMI_SUCCESS;
}


GMI_RESULT UserOperator::SetUser(SysPkgUserInfo * UserInfoPtr)
{
    uint32_t   UserNum;
    GMI_RESULT Result;

    if (NULL == UserInfoPtr)
    {
        SYS_ERROR("UserInfoPtr is null\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "UserInfoPtr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    Result = GetUserNum(&UserNum);
    if (FAILED(Result))
    {
        SYS_ERROR("GetUserNum fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetUserNum fail, Result = 0x%lx\n", Result);
        return Result;
    }

    ReferrencePtr<SysPkgUserInfo, DefaultObjectsDeleter>UserInfo(BaseMemoryManager::Instance().News<SysPkgUserInfo>(UserNum));
    if (NULL == UserInfo.GetPtr())
    {
        SYS_ERROR("UserInfo new fail\n");
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "UserInfo new fail\n");
        return GMI_OUT_OF_MEMORY;
    }

    uint32_t RealUserNum;
    Result = GetAllUsers(UserInfo.GetPtr(), UserNum, &RealUserNum);
    if (FAILED(Result))
    {
        UserInfo = NULL;
        SYS_ERROR("GetAllUsers fail, Result = 0x%lx\n", Result);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "GetAllUsers fail, Result = 0x%lx\n", Result);
        return Result;
    }

    uint32_t i;
    for (i = 0; i < RealUserNum; i++)
    {
        if (0 == strcmp(UserInfoPtr->s_UserName, (UserInfo.GetPtr())[i].s_UserName))
        {
            break;
        }
    }

    if (i >= RealUserNum)
    {
        SYS_INFO("not find '%s' in table %s\n", UserInfoPtr->s_UserName, GMI_USERS_TABLE_NAME);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "not find %s in table %s\n", UserInfoPtr->s_UserName, GMI_USERS_TABLE_NAME);

        if (RealUserNum > MAX_USERS)
        {
            SYS_INFO("user overhead in table, current user num is %d in table\n", RealUserNum);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "user overhead in table, current user num is %d in table\n", RealUserNum);
            return GMI_INVALID_OPERATION;
        }

        char_t  Sql[128];
        char_t *ErrMsg;

        sprintf(Sql, "INSERT INTO \"%s\" VALUES(NULL, '%s', '%s', %d, %d, %d);", \
                GMI_USERS_TABLE_NAME, UserInfoPtr->s_UserName, UserInfoPtr->s_UserPass, UserInfoPtr->s_UserFlag, UserInfoPtr->s_UserLevel, 1);
        if (SQLITE_OK != sqlite3_exec(m_Database, Sql, 0 , 0, &ErrMsg))
        {
            UserInfo = NULL;
            SYS_ERROR("sqlite3_exec INSERT INTO fail, ErrMsg = %s\n", ErrMsg);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "sqlite3_exec INSERT INTO fail, ErrMsg = %s\n", ErrMsg);
            sqlite3_free(ErrMsg);
            return GMI_FAIL;
        }
    }
    else
    {
        SYS_INFO("find '%s' in table %s\n", UserInfoPtr->s_UserName, GMI_USERS_TABLE_NAME);
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Info, "find %s in table %s\n", UserInfoPtr->s_UserName, GMI_USERS_TABLE_NAME);
        char_t  Sql[128];
        char_t *ErrMsg;

        sprintf(Sql, "UPDATE %s SET Password='%s', Role=%d, Popedom=%d, Encrypt=%d WHERE Name='%s';", \
                GMI_USERS_TABLE_NAME, UserInfoPtr->s_UserPass, UserInfoPtr->s_UserFlag, UserInfoPtr->s_UserLevel, 1, UserInfoPtr->s_UserName);
        if (SQLITE_OK != sqlite3_exec(m_Database, Sql, 0 , 0, &ErrMsg))
        {
            UserInfo = NULL;
            SYS_ERROR("sqlite3_exec UPDATE fail, ErrMsg = %s\n", ErrMsg);
            DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "sqlite3_exec UPDATE fail, ErrMsg = %s\n", ErrMsg);
            sqlite3_free(ErrMsg);
            return GMI_FAIL;
        }
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserOperator::DeleteAllUsers(void)
{
    char_t  Sql[128];
    char_t *ErrMsg;

    sprintf(Sql, "DELETE FROM %s WHERE Name<>'%s' and Name<>'%s' and Name<>'%s';", GMI_USERS_TABLE_NAME, GMI_SUPER_USER_NAME, GMI_ADMIN_USER_NAME1, GMI_ADMIN_USER_NAME2);

    if (SQLITE_OK != sqlite3_exec(m_Database, Sql, 0 , 0, &ErrMsg))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "sqlite3_exec delete fail, ErrMsg = %s\n", ErrMsg);
        sqlite3_free(ErrMsg);
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


GMI_RESULT UserOperator::DeleteUser(SysPkgUserInfo * UserInfoPtr)
{
    if (NULL == UserInfoPtr)
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "UserInfoPtr is null\n");
        return GMI_INVALID_PARAMETER;
    }

    if (0 == strcmp(UserInfoPtr->s_UserName, GMI_SUPER_USER_NAME)
            || 0 == strcmp(UserInfoPtr->s_UserName, GMI_ADMIN_USER_NAME2)
            || 0 == strcmp(UserInfoPtr->s_UserName, GMI_ADMIN_USER_NAME1))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "username = %s, be refused to delete\n", UserInfoPtr->s_UserName);
        return GMI_FAIL;
    }

    char_t  Sql[128];
    char_t *ErrMsg;

    sprintf(Sql, "DELETE FROM %s WHERE Name='%s';", GMI_USERS_TABLE_NAME, UserInfoPtr->s_UserName);
    if (SQLITE_OK != sqlite3_exec(m_Database, Sql, 0 , 0, &ErrMsg))
    {
        DEBUG_LOG(g_DefaultLogClient, e_DebugLogLevel_Exception, "sqlite3_exec delete username %s fail, ErrMsg = %s\n", UserInfoPtr->s_UserName, ErrMsg);
        sqlite3_free(ErrMsg);
        return GMI_FAIL;
    }

    return GMI_SUCCESS;
}


