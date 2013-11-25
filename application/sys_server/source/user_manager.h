#ifndef __USER_MANAGER_H__
#define __USER_MANAGER_H__

#include "user_operator.h"
#include "gmi_system_headers.h"


class UserManager : public UserOperator
{
public:
	UserManager();
    ~UserManager();
    GMI_RESULT Initialize(const char_t *DatabaseName, const char_t *TableName);
    GMI_RESULT Deinitialize();
    GMI_RESULT AddDefaultAdmin(void);
    GMI_RESULT FactoryDefault(void);
private:
	GMI_RESULT AddDefaultAdminstrator(const char_t *UserName, const char_t* Password);
private:	
	sqlite3 *m_Database;
	char_t  **m_SqliteResult;
};

#endif

