#include "h264Source.h"
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <netinet/in.h>

h264Source::h264Source(std::string filename,FILE *fileOpen)
	: mediaSource(),
	  filename_(filename)
{
    payload_    = 96;
    media_type_ = H264;
    clock_rate_ = 90000;
    filePtr = fileOpen;
    framerate_ = 30;
}

h264Source* h264Source::CreateNew(std::string filename)
{
    FILE *fileOpen = fopen(filename.c_str(), "rb");	//只读二进制打开文件
	if (fileOpen == nullptr)
	{
		printf("file_fd_ < 0,filename=%s\n",filename.c_str());
		return nullptr;
	}

    return new h264Source(filename,fileOpen);
}

h264Source::~h264Source()
{

}

uint32_t h264Source::GetTimestamp()
{
    auto time_point = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
    return (uint32_t)((time_point.time_since_epoch().count() + 500) / 1000 * 90 );
}

bool h264Source::startCode3(uint8_t* buf)
{
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return true;
    else
        return false;
}

bool h264Source::startCode4(uint8_t* buf)
{
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return true;
    else
        return false;
}

uint8_t* h264Source::findNextStartCode(uint8_t* buf, int len)
{
    int i;

    if(len < 3)
        return NULL;

    for(i = 0; i < len-3; ++i)
    {
        if(startCode3(buf) || startCode4(buf))
            return buf;

        ++buf;
    }

    if(startCode3(buf))
        return buf;

    return NULL;
}

bool h264Source::doGetNextFrame()//get one Frame from H264File
{
    int ret;
    if (filePtr == nullptr)
	{
		perror("filePtr  == nullptr");
		return false;
	}

    int rSize = fread(frameBuff, 1, DEFAULT_MAX_FRAME_SIZE, filePtr);
    if(rSize<=0)
    {
        perror("rSize<=0");
        return false;
    }

    if(!startCode3(frameBuff) && !startCode4(frameBuff))
        return false;

    uint8_t* nextStartCode = findNextStartCode(frameBuff+3, rSize-3);
    if(!nextStartCode)
    {
        fseek(filePtr, 0, SEEK_SET);
        frameSize_ = rSize;
        bytes_used = 0;
    }
    else
    {
        frameSize_ = (nextStartCode-frameBuff);
        //fseek(filePtr, frameSize_-rSize, SEEK_CUR);
        bytes_used += frameSize_;
        ret = fseek(filePtr, bytes_used, SEEK_SET);
        if(ret!=0)
        {
            printf("ret=%d\n",ret);
            return false;
        }
    }

    return true;

}
