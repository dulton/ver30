
#ifndef  __SDK_SERVER_DAEMON_COMM_H__
#define  __SDK_SERVER_DAEMON_COMM_H__



class SdkServerDaemonComm
{
public:
	SdkServerDaemonComm(SdkServerMgmt* pSvrMgmt,int udpport,int* pRunningBits=NULL);
	~SdkServerDaemonComm();
	int Start();
	void Stop();
private:
};

#endif /*__SDK_SERVER_DAEMON_COMM_H__*/

