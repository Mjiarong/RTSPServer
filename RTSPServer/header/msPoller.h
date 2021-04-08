#pragma once

#include <vector>
#include <map>
#include <memory>

class msEvent;
class EventLoop;

class msPoller
{
    using EventVec = std::vector<std::shared_ptr<msEvent>>;
    using EventMap = std::map<int, std::shared_ptr<msEvent>>; // fd => msEvent *
private:
    int epollfd;
    EventMap events;
    EventLoop *eventLoop;
    int epEventsListSize = 64;
    std::vector<struct epoll_event> epEvents;


public:
    msPoller(EventLoop *loop);
    ~msPoller();

    int addPollEvent(int fd, int add_mask);
    int delPollEvent(int fd, int del_mask);

    int poll(int maxWaitTimeM, EventVec *activeEvents);
    int fillActiveEvents(int numevents, EventVec *activeEvents);
};
