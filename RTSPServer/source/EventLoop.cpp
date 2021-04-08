/* EventLoop.cpp */
#include "EventLoop.h"
#include "event.h"
#include "msPoller.h"
#include <unistd.h>

EventLoop::EventLoop():
  stop(false),
  maxWaitTimeM(1000),
  pollerPtr(new msPoller (this))
{

}

EventLoop::~EventLoop(){

}

void EventLoop::loop()
{
    stop = false;
    while(!stop)
    {
        activeEvents.clear();
        pollerPtr->poll(maxWaitTimeM, &activeEvents);
        for(auto &ch: activeEvents)
        {
            ch->handleEvents();
        }
        usleep(10000);
    }
    return ;
}

int EventLoop::addEvent(int fd, int mask, eventCallback proc, void *clientData)
{

    if (pollerPtr->addPollEvent(fd, mask) == -1)
        return MS_ERR;

    if(!findEvents(fd))    //this event is a new one
    {
        auto msev = std::make_shared<msEvent>(this,fd,mask,clientData);
        if (mask & MS_READABLE) msev->setReadCallback (proc);
        if (mask & MS_WRITABLE) msev->setWriteCallback(proc);
        regEvents[fd] = msev;
    }
    else // it alreay stores in regEvents, we just change the value
    {
        auto msev = regEvents[fd];
        msev->update_mask(mask);
        if (mask & MS_READABLE) msev->setReadCallback (proc);
        if (mask & MS_WRITABLE) msev->setWriteCallback(proc);
        msev->set_clientData(clientData);
    }
    return MS_OK;
}


int EventLoop::deleteEvent(int fd, int del_mask)
{
    pollerPtr->delPollEvent(fd, del_mask);
    if(!findEvents(fd))
    {
        return MS_ERR;
    }
    else
    {
        int mask = regEvents[fd]->get_mask()& (~del_mask);
        if (mask != MS_NONE) //we just change the value
        {
            regEvents[fd]->delete_mask(mask);
        }
        else
        {
            regEvents.erase(fd);
        }
    }
    return MS_OK;
}

bool EventLoop::findEvents(int fd)
{
    if(regEvents.count(fd)==0)
        return false;
    else
        return true;
}
