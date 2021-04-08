/* event.cpp */
#include "event.h"
#include "EventLoop.h"
#include <poll.h>

msEvent::msEvent(EventLoop *loop, int fd, int mask, void* clientData):
  ownerLoop(loop),
  fd(fd),
  mask(mask),
  clientData(clientData)
{

}

msEvent::~msEvent()
{
}

void msEvent::handleEvents() {
  if(mask & MS_READABLE){
    if(readCallback) readCallback(fd);
  }
  if(mask & MS_WRITABLE){
    if(writeCallback) writeCallback(fd);
  }
}
