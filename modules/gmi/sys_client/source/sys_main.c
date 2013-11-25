#include "log.h"
#include "sys_utilitly_unix.h"
#include "sys_command_excute.h"
#include "gmi_system_headers.h"
#include "auth_center_api.h"
#include "des.h"

#define SINGLEUSERMAXLINK  20
#define ALLUSERMAXLINK     20
static uint16_t  l_LocalAuthRudpPort;
static boolean_t l_InitDone = false;

GMI_RESULT SysInitialize(uint16_t LocalAuthRudpPort)
{
    GMI_RESULT Result = GMI_SUCCESS;

    SysClientLogInitial();

    Result = SysCmdInit();
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("SysCmdInit fail,Result = 0x%lx\n", Result);
        return Result;
    }

    l_LocalAuthRudpPort = LocalAuthRudpPort;
    l_InitDone = true;

    return GMI_SUCCESS;
}


GMI_RESULT SysInitializeExt(struct timeval *TimeoutPtr, uint16_t TryCount)
{
    if (!l_InitDone)
    {
        SYS_CLIENT_ERROR("SysInitialize not done\n");
        return GMI_FAIL;
    }

    GMI_RESULT Result = SysCmdInitExt(TimeoutPtr, TryCount);
    if (FAILED(Result))
    {
        SYS_CLIENT_ERROR("SysCmdInitExt fail,Result = 0x%lx\n", Result);
        return Result;
    }

    return GMI_SUCCESS;
}


GMI_RESULT SysDeinitialize()
{
    SysCmdUnInit();

    SysClientLogUninitial();

    return GMI_SUCCESS;
}


GMI_RESULT SysAuthLogin(char_t  UserName[32],
                        char_t    UserPasswd[32],
                        uint16_t  InSessionId,
                        uint8_t   ModuleId,
                        uint16_t *OutSessionIdPtr,
                        uint8_t  *UserFlagPtr,
                        uint32_t *AuthvaluePtr
                       )
{
    char_t RandomNumber[8];
    UserAuthRefInfo InputData;
    UserAuthResInfo OutputData;

    if (NULL == UserName
            || NULL == UserPasswd
            || NULL == OutSessionIdPtr
            || NULL == UserFlagPtr
            || NULL == AuthvaluePtr
       )
    {
        return GMI_INVALID_PARAMETER;
    }

    srand(time((time_t *)NULL));
    uint16_t DesKey = (rand()%1000 + 1000);

    memset(&InputData, 0, sizeof(UserAuthRefInfo));
    memcpy(InputData.s_Username, UserName, 32);
    InputData.s_DataType             = TYPE_AUTH_LOGIN;
    InputData.s_UsernameEncType      = TYPE_ENCRYPTION_TEXT;
    InputData.s_PasswordEncType      = TYPE_ENCRYPTION_DES;
    InputData.s_SessionId            = InSessionId;
    InputData.s_SingleUserMaxLinkNum = SINGLEUSERMAXLINK;
    InputData.s_AllUserMaxLinkNum    = ALLUSERMAXLINK;
    InputData.s_UserAuthExtDataLen   = 8;
    InputData.s_MoudleId             = ModuleId;
    memset(RandomNumber, 0, sizeof(RandomNumber));
    sprintf(RandomNumber, "%d", DesKey);
    InputData.s_UserAuthExtData      = RandomNumber;

    //encrypt
    char_t Passwd[LEN_DES_PASSWORD];
    int32_t CiperLen;

    memset(Passwd, 0, sizeof(Passwd));
    strcpy(Passwd, UserPasswd);
    if (0 > DES_Encrypt(Passwd, sizeof(Passwd), RandomNumber, InputData.s_Password, &CiperLen))
    {
        return GMI_FAIL;
    }

    memset(&OutputData, 0, sizeof(UserAuthResInfo));
    GMI_RESULT Result = GMI_UserAuthCheck(&InputData, &OutputData, l_LocalAuthRudpPort);
    if (FAILED(Result))
    {
        return Result;
    }

    *OutSessionIdPtr = OutputData.s_SessionId;
    *UserFlagPtr     = (uint8_t)((OutputData.s_AuthValue >> 24) & 0xff);
    *AuthvaluePtr    = OutputData.s_AuthValue;
    //*TimeoutMinPtr   = 30;

    return GMI_SUCCESS;
}


GMI_RESULT SysAuthLogout(uint16_t SessionId)
{
    //guoqiang, mask,9/4/2013, the reason is web just check auth temporary .
    //GMI_RESULT Result = GMI_UserLogoutNotify(SessionId, l_LocalAuthRudpPort);
    //if (FAILED(Result))
    //{
    //    return Result;
    //}

    return GMI_SUCCESS;
}


GMI_RESULT SysCheckSessionId(uint16_t SessionId, boolean_t *Valid)
{
    UserAuthLinkedSessionId LinkSessions;
    uint32_t SessionIdContainer[ALLUSERMAXLINK];

    if (0 == l_LocalAuthRudpPort
            || NULL == Valid)
    {
        return GMI_INVALID_PARAMETER;
    }

    memset(&LinkSessions, 0, sizeof(UserAuthLinkedSessionId));
    LinkSessions.s_LinkedSessionId = SessionIdContainer;

    GMI_RESULT Result = GMI_UserQueryLogInSessionId(l_LocalAuthRudpPort, &LinkSessions);
    if (FAILED(Result))
    {
        return Result;
    }

    uint32_t i;
    for (i = 0; i < LinkSessions.s_LinkedNum; i++)
    {
        if (SessionId == (uint16_t)LinkSessions.s_LinkedSessionId[i])
        {
            break;
        }
    }

    if (i >= LinkSessions.s_LinkedNum)
    {
        return GMI_FAIL;
    }

    *Valid = true;

    return GMI_SUCCESS;
}


