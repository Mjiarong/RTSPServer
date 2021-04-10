#pragma once

#include <stdint.h>
#include <memory>

#define RTP_VESION              2

#define RTP_PAYLOAD_TYPE_H264   96
#define RTP_PAYLOAD_TYPE_AAC    97

#define RTP_TCP_HEADER_SIZE     4
#define RTP_HEADER_SIZE         12
#define RTP_MAX_PKT_SIZE        1400
#define RTP_SSRC    0x88923423

struct RtpHeader
{
    /* byte 0 */
    uint8_t csrcLen:4;
    uint8_t extension:1;
    uint8_t padding:1;
    uint8_t version:2;

    /* byte 1 */
    uint8_t payloadType:7;
    uint8_t marker:1;

    /* bytes 2,3 */
    uint16_t seq;

    /* bytes 4-7 */
    uint32_t timestamp;

    /* bytes 8-11 */
    uint32_t ssrc;
};

struct RtpPacket
{
    RtpPacket()
		: payload(new uint8_t[RTP_TCP_HEADER_SIZE+RTP_HEADER_SIZE+RTP_MAX_PKT_SIZE+1000], std::default_delete<uint8_t[]>())
	{
	}

    struct RtpHeader rtpHeader;
    std::shared_ptr<uint8_t> payload;
    uint16_t seq;
    uint32_t timestamp;
};


class rtpUtil
{
public:
    static void rtpHeaderInit(struct RtpPacket* rtpPacket, uint8_t csrcLen, uint8_t extension,
                    uint8_t padding, uint8_t version, uint8_t payloadType, uint8_t marker,
                   uint16_t seq, uint32_t timestamp, uint32_t ssrc);
    static int rtpSendPacketUDP(int socket, const char* ip, int16_t port, uint8_t* data, uint32_t dataSize);
};

