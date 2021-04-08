#include "buffer.h"
#include <malloc.h>
#include <cstring>

msBuffer::msBuffer(unsigned int  cap)
{
    data_ = (char *)malloc(cap);
    if(!data_)
    {
        capacity_ = 0;
    }
    capacity_ = cap;
}

msBuffer::~msBuffer()
{
    if(data_) free(data_);
}

void msBuffer::clear() {
  memset(data_,0,capacity_);
}

int msBuffer::resize(unsigned int  cap) {
    data_ = (char *)realloc(data_,cap);
    if(!data_)
    {
        return -1;
    }
    capacity_ = cap;
    return 0;
}
