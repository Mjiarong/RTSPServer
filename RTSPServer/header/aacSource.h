#pragma once

#include "mediaSource.h"

struct adtsHeader
{
    unsigned int syncword;  //12 bit 同步字 '1111 1111 1111'，说明一个ADTS帧的开始
    unsigned int id;        //1 bit MPEG 标示符， 0 for MPEG-4，1 for MPEG-2
    unsigned int layer;     //2 bit 总是'00'
    unsigned int protectionAbsent;  //1 bit 1表示没有crc，0表示有crc
    unsigned int profile;           //1 bit 表示使用哪个级别的AAC
    unsigned int samplingFreqIndex; //4 bit 表示使用的采样频率
    unsigned int privateBit;        //1 bit
    unsigned int channelCfg; //3 bit 表示声道数
    unsigned int originalCopy;         //1 bit
    unsigned int home;                  //1 bit

    /*下面的为改变的参数即每一帧都不同*/
    unsigned int copyrightIdentificationBit;   //1 bit
    unsigned int copyrightIdentificationStart; //1 bit
    unsigned int aacFrameLength;               //13 bit 一个ADTS帧的长度包括ADTS头和AAC原始流
    unsigned int adtsBufferFullness;           //11 bit 0x7FF 说明是码率可变的码流

    /* number_of_raw_data_blocks_in_frame
     * 表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧
     * 所以说number_of_raw_data_blocks_in_frame == 0
     * 表示说ADTS帧中有一个AAC数据块并不是说没有。(一个AAC原始帧包含一段时间内1024个采样及相关数据)
     */
    unsigned int numberOfRawDataBlockInFrame; //2 bit
};

class aacSource : public mediaSource
{
public:
    virtual bool doGetNextFrame();
    virtual uint32_t getFrameIntervalMs();
    static aacSource* CreateNew(std::string filename);
    virtual ~aacSource();

    uint32_t GetSamplerate() const
    { return samplerate_; }

    uint32_t GetChannels() const
    { return channels_; }

    static uint32_t GetTimestamp(uint32_t samplerate =44100);

protected:
    std::string filename_;
    aacSource(std::string filename,FILE *fileOpen);
    FILE *filePtr;

    uint32_t samplerate_ = 44100;
    uint32_t channels_ = 2;
    bool has_adts_ = true;
    struct adtsHeader aac_header_buff_;

    static const int ADTS_SIZE = 7;
    static const int AU_SIZE   = 4;

    aacSource(uint32_t samplerate, uint32_t channels, bool has_adts);
    int parseAdtsHeader(uint8_t* in, struct adtsHeader* res);
};



