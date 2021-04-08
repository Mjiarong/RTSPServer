#pragma once

#include "media.h"
#include "rtpUtil.h"

#define DEFAULT_MAX_FRAME_SIZE 500000

class mediaSource
{
public:
	mediaSource() {}
	virtual ~mediaSource() {}

    virtual bool doGetNextFrame() = 0;

	virtual MediaType GetMediaType() const
	{ return media_type_; }


    int32_t getFrameSize() const
	{ return frameSize_; }

    uint8_t* getFrameBuff() const
	{ return (uint8_t*)frameBuff; }

	uint32_t getPayloadType() const
	{ return payload_; }

	uint32_t getClockRate() const
	{ return clock_rate_; }

protected:
    uint32_t frameSize_ ;
    uint8_t frameBuff[DEFAULT_MAX_FRAME_SIZE];
	MediaType media_type_ = NONE;
	uint32_t  payload_    = 0;
	uint32_t  clock_rate_ = 0;
};

