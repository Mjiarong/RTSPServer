#pragma once

#include "tcpServer.h"
#include "buffer.h"
#include "socketUtil.h"
#include <mutex>
#include <memory>

#define MAX_BUFFLEN 1024

class tcpServer;

class clientConnection : public std::enable_shared_from_this<clientConnection>
{
public:
    clientConnection(std::weak_ptr<tcpServer>, int clientSocket);
	virtual ~clientConnection();

	void Disconnect();

    msBuffer* rbuff() {
        return &rbuff_;
    }

    msBuffer* wbuff() {
        return &wbuff_;
    }

    int sock_fd(){
        return sock_fd_;
    }

    std::string get_peer_ip()
    {
        return socketUtil::GetPeerIp(sock_fd_);
    }

protected:
	friend class tcpServer;

    virtual void closeSockets();
    virtual int incomingRequestHandler(int fd);
    virtual int handleRequestBytes(int bytesRead);

    std::weak_ptr<tcpServer> own_server;
    int sock_fd_;
    msBuffer rbuff_;
    msBuffer wbuff_;

private:
	void Close();

	std::mutex mutex_;

};
