#pragma once
#include "mediaSource.h"
#include "media.h"
#include "rtpSession.h"
#include <mutex>
#include <unordered_map>

class mediaSession
{
public:
    static mediaSession* CreateNew(std::string url_suffix="live");
	~mediaSession();

	bool AddSource(int channel_id, mediaSource* source);
	bool RemoveSource(int channel_id);

    std::string GetSuffix() const
	{ return preSuffix_; }

	void SetSuffix(std::string& suffix)
	{ preSuffix_ = suffix; }

    std::string GetSdpMessage(std::string ip, std::string session_name ="");

	mediaSource* GetMediaSource(int channel_id);

	bool HandleFrame(int channel_id, AVFrame frame);

	bool AddClient(int rtspfd, std::shared_ptr<rtpSession> rtp_conn);
	void RemoveClient(int rtspfd);

    int GetMediaSessionId()
	{ return session_id; }

	//uint32_t GetNumClient() const
	//{ return (uint32_t)clients.size(); }

private:
	mediaSession(std::string url_suffxx);

	int session_id = 0;
	std::string preSuffix_;
	std::string sdp;

	std::vector<std::unique_ptr<mediaSource>> frame_sources;
	//std::vector<RingBuffer<AVFrame>> buffer;

	//std::vector<NotifyConnectedCallback> notify_connected_callbacks_;
	//std::vector<NotifyDisconnectedCallback> notify_disconnected_callbacks_;
	std::mutex mutex_;
	std::mutex map_mutex_;
	std::unordered_map<SOCKET_FD, std::shared_ptr<rtpSession>> clients;

	//std::atomic_bool has_new_client_;

	//static std::atomic_uint last_session_id;
};
