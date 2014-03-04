#include <auth_center_api.h>

#include <GroupsockHelper.hh>

#include "RTSPServerEx.hh"

#include "configure.h"

// This definition is copied from RTSPServer.cpp of live555
#define RTSP_PARAM_STRING_MAX 200

// This function is implemented in RTSPServer.cpp of live555
extern char const* dateHeader();

// This funcion is copied fram RTSPServer.cpp of live555, because it is defined with keyword 'static'
static Boolean parseAuthorizationHeader(char const * buf, char const * & username, char const * & realm, char const * & nonce, char const * & uri, char const * & response) {
    // Initialize the result parameters to default values:
    username = realm = nonce = uri = response = NULL;

    // First, find "Authorization:"
    while (1) {
        if (*buf == '\0') return False; // not found
        if (strncasecmp(buf, "Authorization: Digest ", 22) == 0) break;
        ++buf;
    }

    // Then, run through each of the fields, looking for ones we handle:
    char const * fields = buf + 22;
    while (*fields == ' ') ++fields;
    char * parameter = strDupSize(fields);
    char * value = strDupSize(fields);
    while (1) {
        value[0] = '\0';
        if (sscanf(fields, "%[^=]=\"%[^\"]\"", parameter, value) != 2 &&
            sscanf(fields, "%[^=]=\"\"", parameter) != 1) {
            break;
        }
        if (strcmp(parameter, "username") == 0) {
            username = strDup(value);
        } else if (strcmp(parameter, "realm") == 0) {
            realm = strDup(value);
        } else if (strcmp(parameter, "nonce") == 0) {
            nonce = strDup(value);
        } else if (strcmp(parameter, "uri") == 0) {
            uri = strDup(value);
        } else if (strcmp(parameter, "response") == 0) {
            response = strDup(value);
        }

        fields += strlen(parameter) + 2 /*="*/ + strlen(value) + 1 /*"*/;
        while (*fields == ',' || *fields == ' ') ++fields;
        // skip over any separating ',' and ' ' chars
        if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
    }
    delete[] parameter;
    delete[] value;
    return True;
}


RTSPServerEx * RTSPServerEx::createNew(UsageEnvironment & env, Port ourPort, const char * realm, unsigned reclamationTestSeconds) {
    int ourSocket = setUpOurSocket(env, ourPort);
    if (ourSocket == -1) return NULL;

    return new RTSPServerEx(env, ourSocket, ourPort, realm, reclamationTestSeconds);
}

RTSPServerEx::RTSPServerEx(UsageEnvironment & env, int ourSocket, Port ourPort, const char * realm, unsigned reclamationTestSeconds)
    : RTSPServer(env, ourSocket, ourPort, NULL, reclamationTestSeconds) {
    snprintf(fRealm, sizeof(fRealm), "%s", realm == NULL ? "GMI_RTSP_SERVER" : realm);
}

RTSPServerEx::~RTSPServerEx() {
    // Nothing to do
}

RTSPServerEx::RTSPClientConnection * RTSPServerEx::createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr) {
    DUMP_VARIABLE(clientSocket);

    return new RTSPClientConnectionEx(*this, clientSocket, clientAddr);
}

RTSPServerEx::RTSPClientConnectionEx::RTSPClientConnectionEx(RTSPServerEx & ourServer, int clientSocket, struct sockaddr_in clientAddr)
    : RTSPClientConnection(ourServer, clientSocket, clientAddr)
    , fOurServerEx(ourServer)
    , fSessionId(0)
    , fUsername(NULL)
    , fNonce(NULL) {
}

RTSPServerEx::RTSPClientConnectionEx::~RTSPClientConnectionEx() {
    if (fSessionId > 0 && fUsername != NULL) {
        if (GMI_SUCCESS != GMI_UserLogoutNotify(fSessionId, Configure::GetInstance().GetAuthLocalPort())) {
            PRINT_LOG(ERROR, "Failed to logout %s", fUsername);
        }

        PRINT_LOG(VERBOSE, "User %s is logout, session id = %u", fUsername, fSessionId);
    }

    delete [] fUsername;
    delete [] fNonce;
}

void RTSPServerEx::RTSPClientConnectionEx::handleCmd_DESCRIBE(char const * urlPreSuffix, char const * urlSuffix, char const * fullRequestStr) {
    char * sdpDescription = NULL;
    char * rtspURL        = NULL;
    do {
        char urlTotalSuffix[RTSP_PARAM_STRING_MAX];
        if (strlen(urlPreSuffix) + strlen(urlSuffix) + 2 > sizeof urlTotalSuffix) {
            handleCmd_bad();
            break;
        }
        urlTotalSuffix[0] = '\0';
        if (urlPreSuffix[0] != '\0') {
            strcat(urlTotalSuffix, urlPreSuffix);
            strcat(urlTotalSuffix, "/");
        }
        strcat(urlTotalSuffix, urlSuffix);

        /*
         * We need to override RTSPServer::RTSPClientConnection::authenticationOK(char const *, char const *, char const *), 
         * but this member method is not a virtual function, so we need to override the function who called it.
         */
        if (!authenticationOKEx("DESCRIBE", urlTotalSuffix, fullRequestStr)) break;

        // We should really check that the request contains an "Accept:" #####
        // for "application/sdp", because that's what we're sending back #####

        // Begin by looking up the "ServerMediaSession" object for the specified "urlTotalSuffix":
        ServerMediaSession* session = fOurServer.lookupServerMediaSession(urlTotalSuffix);
        if (session == NULL) {
            handleCmd_notFound();
            break;
        }
    
        // Then, assemble a SDP description for this session:
        sdpDescription = session->generateSDPDescription();
        if (sdpDescription == NULL) {
            // This usually means that a file name that was specified for a
            // "ServerMediaSubsession" does not exist.
            setRTSPResponse("404 File Not Found, Or In Incorrect Format");
            break;
        }
        unsigned sdpDescriptionSize = strlen(sdpDescription);

        // Also, generate our RTSP URL, for the "Content-Base:" header
        // (which is necessary to ensure that the correct URL gets used in subsequent "SETUP" requests).
        rtspURL = fOurServer.rtspURL(session, fClientInputSocket);

        snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
                 "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
                 "%s"
                 "Content-Base: %s/\r\n"
                 "Content-Type: application/sdp\r\n"
                 "Content-Length: %d\r\n\r\n"
                 "%s",
                 fCurrentCSeq,
                 dateHeader(),
                 rtspURL,
                 sdpDescriptionSize,
                 sdpDescription);
    } while (0);

    delete[] sdpDescription;
    delete[] rtspURL;
}

Boolean RTSPServerEx::RTSPClientConnectionEx::authenticationOKEx(char const * cmdName, char const * urlSuffix, char const * fullRequestStr) {
    if (!fOurServerEx.specialClientAccessCheck(fClientInputSocket, fClientAddr, urlSuffix)) {
        setRTSPResponse("401 Unauthorized");
        return False;
    }

    char const * realm    = NULL;
    char const * nonce    = NULL;
    char const * uri      = NULL;
    char const * response = NULL;
    Boolean      success  = False;

    do {
        // To authenticate, we first need to have a nonce set up
        // from a previous attempt:
        if (NULL == fNonce) {
            break;
        }

        // Release username first
        delete [] fUsername;

        // Next, the request needs to contain an "Authorization:" header,
        // containing a username, (our) realm, (our) nonce, uri,
        // and response string:
        if (!parseAuthorizationHeader(fullRequestStr, fUsername, realm, nonce, uri, response)
            || fUsername == NULL
            || realm == NULL || strcmp(realm, fOurServerEx.fRealm) != 0
            || nonce == NULL || strcmp(nonce, fNonce) != 0
            || uri == NULL || response == NULL) {
            break;
        }

        UserAuthRefInfo Request;
        UserAuthExtInfo RequestExt;
        UserAuthResInfo Response;

        memset(&Request, 0x00, sizeof(Request));
        memset(&RequestExt, 0x00, sizeof(RequestExt));
        memset(&Response, 0x00, sizeof(Response));

        snprintf(RequestExt.s_Realm, sizeof(RequestExt.s_Realm), "%s", realm);
        snprintf(RequestExt.s_Nonce, sizeof(RequestExt.s_Nonce), "%s", nonce);
        snprintf(RequestExt.s_Cmd, sizeof(RequestExt.s_Cmd), "%s", cmdName);
        snprintf(RequestExt.s_Url, sizeof(RequestExt.s_Url), "%s", uri);

        Request.s_DataType = TYPE_AUTH_LOGIN;
        snprintf(Request.s_Username, sizeof(Request.s_Username), "%s", fUsername);
        snprintf(Request.s_Password, sizeof(Request.s_Password), "%s", response);
        Request.s_UsernameEncType = TYPE_ENCRYPTION_TEXT;
        Request.s_PasswordEncType = TYPE_ENCRYPTION_MD5_RTSP;
        Request.s_UserAuthExtDataLen = sizeof(RequestExt);
        Request.s_UserAuthExtData = (char *)&RequestExt;
        Request.s_MoudleId = ID_MOUDLE_REST_RTSP;
        Request.s_SessionId = fSessionId;

        GMI_RESULT RetVal = GMI_UserAuthCheck(&Request, &Response, Configure::GetInstance().GetAuthLocalPort());
        if (RetVal != GMI_SUCCESS) {
            PRINT_LOG(ERROR, "Failed to login %s", fUsername);
            break;
        }

        if (GMI_CODE_SUCCESS != Response.s_AuthResult) {
            PRINT_LOG(INFO, "Failed to auth user %s", fUsername);
            break;
        }

        fSessionId = Response.s_SessionId;
        success = True;

        PRINT_LOG(VERBOSE, "User %s is login, session id = %u", fUsername, fSessionId);
    } while (0);

    delete [] realm;
    delete [] nonce;
    delete [] uri;
    delete [] response;

    if (success) {
        // The user has been authenticated.
        // Now allow subclasses a chance to validate the user against the IP address and/or URL suffix.
        if (!fOurServerEx.specialClientUserAccessCheck(fClientInputSocket, fClientAddr, urlSuffix, fUsername)) {
            // Note: We don't return a "WWW-Authenticate" header here, because the user is valid,
            // even though the server has decided that they should not have access.
            setRTSPResponse("401 Unauthorized");
            return False;
        }
    }

    if (success) {
        return True;
    }

    u_int32_t randomValue = (u_int32_t)our_random32();
    char      nonceBuf[9];

    snprintf(nonceBuf, sizeof(nonceBuf), "%08X", randomValue);

    fNonce = strDup(nonceBuf);

    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
             "RTSP/1.0 401 Unauthorized\r\n"
             "CSeq: %s\r\n"
             "%s"
             "WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"\r\n\r\n",
             fCurrentCSeq,
             dateHeader(),
             fOurServerEx.fRealm, fNonce);

    return False;
}

