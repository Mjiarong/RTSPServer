#pragma once

#include "EventLoop.h"
#include "buffer.h"
#include <functional>
#include <iostream>


class msEvent
{
    //using eventCallback = std::function<int(void*)>;

    using eventCallback = std::function<int(int)>;

    private:
    EventLoop *ownerLoop; // ownerLoop_ owns this event
    int fd;
    int mask;
    int events;
    void *clientData;
    //char buff[MAX_BUFFLEN];
    //msBuffer rbuff;
    //msBuffer wbuff;

    eventCallback readCallback;
    eventCallback writeCallback;
    eventCallback errorCallback;

    public:
    msEvent(EventLoop *loop, int fd, int mask, void *clientData);
    ~msEvent();

    int get_fd() const {
        return fd;
    }

    int get_mask() const {
        return mask;
    }

    //msBuffer* get_rbuff() {
    //    return &rbuff;
    //}

    //msBuffer* get_wbuff() {
    //    return &wbuff;
    //}


    void update_mask(int m){
        mask |= m;
    }

    void delete_mask(int m){
        mask &= (~m);
    }

    void set_clientData(void* data){
        clientData = data;
    }



    void setErrorCallback(const eventCallback cb){
        errorCallback = cb;
    }

    void setReadCallback(const eventCallback cb){
        readCallback = cb;
    }
    void setWriteCallback(const eventCallback cb){
        writeCallback = cb;
    }

    void handleEvents();
    //void update();
};
