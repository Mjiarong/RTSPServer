#include "mediaSession.h"
#include "rtpUtil.h"
#include <iostream>

std::atomic_uint mediaSession::last_session_id_(1);

mediaSession::mediaSession(std::string preSuffix)
	: preSuffix_(preSuffix),
	  media_sources_(MAX_MEDIA_CHANNEL)
{
	session_id_ = ++last_session_id_;
}

mediaSession::~mediaSession()
{

}

mediaSession* mediaSession::CreateNew(std::string pre_suffix)
{
	return new mediaSession(std::move(pre_suffix));
}

bool mediaSession::addSource(int channel_id, mediaSource* source)
{
    media_sources_[channel_id].reset(source);
	return true;
}

bool mediaSession::removeSource(int channel_id)
{
	media_sources_[channel_id] = nullptr;
	return true;
}

mediaSource* mediaSession::getMediaSource(int channel_id)
{
	if (media_sources_[channel_id]) {
		return media_sources_[channel_id].get();
	}

	return nullptr;
}

bool mediaSession::addClient(SOCKET rtspfd, std::shared_ptr<rtpSession> rtp_conn)
{
	std::lock_guard<std::mutex> lock(map_mutex_);

	auto iter = clients_.find (rtspfd);
	if(iter == clients_.end()) {
		clients_.emplace(rtspfd, rtp_conn);
		has_new_client_ = true;
		return true;
	}

	return false;
}

void mediaSession::removeClient(SOCKET rtspfd)
{
	std::lock_guard<std::mutex> lock(map_mutex_);

	auto iter = clients_.find(rtspfd);
	if (iter != clients_.end()) {
        clients_.erase(iter);
	}
}



int mediaSession::sendRtpVideoFrame(int channel, std::shared_ptr<rtpSession> session)
{
    mediaSource* source = getMediaSource(channel);
    if (!source)
    {
        return MS_ERR;
    }

    RtpPacket* rtpPacket = &rtpPackets_;
    uint8_t *frame = source->getFrameBuff();
    uint32_t frameSize = source->getFrameSize();

    int socket = session->rtpfd_[channel];
    uint16_t port = session->peer_rtp_port_[channel];
    std::string ip = session->peer_ip_;

    int sendBytes = 0;
    int ret;
    auto payload = rtpPacket->payload.get();

    RtpHeader* header_info = &(session->rtp_header_info[channel]);
    rtpPacket->rtpHeader.version = RTP_VESION;
    rtpPacket->rtpHeader.payloadType = RTP_PAYLOAD_TYPE_H264;
    rtpPacket->rtpHeader.ssrc = htonl(RTP_SSRC);
    rtpPacket->rtpHeader.timestamp = htonl(header_info->timestamp);


    if(frame[0] == 0 && frame[1] == 0 && frame[2] == 1) //delete startcode
    {
        frame+=3;
        frameSize -=3;
    }
    else if(frame[0] == 0 && frame[1] == 0 && frame[2] == 0 && frame[3] == 1)
    {
        frame+=4;
        frameSize -=4;
    }

    uint8_t naluType = frame[0];// nalu第一个字节
    if (frameSize <= RTP_MAX_PKT_SIZE) // nalu长度小于最大包长：单一NALU单元模式
    {
        /*
         *   0 1 2 3 4 5 6 7 8 9
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         *  |F|NRI|  Type   | a single NAL unit ... |
         *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         */
        rtpPacket->rtpHeader.seq = htons(header_info->seq++);
        memcpy(payload, &(rtpPackets_.rtpHeader), RTP_HEADER_SIZE);
        memcpy(payload+RTP_HEADER_SIZE, frame, frameSize);
        ret = rtpUtil::rtpSendPacketUDP(socket, ip.c_str(), port, payload, frameSize+RTP_HEADER_SIZE);
        if(ret < 0)
            return -1;
        sendBytes += ret;
        if ((naluType & 0x1F) == 7 || (naluType & 0x1F) == 8) // 如果是SPS、PPS就不需要加时间戳
                return sendBytes;
    }
    else // nalu长度大于最大包长：分片模式
    {
        /*
         *  0                   1                   2
         *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         * | FU indicator  |   FU header   |   FU payload   ...  |
         * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
         */

        /*
         *     FU Indicator
         *    0 1 2 3 4 5 6 7
         *   +-+-+-+-+-+-+-+-+
         *   |F|NRI|  Type   |
         *   +---------------+
         */

        /*
         *      FU Header
         *    0 1 2 3 4 5 6 7
         *   +-+-+-+-+-+-+-+-+
         *   |S|E|R|  Type   |
         *   +---------------+
         */
        frame+=1;
        frameSize -=1;
        int pktNum = frameSize / RTP_MAX_PKT_SIZE;       // 有几个完整的包
        int remainPktSize = frameSize % RTP_MAX_PKT_SIZE; // 剩余不完整包的大小
        int i, pos = 0;

        /* 发送完整的包 */
        for (i = 0; i < pktNum; i++)
        {
            rtpPacket->rtpHeader.seq = htons(header_info->seq++);
            memcpy(payload, &(rtpPacket->rtpHeader), RTP_HEADER_SIZE);
            payload[0+RTP_HEADER_SIZE] = (naluType & 0xE0) + 0x1C ;
            payload[1+RTP_HEADER_SIZE] = naluType & 0x1F;
            if (i == 0) //第一包数据
                payload[1+RTP_HEADER_SIZE] |= 0x80; // start
            else if (remainPktSize == 0 && i == pktNum - 1) //最后一包数据
                payload[1+RTP_HEADER_SIZE] |= 0x40; // end
            memcpy(payload+RTP_HEADER_SIZE+2, frame+pos, RTP_MAX_PKT_SIZE);
            ret = rtpUtil::rtpSendPacketUDP(socket, ip.c_str(), port, payload, RTP_MAX_PKT_SIZE+RTP_HEADER_SIZE+2);
            if(ret < 0)
                return ret;
            sendBytes += ret;
            pos += RTP_MAX_PKT_SIZE;
        }

        /* 发送剩余的数据 */
        if (remainPktSize > 0)
        {
            rtpPacket->rtpHeader.seq = htons(header_info->seq++);
            memcpy(payload, &(rtpPacket->rtpHeader), RTP_HEADER_SIZE);
            payload[0+RTP_HEADER_SIZE] = (naluType & 0xE0) + 0x1C ;
            payload[1+RTP_HEADER_SIZE] = naluType & 0x1F;
            payload[1+RTP_HEADER_SIZE] |= 0x40; //end
            memcpy(payload+RTP_HEADER_SIZE+2, frame+pos, remainPktSize);
            ret = rtpUtil::rtpSendPacketUDP(socket, ip.c_str(), port, payload, remainPktSize+2+RTP_HEADER_SIZE);
            if(ret < 0)
                return ret;

            sendBytes += ret;
        }
    }
    header_info->timestamp+= 90000/30;
    return sendBytes;
}

void mediaSession::sendFrame(int channel)
{
    mediaSource* source = getMediaSource(channel);
    if (source != nullptr && source->doGetNextFrame())
    {
        std::lock_guard<std::mutex> lock(map_mutex_);
        for (auto iter = clients_.begin(); iter != clients_.end(); ++iter)
        {
            auto session = iter->second;
            if (session->send_frame_callback_)
            {
                session->send_frame_callback_[channel](channel, session);
            }

        }

    }
}

