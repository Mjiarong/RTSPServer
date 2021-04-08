#pragma once
#include "EventLoop.h"
#include "msPoller.h"
//#include "rtspServer.h"
#include "tcpClientConnection.h"
#include "rtpSession.h"

class rtspServer;

struct RtspUrlInfo
{
	std::string url;
	std::string username;
	std::string password;
	std::string ip;
	uint16_t port;
	std::string preSuffix;
	std::string suffix;
};

class rtspClientConnection : public clientConnection
{
public:
	enum ConnectionState
	{
		START_CONNECT,
		START_PLAY,
		START_PUSH,
        START_PAUSE,
        START_CLOSE
	};

	rtspClientConnection() = delete;
    rtspClientConnection(std::weak_ptr<tcpServer> ownServer, int clientSocket);
    virtual ~rtspClientConnection();

	int GetMediaSessionId()
	{ return session_id_; }


    std::string GetRtspUrl()
    {
        return rtsp_url_info_.url;
    }

protected:
    virtual int handleRequestBytes(int bytesRead);
    bool parseRTSPURL(char const* url);

private:
    std::shared_ptr<rtpSession> createClientSession();
    static char* getLineFromBuf(char* buf, char* line);
    int sendRtspMessage(int fd);
	int handleCmdOption(unsigned int  cseq);
	int handleCmdDescribe(unsigned int  cseq, char* url);
	int handleCmdSetup(unsigned int  cseq, unsigned int  clientRtpPort);
	int handleCmdPlay(unsigned int  cseq);
    int handleCmdPause(unsigned int  cseq);
	int handleCmdTeardown(unsigned int  cseq);


	ConnectionState conn_state_ = START_CONNECT;
	int  session_id_ = 0;
	struct RtspUrlInfo rtsp_url_info_;

	std::shared_ptr<rtpSession> rtp_session_;
};
