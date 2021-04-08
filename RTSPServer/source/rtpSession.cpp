#include "rtpSession.h"
#include <iostream>

rtpSession::rtpSession(std::weak_ptr<clientConnection> rtsp_connection)
{
    rtsp_connection_ = rtsp_connection;

}

rtpSession::~rtpSession()
{
    for(int i=0;i<MAX_MEDIA_CHANNEL;i++)
    {
        if(rtpfd_[i]>0)
            close(rtpfd_[i]);
    }

}

void rtpSession::rtpSessionSetup(int channel,unsigned int clientRtpPort,int sock_fd)
{
    memset(&rtp_header_info[channel],0,sizeof(RtpHeader));
    rtpfd_[channel] = socketUtil::CreateUdpSocket();
    setPeerRtpPort(channel,clientRtpPort);
    setRtspFd(sock_fd);
    setPeerIP(socketUtil::GetPeerIp(sock_fd));
}
