#include "aacSource.h"
#include <chrono>

aacSource::aacSource(uint32_t samplerate, uint32_t channels, bool has_adts)
	: samplerate_(samplerate)
	, channels_(channels)
	, has_adts_(has_adts)
{
	payload_    = 97;
	media_type_ = AAC;
	clock_rate_ = samplerate;
}

aacSource* aacSource::CreateNew(uint32_t samplerate, uint32_t channels, bool has_adts)
{
    return new aacSource(samplerate, channels, has_adts);
}

aacSource::~aacSource()
{

}

uint32_t aacSource::GetTimestamp(uint32_t sampleRate)
{
	auto time_point = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::steady_clock::now());
	return (uint32_t)((time_point.time_since_epoch().count()+500) / 1000 * sampleRate / 1000);
}

bool aacSource::doGetNextFrame()//getFrameFromAacFile
{
    return true;

}
