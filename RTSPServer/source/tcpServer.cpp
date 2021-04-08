#include "tcpServer.h"
#include "msPoller.h"
#include "event.h"
#include <cstring>

#define my_error(x, str) \
do{\
    if(x < 0)\
    {   perror(str); exit(-1);}\
}while(0)

void tcpServer::start()
{
    startup(ip,port);
    std::cout<<"Serverstart up!"<<std::endl;
    eventLoop->loop();
}

int tcpServer::setnonblocking(int fd)
{
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}


std::shared_ptr<clientConnection> tcpServer::createConnection(int sockfd)
{
	return std::make_shared<clientConnection>(shared_from_this(), sockfd);
}

void tcpServer::addConnection(int sockfd, std::shared_ptr<clientConnection> connection)
{
	std::lock_guard<std::mutex> locker(mutex_);
	clientMap.emplace(sockfd, connection);
}

void tcpServer::removeConnection(int sockfd)
{
	std::lock_guard<std::mutex> locker(mutex_);
    eventLoop->deleteEvent(sockfd,MS_READABLE&MS_WRITABLE);
	clientMap.erase(sockfd);

}

int tcpServer::addEvent(int fd, int mask, eventCallback proc,void *clientData)
{
    return eventLoop->addEvent(fd,mask,proc,clientData);
}

int tcpServer::deleteEvent(int fd, int mask)
{
    return eventLoop->deleteEvent(fd,mask);
}

int tcpServer::startup(const std::string& local_ip,int local_port)
{
    //1.create sock
    server_fd = socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK,0);
    my_error(server_fd, "socket error");
    //2,createbind
    struct sockaddr_in local;
    //bzero(&local,sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(local_port);
    local.sin_addr.s_addr = inet_addr(local_ip.c_str());
    int ret = bind(server_fd,(struct sockaddr *)&local,sizeof(local)) ;
    my_error(ret, "bind error");
    //3.listen
    ret = listen(server_fd,10);
    my_error(ret, "listen error");

    ret = eventLoop->addEvent(server_fd,MS_READABLE,std::bind(&tcpServer::accpetConn, shared_from_this(), std::placeholders::_1),nullptr);
    my_error(ret, "addEvent error");
    return MS_OK;
};

int tcpServer::accpetConn(int fd)
{
    int cfd = accept4(server_fd, NULL, NULL, SOCK_NONBLOCK);
    my_error(cfd, "accept4");
    printf("accept new fd = %d\n",cfd);
    std::shared_ptr<clientConnection> p3 = std::move(createConnection(cfd));
    addConnection(cfd,p3);
    int ret = eventLoop->addEvent(cfd,MS_READABLE,std::bind(&clientConnection::incomingRequestHandler, p3, std::placeholders::_1),nullptr);
    my_error(ret, "addEvent");
    return MS_OK;
}



/*
void tcpServer::readdata(int fd)
{
    printf("in readdata\n");
    auto event = eventLoop->getEvent(fd);
    if(event==nullptr)
        my_error(MS_ERR, "nullptr");
    event->get_rbuff()->clear();

    int nread,nbytes = 0;
    //在epoll的ET模式下，正确的读写方式为:
    //读：只要可读，就一直读，直到返回0，或者 errno = EAGAIN 写:只要可写，就一直写，直到数据发送完，或者 errno = EAGAIN。
    while ((nread = read(fd, event->get_rbuff()->get_data() + nbytes, event->get_rbuff()->get_length() - nbytes)) > 0)
    {
        if (nread == -1 && errno != EAGAIN)
        {
            perror("read error");
        }
        nbytes += nread;
    }

    if(nbytes > 0)
    {
        printf("nbytes=%d\n",nbytes);
        std::cout<< event->get_rbuff()->get_data() << std::endl;
        memcpy(event->get_wbuff()->get_data(),event->get_rbuff()->get_data(),nbytes);
        event->get_wbuff()->set_dataLength(nbytes);
        int nwrite = 0;
        nwrite = write(fd, event->get_wbuff()->get_data(), event->get_wbuff()->get_dataLength());
        if (nwrite < nbytes)
        {
            if (nwrite == -1 && errno != EAGAIN)
            {
                perror("write error");
                eventLoop->deleteEvent(fd,MS_WRITABLE);
            }
            else
            {
                  ////删除前nwrite个元素
                int ret = eventLoop->addEvent(fd,MS_WRITABLE,std::bind(&tcpServer::senddata, this, std::placeholders::_1),nullptr);
                my_error(ret, "addEvent");
            }
         }
        //需要向socket写数据的时候，直接调用write或者send发送数据。如果返回EAGAIN，把socket加入epoll，在epoll的驱动下写数据，全部数据发送完毕后，再移出epoll。
        //这种方式的优点是：数据不多的时候可以避免epoll的事件处理，提高效率。

    }
    else if(nbytes == 0)
    {
        printf("close fd\n");
        eventLoop->deleteEvent(fd,MS_READABLE);
        close(fd);
    }
    else
    {
        printf("read error\n");
        eventLoop->deleteEvent(fd,MS_READABLE);
        close(fd);
    }
}


void tcpServer::senddata(int fd)
{
    printf("in senddata\n");
    auto event = eventLoop->getEvent(fd);
    if(event==nullptr)
        my_error(MS_ERR, "nullptr");

    int nwrite, data_size = event->get_rbuff()->get_length();
    int n = data_size;
    while (n > 0)
    {
        nwrite = write(fd, event->get_rbuff()->get_data() + data_size - n, n);
        if (nwrite < n)
        {
            if (nwrite == -1 && errno != EAGAIN)
            {
                perror("write error");
                eventLoop->deleteEvent(fd,MS_WRITABLE);
                close(fd);
            }
            break;
        }
        n -= nwrite;
    }
}

*/
