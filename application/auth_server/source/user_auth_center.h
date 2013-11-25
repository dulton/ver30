#ifndef __USER_AUTH_CENTER_H__
#define __USER_AUTH_CENTER_H__

#include "gmi_system_headers.h"
#include <pthread.h>
#include <sqlite3.h>


//#define PRT_TEST(x) printf x
#define PRT_TEST(x) 

//need read database
#define TYPE_DATABASE_MANAGE_USEINFO

#define USER_INFO_FILE_PATH           "/opt/config/password.txt"

#define GMI_USERS_FILE_NAME           "/opt/config/gmi_users.db"
#define GMI_USERS_TABLE_NAME          "Users"
#define GMI_USERS_TABLE_FIELD_NAME   "Name"
//#define GMI_USERS_TABLE_VALUES   "(ID_INTEGER PRIMARY KEY AUTOINCREMENT, Name TEXT, Password TEXT, Role INTERGER, Popedom INTERGER, Encrypt INTERGER)"

#define GMI_TMP_SUCCESS             0                   //success
#define GMI_TMP_FAIL                1                   //normal error
#define GMI_FAIL_SINGLELINK        2                   //overflow of linking numbers of single user
#define GMI_FAIL_ALLLINK           3                   //overflow of linking numbers of all user
#define GMI_FAIL_OPENFILE          4                   //open file error
#define GMI_FAIL_NOUSER            5                   //username inexistence

#define GMI_RECORD_DEL             1                   //record deletion 
#define GMI_RECORD_ADD             2	                //record adding


#define MAX_USER_LINK_NUM         64                  //max linking number
#define MAX_USER_ACCOUNT_NUM      64                  //max number of user

#define MAX_LEN_USERNAME          128                 //max length of username
#define MAX_LEN_PASSWORD          128                 //max length of password

#define MAX_NUM_SESSIONID         65000               //threshold value of sessionId


//user information
typedef struct tagAccountInfo 
{
    char_t   s_Username[MAX_LEN_USERNAME];            //user name
    char_t   s_Password[MAX_LEN_PASSWORD];            //password
    uint32_t s_AuthValue;                             //value of authority
}UserAccountInfo;

//relation of user--sessionId
typedef struct tagUserSessionId
{
    char_t     s_Username[MAX_LEN_USERNAME];         //username
    uint16_t   s_SessionId;                          //sessionId of login
    uint8_t   s_MoudleId;                          //moudle info:1-SDK server,2-onvif, 3-GB, 4-web
    uint8_t   s_Reserve;                           
}UserSessionId; 


//relation of user--link
typedef struct tagUserLinkNum
{
    char_t     s_Username[MAX_LEN_USERNAME];        //username
    uint16_t   s_LinkNum;                           //linking number of the certain user
    uint16_t   s_Reserve;                            
}UserLinkNum;

class UserAuthentication
{
    public:
		UserAuthentication(void);
		virtual ~UserAuthentication(void);

		GMI_RESULT Initialize(pthread_rwlock_t * PLock);
		GMI_RESULT Deinitialize(void);

		GMI_RESULT SetUserSessionIdRecord(const uint16_t InSessionId, char_t *InOutUserName, const uint32_t MoudleId, const uint32_t ModifyFlag);                             //set table of relation between users and sessionId, 1-delete record,2-add record
		GMI_RESULT SetUserLinkNumRecord(const char_t *InUserName, const uint32_t ModifyFlag);                                                         //set table of relation between users and linking numbers, 1-delete record, 2-add record
		GMI_RESULT GetUserSessionIdRecord(UserSessionId **OutRecord); 
		GMI_RESULT GetUserLinkNumRecord(UserLinkNum *OutRecord);
		GMI_RESULT CheckUserNameValid(const char_t *InUserName, const uint16_t InUserNameEncType, char_t *OutPassword, uint32_t *OutAuthValue);   //check valid username, return password and authority when username is valid
		GMI_RESULT CheckUserLinkNumValid(const char_t *InUserName, const uint16_t InSingleUserMaxLinkNum, const uint16_t InAllUserMaxLinkNum);   //check linking numbers of the certain user and check linking total numbers of all users
		GMI_RESULT CheckSessionIdValid(const uint16_t InSessionId, const char_t *InUserName);                                                          //check valid of sessionId of reconnection
		GMI_RESULT CreateSessionId(uint16_t *OutSessionId);                                                                                              //create sessionId 
		GMI_RESULT ClearRecordInfo(uint32_t MoudleId);                                                                                                    //clear record of user-links and user-sessionId, InRecordId:0-all, 1-user-links, 2-user-sessionId
		
	private:
		GMI_RESULT GetUserInfoFormFile(uint32_t *OutUserNum, UserAccountInfo *OutUserInfo);                                                            //get user infomation
		GMI_RESULT CheckSessionIdUnique(const uint16_t InSessionId);                                                                                   //check uniqueness of created sessionId

	private:
		static UserSessionId m_UserSessionId[MAX_USER_LINK_NUM];            //Table of user-sessionId
		static UserLinkNum m_UserLinkNum[MAX_USER_ACCOUNT_NUM];             //Table of user-linking numbers
		static uint32_t m_AllUserLinkTotalNum;                             //total linking numbers
		static uint32_t m_AllLoginUserTotalNum;                            //total numbers of linking user 
		static uint16_t m_SessionIdSeed;                                    //seed of sessionId
  		pthread_rwlock_t *m_RdWrLock;										  //lock of read-write
};


#define ERR_PRINT(x) do{ \
	                       fprintf(stderr, "[AUTH_CENTER] %s:%d ", __FUNCTION__, __LINE__); \
	                       fprintf(stderr, x); \
                         }while(0)

#endif
