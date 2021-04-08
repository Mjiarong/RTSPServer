/* EventLoop.h */
#pragma once

#include <vector>
#include <map>
#include <memory>
#include <functional>

#define MS_OK 0
#define MS_ERR -1

#define MS_NONE 0       /* No events registered. */
#define MS_READABLE 1   /* Fire when descriptor is readable. */
#define MS_WRITABLE 2   /* Fire when descriptor is writable. */
#define MS_BARRIER 4    /* With WRITABLE, never fire the event if the
                           READABLE event already fired in the same event
                           loop iteration. Useful when you want to persist
                           things to disk before sending replies, and want
                           to do that in a group fashion. */
class msEvent;
class msPoller;

// core of Reactor
class EventLoop
{
    using eventCallback = std::function<int(int)>;
    using EventVec = std::vector<std::shared_ptr<msEvent>>;
    using EventMap = std::map<int, std::shared_ptr<msEvent>>; //key:fd => msEvent *
    private:
    EventVec activeEvents;
    EventMap regEvents;/* Registered events */
    bool stop; // should be atomatic
    bool isLoopping;
    int setsize = 1024; /* max number of file descriptors tracked */
    int maxWaitTimeM;
    std::unique_ptr<msPoller> pollerPtr;


    public:
    EventLoop();
    ~EventLoop();
    // precondition:
    // 1. should be IO thread
    // 2. cannot call loop() repeately
    void loop();

    int get_setsize()
    {
        return setsize;
    }

    bool findEvents(int fd);
    std::shared_ptr<msEvent> getEvent(int fd)
    {

        return findEvents(fd)?regEvents[fd]:nullptr;
    }
    int deleteEvent(int fd, int mask);
    int addEvent(int fd, int mask, eventCallback proc,void *clientData);
    void stopLoopping() {
        stop = true;
    }
};
