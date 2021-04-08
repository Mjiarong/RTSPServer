#include "rtspServer.h"

rtspServer::rtspServer(const std::string& local_ip,int local_port):tcpServer(local_ip,local_port)
{

}


rtspServer::~rtspServer()
{

}

std::shared_ptr<clientConnection> rtspServer::createConnection(int sockfd)
{
    return std::make_shared<rtspClientConnection>(shared_from_this(), sockfd);
}

void rtspServer::SetAuthConfig(std::string realm, std::string username, std::string password)
{
    realm_ = realm;
    username_ = username;
    password_ = password;
    has_auth_info_ = true;

    if (realm_=="" || username=="") {
        has_auth_info_ = false;
    }
}

void rtspServer::SetVersion(std::string version)
{
    version_ = std::move(version);
}

std::string rtspServer::GetVersion()
{
    return version_;
}

int rtspServer::addSession(mediaSession* session)
{
    std::lock_guard<std::mutex> locker(mutex_);

    if (rtsp_suffix_map_.find(session->getPreSuffix()) != rtsp_suffix_map_.end()) {
        return 0;
    }

    std::shared_ptr<mediaSession> media_session(session);
    int sessionId = media_session->GetMediaSessionId();
	rtsp_suffix_map_.emplace(std::move(media_session->getPreSuffix()), sessionId);
	media_sessions_map_.emplace(sessionId, std::move(media_session));

    return sessionId;
}

void rtspServer::removeSession(int sessionId)
{
    std::lock_guard<std::mutex> locker(mutex_);

    auto iter = media_sessions_map_.find(sessionId);
    if(iter != media_sessions_map_.end()) {
        rtsp_suffix_map_.erase(iter->second->getPreSuffix());
        media_sessions_map_.erase(sessionId);
    }
}

std::shared_ptr<mediaSession> rtspServer::lookMediaSession(const std::string& suffix)
{
    std::lock_guard<std::mutex> locker(mutex_);

    auto iter = rtsp_suffix_map_.find(suffix);
    if(iter != rtsp_suffix_map_.end()) {
        int id = iter->second;
        return media_sessions_map_[id];
    }

    return nullptr;
}

std::shared_ptr<mediaSession> rtspServer::lookMediaSession(int session_Id)
{
    std::lock_guard<std::mutex> locker(mutex_);

    auto iter = media_sessions_map_.find(session_Id);
    if(iter != media_sessions_map_.end()) {
        return iter->second;
    }

    return nullptr;
}
