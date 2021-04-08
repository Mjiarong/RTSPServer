#pragma once

#include "EventLoop.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unordered_map>
#include <mutex>
#include "tcpClientConnection.h"

class clientConnection;

class tcpServer : public std::enable_shared_from_this<tcpServer>
{
    using ConnectionMap = std::unordered_map<int, std::shared_ptr<clientConnection>>;
    using eventCallback = std::function<int(int)>;
public:
    tcpServer(const std::string& local_ip,int local_port)
    : ip(local_ip),
    port(local_port),
    eventLoop(new EventLoop)
    {

    }

    virtual ~tcpServer()
    {

    }


    void start();

    int deleteEvent(int fd, int mask);
    int addEvent(int fd, int mask, eventCallback proc,void *clientData);
    virtual void addConnection(int sockfd, std::shared_ptr<clientConnection> connection);
	virtual void removeConnection(int sockfd);
protected:
    //friend class clientConnection;
    virtual std::shared_ptr<clientConnection> createConnection(int sockfd);
    int startup(const std::string& local_ip,int local_port);
    ConnectionMap clientMap;
    std::string ip;
    int port;
    std::unique_ptr<EventLoop> eventLoop;
    int server_fd;
	std::mutex mutex_;

private:
    int accpetConn(int fd);
    void readdata(int fd);
    void senddata(int fd);
    int setnonblocking(int fd);
    //std::map<int, std::shared_ptr<Handler>> handleList;
};

