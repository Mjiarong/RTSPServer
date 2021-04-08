#include "socketUtil.h"
#include <iostream>

bool socketUtil::Bind(SOCKET sockfd, std::string ip, uint16_t port)
{
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if(::bind(sockfd, (struct sockaddr*)&addr, sizeof addr) == SOCKET_ERROR) {
        return false;
    }

    return true;
}

void socketUtil::SetNonBlock(SOCKET fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void socketUtil::SetBlock(SOCKET fd, int write_timeout)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags&(~O_NONBLOCK));
    if(write_timeout > 0)
    {
#ifdef SO_SNDTIMEO
    struct timeval tv = {write_timeout/1000, (write_timeout%1000)*1000};
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof tv);
#endif
	}
}

void socketUtil::SetReuseAddr(SOCKET sockfd)
{
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof on);
}

void socketUtil::SetReusePort(SOCKET sockfd)
{
#ifdef SO_REUSEPORT
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&on, sizeof(on));
#endif
}

void socketUtil::SetNoDelay(SOCKET sockfd)
{
#ifdef TCP_NODELAY
    int on = 1;
    int ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
#endif
}

void socketUtil::SetKeepAlive(SOCKET sockfd)
{
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));
}

void socketUtil::SetNoSigpipe(SOCKET sockfd)
{
#ifdef SO_NOSIGPIPE
    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, (char *)&on, sizeof(on));
#endif
}

void socketUtil::SetSendBufSize(SOCKET sockfd, int size)
{
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
}

void socketUtil::SetRecvBufSize(SOCKET sockfd, int size)
{
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size));
}

std::string socketUtil::GetPeerIp(SOCKET sockfd)
{
    struct sockaddr_in addr = { 0 };
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0)
    {
        return inet_ntoa(addr.sin_addr);
    }
    return "0.0.0.0";
}

std::string socketUtil::GetSocketIp(SOCKET sockfd)
{
    struct sockaddr_in addr = {0};
    char str[INET_ADDRSTRLEN] = "127.0.0.1";
    if (GetSocketAddr(sockfd, &addr) == 0) {
        inet_ntop(AF_INET, &addr.sin_addr, str, sizeof(str));
    }
    return str;
}

int socketUtil::GetSocketAddr(SOCKET sockfd, struct sockaddr_in* addr)
{
    socklen_t addrlen = sizeof(struct sockaddr_in);
    return getsockname(sockfd, (struct sockaddr*)addr, &addrlen);
}

uint16_t socketUtil::GetPeerPort(SOCKET sockfd)
{
    struct sockaddr_in addr = { 0 };
    socklen_t addrlen = sizeof(struct sockaddr_in);
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addrlen) == 0)
    {
        return ntohs(addr.sin_port);
    }
    return 0;
}

int socketUtil::GetPeerAddr(SOCKET sockfd, struct sockaddr_in *addr)
{
    socklen_t addrlen = sizeof(struct sockaddr_in);
    return getpeername(sockfd, (struct sockaddr *)addr, &addrlen);
}

void socketUtil::Close(SOCKET sockfd)
{
    ::close(sockfd);
}

bool socketUtil::Connect(SOCKET sockfd, std::string ip, uint16_t port, int timeout)
{
	bool is_connected = true;

	if (timeout > 0) {
		socketUtil::SetNonBlock(sockfd);
	}

	struct sockaddr_in addr = { 0 };
	socklen_t addrlen = sizeof(addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());

	if (::connect(sockfd, (struct sockaddr*)&addr, addrlen) == SOCKET_ERROR) {
		if (timeout > 0) {
            is_connected = false;
			fd_set fd_write;
			FD_ZERO(&fd_write);
			FD_SET(sockfd, &fd_write);
			struct timeval tv = { timeout / 1000, timeout % 1000 * 1000 };
			select((int)sockfd + 1, NULL, &fd_write, NULL, &tv);
			if (FD_ISSET(sockfd, &fd_write)) {
                is_connected = true;
			}
			socketUtil::SetBlock(sockfd);
		}
		else {
            is_connected = false;
		}
	}

	return is_connected;
}

int socketUtil::CreateUdpSocket()
{
    int sockfd;
    int on = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
        return -1;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

    return sockfd;
}


