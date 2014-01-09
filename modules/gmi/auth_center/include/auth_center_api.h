
#ifndef __AUTH_CENTER_API_H__
#define __AUTH_CENTER_API_H__

#include "gmi_system_headers.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_LEN_USERNAME            128    //max length of username
#define MAX_LEN_PASSWORD            128    //max length of password

//#define LOCAL_AUTH_PORT             51243   //port of authority in RUDP
#define LOCAL_AUTH_PORT             GMI_AUTH_SERVER_PORT

//#define KEY_SHM_NUM                  6451   //key of creating share memory


#define TYPE_AUTH_LOGIN             1      //type of authentication:login
#define TYPE_AUTH_LOGOUT            2      //type of authentication:logout
#define TYPE_AUTH_RESET             3      //type of authentication:clear record
#define TYPE_AUTH_GETSESSIONID      4      //type of authentication:get sessionIds


#define TYPE_ENCRYPTION_TEXT        0     //type of encryption:text
#define TYPE_ENCRYPTION_MD5         1     //type of encryption:MD5
#define TYPE_ENCRYPTION_MSCHAP      2     //type of encryption:MD5(MD5(password)+random number)
#define TYPE_ENCRYPTION_DES         3     //type of encryption:DES
#define TYPE_ENCRYPTION_MD5_RTSP    4     //type of encryption:md5(md5(<username>:<realm>:<password>):<nonce>:md5(<cmd>:<url>))


#define GMI_CODE_SUCCESS             0    //success
#define GMI_CODE_ERR_USER            1    //username no exist 
#define GMI_CODE_ERR_PASSWORD        2    //password error
#define GMI_CODE_ERR_SINGLELINK      3    //overflow of  linking numbers of single user
#define GMI_CODE_ERR_ALLLINK         4    //overflow of total linking numbers of all users
#define GMI_CODE_ERR_SESSIONID       5    //sessrion error
#define GMI_CODE_ERR_PARAM           6    //input param error
#define GMI_CODE_ERR_INTER           7   //internal error

#define ID_MOUDLE_REST_SDK          0x1   //sdk server reset linknum and sessionId
#define ID_MOUDLE_REST_ONVIF        0x2  //onvif server
#define ID_MOUDLE_REST_GB           0x3   //GB
#define ID_MOUDLE_REST_WEB          0x4   //WEB
#define ID_MOUDLE_REST_CONFIG       0x5   //CONFIG TOOL
#define ID_MOUDLE_REST_RTSP         0x6   //rtsp server
#define ID_MOUDLE_REST_ALL          0xF   //all login info of linknums and sessionId

#define ID_SESSIONID_INTER_BASE          65100
#define ID_SESSIONID_INTER_SDK           (ID_SESSIONID_INTER_BASE+1)
#define ID_SESSIONID_INTER_CONFIGROOT   (ID_SESSIONID_INTER_BASE+10)
#define ID_SESSIONID_INTER_CONFIGADMIN  (ID_SESSIONID_INTER_BASE+11)

//authentication information of user 
typedef struct tagAuthRefInfo
{
    uint32_t   s_DataType;                         //data type: 1-login, 2-logout, 3-clear record, 4-get sessionIds
	char_t     s_Username[MAX_LEN_USERNAME];       //username
    char_t     s_Password[MAX_LEN_PASSWORD];       //password
    uint8_t    s_UsernameEncType;                 //type 0f enctyption : 0-text£¬1-MD5£¬...
    uint8_t    s_PasswordEncType;                 //type 0f enctyption : 0-text£¬1-MD5£¬2-MSCHAP,  3-DES, 4-MD5_RTSP...
    uint16_t   s_SessionId;                       //new login:0, sessionId of SDK in the reconnection, logout:using sessionId
    uint16_t   s_SingleUserMaxLinkNum;           //max linking numbers of single user
    uint16_t   s_AllUserMaxLinkNum;               //max linking numbers of all users
    uint32_t   s_UserAuthExtDataLen;              //length of extended infomation of authority
    char_t    *s_UserAuthExtData;                 //extended infomation of authority
    uint8_t    s_MoudleId;                         //moudle info: 1-SDK, 2-ONVIF, 3-GB, 4-WEB, 5-CONFIGTOOL, 6-RTSP
    uint8_t    s_Reserved[15];
}UserAuthRefInfo;

//result of authentication
typedef struct tagAuthResInfo 
{
    uint32_t   s_DataType;                        //data type: 1-login, 2-logout, 3-clear record, 4-get sessionIds
    uint16_t   s_AuthResult;                      //result of authority, 0-success£¬!0-failed
    uint16_t   s_SessionId;                       //sessionId£¬range of 1-65535
    uint32_t   s_AuthValue;                       //value of authority
}UserAuthResInfo;

//result of query all linked sessioId
typedef struct tagAuthLinkedSessionId
{
	uint32_t   s_LinkedNum;                      //number of sessionId in using 
	uint32_t   *s_LinkedSessionId;              //record of sessionId  in using
}UserAuthLinkedSessionId;


//information of rtsp of authentication
typedef struct tagAuthExtInfo
{
	char_t   s_Realm[128];
	char_t   s_Nonce[128];
	char_t   s_Cmd[128];
	char_t   s_Url[256];	
}UserAuthExtInfo;


/*===============================================================
func name:GMI_UserAuthCheck
func:user identification of login and get authority
input:UserAuthInputData--authentication information of user ;
        UserAuthOutputData -- result of authentication;
        InPortNum--input port ,if the value is invalid, use default value
return:success--return GMI_SUCCESS, 
	failed -- return GMI_FAIL
---------------------------------------------------------------------*/

GMI_RESULT GMI_UserAuthCheck(UserAuthRefInfo *UserAuthInputData, UserAuthResInfo *UserAuthOutputData, const uint16_t InPortNum);

/*===============================================================
func name:GMI_UserLogoutNotify
func:linking sessionId  is notified when user has executed logout
input:sessionId--linking user sessionId ;
         InPortNum--input port ,if the value is invalid, use default value
return:success--return GMI_SUCCESS, 
	failed -- return GMI_FAIL
---------------------------------------------------------------------*/

GMI_RESULT GMI_UserLogoutNotify(const uint16_t  sessionId, const uint16_t InPortNum);


/*===============================================================
func name:GMI_UserLogInDataReset
func:clear record of login
input:InPortNum--input port ,if the value is invalid, use default value
return:success--return GMI_SUCCESS, 
	failed -- return GMI_FAIL
---------------------------------------------------------------------*/

GMI_RESULT GMI_UserLogInDataReset(const uint32_t MoudleId, const uint16_t InPortNum);

/*===============================================================
func name:GMI_UserQueryLogInSessionId
func:get sessionId of Login
input:InPortNum--input port ,if the value is invalid, use default value
         SessionIdOutData--result of SessionId in using
return:success--return GMI_SUCCESS, 
	failed -- return GMI_FAIL
---------------------------------------------------------------------*/

GMI_RESULT GMI_UserQueryLogInSessionId(const uint16_t InPortNum, UserAuthLinkedSessionId *SessionIdOutData);


#ifdef __cplusplus
}
#endif

#endif
