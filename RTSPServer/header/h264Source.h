#pragma onces

#include "mediaSource.h"
#include "rtpUtil.h"

class h264Source : public mediaSource
{
public:
	static h264Source* CreateNew(std::string filename);
	~h264Source();

	void SetFramerate(uint32_t framerate)
	{ framerate_ = framerate; }

	uint32_t GetFramerate() const
	{ return framerate_; }

    static uint32_t GetTimestamp();
    virtual bool doGetNextFrame();

protected:
    std::string filename_;
    h264Source(std::string filename,FILE *fileOpen);
	uint32_t framerate_ = 30;

    FILE *filePtr;
    int file_flag = 0;
    int bytes_used = 0;

private:
    bool startCode3(uint8_t* buf);
    bool startCode4(uint8_t* buf);
    uint8_t* findNextStartCode(uint8_t* buf, int len);
};
