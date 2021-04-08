#pragma once

#include <stdint.h>
#include <memory>

#define SOCKET_FD int
#define MediaSessionId int


static const int MAX_MEDIA_CHANNEL = 2;

enum MediaChannelId
{
	channel_0,
	channel_1
};

enum TransportMode
{
	RTP_OVER_TCP = 1,
	RTP_OVER_UDP = 2,
	RTP_OVER_MULTICAST = 3,
};

/* RTSP服务支持的媒体类型 */
enum MediaType
{
	//PCMU = 0,
	PCMA = 8,
	H264 = 96,
	AAC  = 37,
	H265 = 265,
	NONE
};

struct MediaChannelInfo
{
	//RtpHeader rtp_header;

	// tcp
	uint16_t rtp_channel;
	uint16_t rtcp_channel;

	// udp
	uint16_t rtp_port;
	uint16_t rtcp_port;
	uint16_t packet_seq;
	uint32_t clock_rate;

	// rtcp
	uint64_t packet_count;
	uint64_t octet_count;
	uint64_t last_rtcp_ntp_time;

	bool is_setup;
	bool is_play;
	bool is_record;
};

