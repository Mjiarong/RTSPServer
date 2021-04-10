#pragma once
#include "mediaSource.h"
#include "media.h"
#include "rtpSession.h"
#include "rtpUtil.h"
#include <mutex>
#include <shared_mutex> //C++14
#include <unordered_map>
#include <atomic>

class mediaSession
{
public:
    static mediaSession* CreateNew(std::string pre_suffix="live");
	~mediaSession();

	bool addSource(int channel_id, mediaSource* source);
	bool removeSource(int channel_id);

    std::string getPreSuffix() const
	{ return preSuffix_; }

	void SetPreSuffix(std::string& preSuffix)
	{ preSuffix_ = preSuffix; }

	std::shared_ptr<mediaSource> lookSource(int channel_id);

	bool addClient(int rtspfd, std::shared_ptr<rtpSession> rtp_conn);
	void removeClient(int rtspfd);

    int GetMediaSessionId()
	{ return session_id_; }

    unsigned int GetNumClient() const
	{ return (unsigned int)clients_.size(); }

	void sendFrame(int channel);
	int rtpSendH264Frame(int channel, std::shared_ptr<rtpSession> session);
	int rtpSendAACFrame(int channel, std::shared_ptr<rtpSession> session);

private:
	mediaSession(std::string preSuffix);

	int session_id_ = 0;
	std::string preSuffix_;
	std::string sdp;
	std::mutex mutex_;
	//std::mutex map_mutex_;
	std::shared_timed_mutex map_mutex_; //C++14
    std::vector<std::shared_ptr<mediaSource>> media_sources_;
	std::unordered_map<SOCKET_FD, std::shared_ptr<rtpSession>> clients_;
	std::atomic_bool has_new_client_;
	static std::atomic_uint last_session_id_;
	std::vector<std::shared_ptr<RtpPacket>> rtpPackets_;
};
