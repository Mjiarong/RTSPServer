#include "tcpClientConnection.h"
#include <unistd.h>
#include <iostream>
#include <cstring>

clientConnection::clientConnection(std::weak_ptr<tcpServer> ownServer, int clientSocket)
  :own_server(ownServer),
  sock_fd_(clientSocket),
  rbuff_(MAX_BUFFLEN),
  wbuff_(MAX_BUFFLEN)
{

}

void clientConnection::closeSockets()
{

  if (sock_fd_>= 0) close(sock_fd_);
  sock_fd_ = -1;
  return ;
}

clientConnection::~clientConnection()
{
    closeSockets();
}


int clientConnection::incomingRequestHandler(int fd)
{
    int nread,nbytes = 0;
    //在epoll的ET模式下，正确的读写方式为:
    //读：只要可读，就一直读，直到返回0，或者 errno = EAGAIN 写:只要可写，就一直写，直到数据发送完，或者 errno = EAGAIN。
    while ((nread = read(sock_fd_, rbuff_.data() + nbytes, rbuff_.capacity() - nbytes)) > 0)
    {
        if (nread == -1 && errno != EAGAIN)
        {
            perror("read error");
        }
        nbytes += nread;
    }
    return handleRequestBytes(nbytes);
}

int clientConnection::handleRequestBytes(int bytesRead)
{
    std::shared_ptr<tcpServer> server = own_server.lock();
	if (!server) {
		return MS_ERR;
	}

    if(bytesRead > 0)
    {
        memcpy(wbuff_.data(),rbuff_.data(),bytesRead);
        wbuff_.set_dataLength(bytesRead);


        int nwrite = 0;
        nwrite = write(sock_fd_, wbuff_.data(), wbuff_.dataLength());
        if (nwrite < bytesRead)
        {
            if (nwrite == -1 && errno != EAGAIN)
            {
                perror("write error");
                server->deleteEvent(sock_fd_,MS_WRITABLE);
            }
            else
            {
                ////删除前nwrite个元素
                int ret = server->addEvent(sock_fd_,MS_WRITABLE,std::bind(&clientConnection::incomingRequestHandler, shared_from_this(), std::placeholders::_1),nullptr);
            }
         }
        //需要向socket写数据的时候，直接调用write或者send发送数据。如果返回EAGAIN，把socket加入epoll，在epoll的驱动下写数据，全部数据发送完毕后，再移出epoll。
        //这种方式的优点是：数据不多的时候可以避免epoll的事件处理，提高效率。

    }
    else if(bytesRead == 0)
    {
        server->deleteEvent(sock_fd_,MS_READABLE);
        server->removeConnection(sock_fd_);
    }
    else
    {
        server->deleteEvent(sock_fd_,MS_READABLE);
        server->removeConnection(sock_fd_);
    }
    return MS_OK;
}
