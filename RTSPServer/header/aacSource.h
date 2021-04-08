#pragma once

#include "mediaSource.h"

class aacSource : public mediaSource
{
public:
    virtual bool doGetNextFrame();

    static aacSource* CreateNew(uint32_t samplerate=44100, uint32_t channels=2, bool has_adts=true);
    virtual ~aacSource();

    uint32_t GetSamplerate() const
    { return samplerate_; }

    uint32_t GetChannels() const
    { return channels_; }

    static uint32_t GetTimestamp(uint32_t samplerate =44100);

protected:
    std::string filename;

    aacSource(uint32_t samplerate, uint32_t channels, bool has_adts);

    uint32_t samplerate_ = 44100;
    uint32_t channels_ = 2;
    bool has_adts_ = true;

    static const int ADTS_SIZE = 7;
    static const int AU_SIZE   = 4;
};

