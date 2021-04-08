#pragma once

#include "media.h"
#include <netinet/in.h>
#include "tcpClientConnection.h"
#include "rtpUtil.h"

class rtpSession
{
public:
    using SendFrameCallback = std::function<bool (int channel_id, std::shared_ptr<rtpSession> session)>;

    rtpSession(std::weak_ptr<clientConnection> rtsp_connection);
    ~rtpSession();

    uint16_t getPeerRtpPort(int channel_id) const
    { return peer_rtp_port_[channel_id]; }

    void setPeerRtpPort(int channel_id, int port)
    {  peer_rtp_port_[channel_id] = port; }

    void setPeerIP(std::string ip)
    {  peer_ip_ = ip; }

    std::string getPeerIp()
    { return peer_ip_; }

    void setRtspFd(int fd)
    {  rtspfd_ = fd; }

    int getRtpSocket(int channel_id) const
    { return rtpfd_[channel_id];}

    bool isClosed() const
    { return is_closed_; }

    void rtpSessionSetup(int channel,unsigned int clientRtpPort,int sock_fd);

    void SetSendFrameCallback(const SendFrameCallback callback, int channel)
	{ send_frame_callback_[channel]= callback; }

private:
    friend class mediaSession;

	std::weak_ptr<clientConnection> rtsp_connection_;
    int rtspfd_;
    unsigned int  local_rtp_port_[MAX_MEDIA_CHANNEL];
    unsigned int  local_rtcp_port_[MAX_MEDIA_CHANNEL];
    int  rtpfd_[MAX_MEDIA_CHANNEL];

    SendFrameCallback send_frame_callback_[MAX_MEDIA_CHANNEL];
    unsigned int  peer_rtp_port_[MAX_MEDIA_CHANNEL];
    std::string peer_ip_;
    RtpHeader rtp_header_info[MAX_MEDIA_CHANNEL];
	bool is_closed_ = false;
};
