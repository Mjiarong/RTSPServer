#include "aacSource.h"
#include <chrono>
#include <string.h>

aacSource::aacSource(std::string filename,FILE *fileOpen)
	: mediaSource(),
	filename_(filename),
	filePtr(fileOpen)
{
	payload_    = 97;
	media_type_ = AAC;
}

aacSource* aacSource::CreateNew(std::string filename)
{
    FILE *fileOpen = fopen(filename.c_str(), "rb");	//只读二进制打开文件
	if (fileOpen == nullptr)
	{
		printf("file_fd_ < 0,filename=%s\n",filename.c_str());
		return nullptr;
	}

    return new aacSource(filename,fileOpen);
}

aacSource::~aacSource()
{

}

uint32_t aacSource::GetTimestamp(uint32_t sampleRate)
{
	auto time_point = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
	return (uint32_t)((time_point.time_since_epoch().count()+500) / 1000 * sampleRate / 1000);
}

int aacSource::parseAdtsHeader(uint8_t* in, struct adtsHeader* res)
{
    memset(res,0,sizeof(*res));

    if ((in[0] == 0xFF)&&((in[1] & 0xF0) == 0xF0))
    {
        res->id = ((unsigned int) in[1] & 0x08) >> 3;
        res->layer = ((unsigned int) in[1] & 0x06) >> 1;
        res->protectionAbsent = (unsigned int) in[1] & 0x01;
        res->profile = ((unsigned int) in[2] & 0xc0) >> 6;
        res->samplingFreqIndex = ((unsigned int) in[2] & 0x3c) >> 2;
        res->privateBit = ((unsigned int) in[2] & 0x02) >> 1;
        res->channelCfg = ((((unsigned int) in[2] & 0x01) << 2) | (((unsigned int) in[3] & 0xc0) >> 6));
        res->originalCopy = ((unsigned int) in[3] & 0x20) >> 5;
        res->home = ((unsigned int) in[3] & 0x10) >> 4;
        res->copyrightIdentificationBit = ((unsigned int) in[3] & 0x08) >> 3;
        res->copyrightIdentificationStart = (unsigned int) in[3] & 0x04 >> 2;
        res->aacFrameLength = (((((unsigned int) in[3]) & 0x03) << 11) |
                                (((unsigned int)in[4] & 0xFF) << 3) |
                                    ((unsigned int)in[5] & 0xE0) >> 5) ;
        //printf( "adts:aac_frame_length  %d\n", res->aacFrameLength);
        res->adtsBufferFullness = (((unsigned int) in[5] & 0x1f) << 6 |
                                        ((unsigned int) in[6] & 0xfc) >> 2);
        res->numberOfRawDataBlockInFrame = ((unsigned int) in[6] & 0x03);

        return 0;
    }
    else
    {
        printf("failed to parse adts header\n");
        return -1;
    }
}

uint32_t aacSource::getFrameIntervalMs()
{
    return 1000*1000/(samplerate_/1024);
}

bool aacSource::doGetNextFrame()//getFrameFromAacFile
{
    if (filePtr == nullptr)
	{
		perror("filePtr  == nullptr");
		return false;
	}

    int rSize = fread(frameBuff,1, ADTS_SIZE, filePtr);
    if(rSize <= 0)
    {
        fseek(filePtr, 0, SEEK_SET);
        return false;
    }

    if(parseAdtsHeader(frameBuff, &aac_header_buff_)<0)
    {
        perror("parse err\n");
        return false;
    }

    frameSize_ = fread(frameBuff, 1, aac_header_buff_.aacFrameLength-ADTS_SIZE, filePtr);
    if(frameSize_ < 0)
    {
        perror("frameSize_<=0");
        return false;
    }
    return true;

}
