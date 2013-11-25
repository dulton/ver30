#ifndef __MY_RTSP_SERVER_HH__
#define __MY_RTSP_SERVER_HH__

#include "gmi_system_headers.h"
#include "RTSPServer.hh"

#if !defined( RTSP_EVENT_PROCESS_DEF )
#define RTSP_EVENT_PROCESS_DEF 1
enum RTSP_EventType
{
	RTSP_ET_OPTIONS       = 1,
	RTSP_ET_DESCRIBE      = 2,
	RTSP_ET_SETUP         = 3,
	RTSP_ET_PLAY          = 4,
	RTSP_ET_PAUSE         = 5,
	RTSP_ET_GET_PARAMETER = 6,
	RTSP_ET_SET_PARAMETER = 7,
	RTSP_ET_TEARDOWN      = 8
};

typedef void_t (*RTSP_EVENT_CALLBACK)( void_t *UserData, uint32_t EventType, ReferrencePtr<uint8_t,DefaultObjectsDeleter>& EventData, size_t EventDataLength );
#endif//RTSP_EVENT_PROCESS_DEF

class MyRTSPServer : public RTSPServer
{
public:
	static MyRTSPServer* createNew(UsageEnvironment& env, Port ourPort = 554, UserAuthenticationDatabase* authDatabase = NULL, unsigned reclamationTestSeconds = 65);

	void_t SetCallback( RTSP_EVENT_CALLBACK Callback, void_t *UserData );

protected:
	MyRTSPServer(UsageEnvironment& env, int ourSocket, Port ourPort, UserAuthenticationDatabase* authDatabase, unsigned reclamationTestSeconds);// called only by createNew();
	virtual ~MyRTSPServer();

protected:
	class MyRTSPClientSession : public RTSPServer::RTSPClientSession
	{
	public:
		MyRTSPClientSession(MyRTSPServer& ourServer, unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr, RTSP_EVENT_CALLBACK Callback, void_t *UserData );
		virtual ~MyRTSPClientSession();

		virtual void handleCmd_PLAY(ServerMediaSubsession* subsession, char const* cseq, char const* fullRequestStr);

	protected:
		RTSP_EVENT_CALLBACK m_Callback;
		void_t              *m_UserData;
	};

protected:
	virtual RTSPServer::RTSPClientSession* createNewClientSession(unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr);

protected:
	RTSP_EVENT_CALLBACK m_Callback;
	void_t              *m_UserData;
};

#endif
