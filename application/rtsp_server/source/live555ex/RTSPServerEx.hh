#ifndef _RTSP_SERVER_EX_HH
#define _RTSP_SERVER_EX_HH

#include <RTSPServer.hh>

class RTSPServerEx : public RTSPServer {
public:
    static RTSPServerEx * createNew(UsageEnvironment & env, Port ourPort = 554, const char * realm = NULL, unsigned reclamationTestSeconds = 65);

    class RTSPClientConnectionEx : public RTSPClientConnection {
    public:
        RTSPClientConnectionEx(RTSPServerEx & ourServer, int clientSocket, struct sockaddr_in clientAddr);
        virtual ~RTSPClientConnectionEx();

    protected:
        virtual void handleCmd_DESCRIBE(char const * urlPreSuffix, char const * urlSuffix, char const * fullRequestStr);

        // We need to override RTSPServer::RTSPClientConnection::authenticationOK(char const *, char const *, char const *)
        Boolean authenticationOKEx(char const * cmdName, char const * urlSuffix, char const * fullRequestStr);

        RTSPServerEx & fOurServerEx;
        u_int16_t      fSessionId;
        const char   * fUsername;
        char         * fNonce;
    };

protected:
    RTSPServerEx(UsageEnvironment & env, int ourSocket, Port ourPort, const char * realm, unsigned reclamationTestSeconds);

    virtual ~RTSPServerEx();

    virtual RTSPClientConnection * createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr);

    char fRealm[33];

    friend class RTSPClientConnectionEx;
};

#endif

