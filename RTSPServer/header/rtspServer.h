#pragma once
#include "EventLoop.h"
#include "msPoller.h"
#include "tcpServer.h"
#include "rtspClientConnection.h"
#include "mediaSession.h"
#include "media.h"

#include <string>
#include <mutex>
#include <unordered_map>


class rtspServer : public tcpServer
{
public:
    //static std::shared_ptr<rtspServer> Create(EventLoop* loop);
    rtspServer(const std::string& local_ip,int local_port);
    ~rtspServer();

    std::shared_ptr<clientConnection> createConnection(int sockfd);

    int addSession(mediaSession* session);
    void removeSession(int sessionId);
    std::shared_ptr<mediaSession> lookMediaSession(const std::string& suffix);
    std::shared_ptr<mediaSession> lookMediaSession(int session_Id);

    void SetAuthConfig(std::string realm, std::string username, std::string password);
    void SetVersion(std::string version); // SDP Session Name
    std::string GetVersion();
    std::string GetRtspUrl();
    bool ParseRtspUrl(std::string url);
    int rtp_port()
    {
        return rtp_port_;
    }

    int rtcp_port()
    {
        return rtcp_port_;
    }


    //bool PushFrame(int sessionId, MediaChannelId channelId, AVFrame frame);

private:
    //friend class RtspConnection;
    //MediaSession::Ptr LookMediaSession(const std::string& suffix);
    //MediaSession::Ptr LookMediaSession(MediaSessionId session_id);
    //virtual TcpConnection::Ptr OnConnect(SOCKET sockfd);
	bool has_auth_info_ = false;
	bool start_ = true;
	std::string realm_;
	std::string username_;
	std::string password_;
	std::string version_;


    std::mutex mutex_;
    std::unordered_map<SOCKET_FD, std::shared_ptr<rtspClientConnection>> rtsp_connections_map;
    std::unordered_map<MediaSessionId, std::shared_ptr<mediaSession>> media_sessions_map_;
    std::unordered_map<std::string, MediaSessionId> rtsp_suffix_map_;

    int rtp_port_ = 55532;
    int rtcp_port_ = 55533;
};

