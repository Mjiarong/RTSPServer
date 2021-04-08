#include <assert.h>
#include <sys/epoll.h>
#include "msPoller.h"
#include "event.h"


msPoller::msPoller(EventLoop *loop):
    eventLoop(loop),
    epEvents(epEventsListSize)
{
    epollfd = epoll_create(1024);
}

msPoller::~msPoller() {

}

int msPoller::addPollEvent(int fd, int add_mask){
    int old_mask = MS_NONE,op;
    if(!eventLoop->findEvents(fd))//this event is a new one
    {
        op = EPOLL_CTL_ADD;
    }
    else
    {
        old_mask = eventLoop->getEvent(fd)->get_mask();
        op = old_mask == MS_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    }
    /* If the fd was already monitored for some event, we need a MOD
     * operation. Otherwise we need an ADD operation. */
    struct epoll_event ee = {0};
    int mask  = add_mask|= old_mask; /* Merge old events */
    if (mask & MS_READABLE) ee.events |= EPOLLIN|EPOLLET;
    if (mask & MS_WRITABLE) ee.events |= EPOLLOUT|EPOLLET;
    ee.data.fd = fd;
    if (epoll_ctl(epollfd,op,fd,&ee) == -1) return MS_ERR;
    return MS_OK;

}

int msPoller::delPollEvent(int fd, int del_mask){
    if(!eventLoop->findEvents(fd))
    {
        return MS_OK;
    }

    struct epoll_event ee = {0}; /* avoid valgrind warning */
    int mask = eventLoop->getEvent(fd)->get_mask()& (~del_mask);
    if (mask & MS_READABLE) ee.events |= EPOLLIN;
    if (mask & MS_WRITABLE) ee.events |= EPOLLOUT;
    ee.data.fd = fd;
    if (mask != MS_NONE)
    {
        epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ee);
    }
    else
    {
        epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ee);
    }
    return MS_OK;
}

int msPoller::poll(int maxWaitTimeM, EventVec *activeEvents){
    int numevents = epoll_wait(epollfd, epEvents.data(), epEvents.capacity(), maxWaitTimeM);
    //printf("eventLoop->get_setsize()=%d\n",eventLoop->get_setsize());
    //printf("    epEventsListSize=%d\n", epEventsListSize);
    //printf("epEvents.capacity ()=%d\n",epEvents.capacity());
    //printf("epollfd=%d\n",epollfd);

    if(numevents>0)
    {
        fillActiveEvents(numevents, activeEvents);
    }
    else if(!numevents)
    {

    }
    else
    {
    }

    return MS_OK;
}

int msPoller::fillActiveEvents(int numevents, EventVec *activeEvents){

    for(int j=0;j<numevents;j++)
    {
        struct epoll_event &e = epEvents[j];
        int fd = e.data.fd;
        if(!eventLoop->findEvents(fd))
        {
            perror("fillActiveChannels: fd not active");
            continue;
        }

        int mask = 0;
        if (e.events & EPOLLIN) mask |= MS_READABLE;
        if (e.events & EPOLLOUT) mask |= MS_WRITABLE;
        if (e.events & EPOLLERR) mask |= MS_WRITABLE|MS_READABLE;
        if (e.events & EPOLLHUP) mask |= MS_WRITABLE|MS_READABLE;
        auto msev = eventLoop->getEvent(fd);
        msev->update_mask(mask);
        activeEvents->push_back(msev);
    }
    return MS_OK;
}
