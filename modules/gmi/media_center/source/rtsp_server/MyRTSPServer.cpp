
#include "MyRTSPServer.hh"

MyRTSPServer* MyRTSPServer::createNew(UsageEnvironment& env, Port ourPort,
                                      UserAuthenticationDatabase* authDatabase,
                                      unsigned reclamationTestSeconds )
{
    int ourSocket = -1;

    do
    {
        ourSocket = RTSPServer::setUpOurSocket(env, ourPort);
        if (ourSocket == -1) break;

        return new MyRTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
    }
    while (0);

    if (ourSocket != -1) ::closeSocket(ourSocket);
    return NULL;
}

MyRTSPServer::MyRTSPServer(UsageEnvironment& env, int ourSocket, Port ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds)
    : RTSPServer( env, ourSocket, ourPort, authDatabase, reclamationTestSeconds )
    , m_Callback( NULL )
    , m_UserData( NULL )
{
}

MyRTSPServer::~MyRTSPServer()
{
}

void_t MyRTSPServer::SetCallback( RTSP_EVENT_CALLBACK Callback, void_t *UserData )
{
    m_Callback = Callback;
    m_UserData = UserData;
}

RTSPServer::RTSPClientSession* MyRTSPServer::createNewClientSession( unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr )
{
    return new MyRTSPClientSession(*this, sessionId, clientSocket, clientAddr, m_Callback, m_UserData );
}

MyRTSPServer::MyRTSPClientSession::MyRTSPClientSession( MyRTSPServer& ourServer, unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr, RTSP_EVENT_CALLBACK Callback, void_t *UserData )
    : RTSPServer::RTSPClientSession( ourServer, sessionId, clientSocket, clientAddr )
    , m_Callback( Callback )
    , m_UserData( UserData )
{
}

MyRTSPServer::MyRTSPClientSession::~MyRTSPClientSession()
{
}

void MyRTSPServer::MyRTSPClientSession::handleCmd_PLAY(ServerMediaSubsession* subsession, char const* cseq, char const* fullRequestStr)
{
    printf( "====== MyRTSPServer::MyRTSPClientSession::handleCmd_PLAY ====== this=%p \n", this );
    RTSPServer::RTSPClientSession::handleCmd_PLAY( subsession, cseq, fullRequestStr );

    if ( NULL != m_Callback )
    {
        ReferrencePtr<uint8_t,DefaultObjectsDeleter> EventData;
        m_Callback( m_UserData, RTSP_ET_PLAY, EventData, 0 );
    }
}
