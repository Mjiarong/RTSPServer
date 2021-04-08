#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <net/route.h>
#include <netdb.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>
#define SOCKET int
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR    (-1)
#include <string>
#include <cstdint>
#include <cstring>

class socketUtil
{
public:
    static bool Bind(SOCKET sockfd, std::string ip, uint16_t port);
    static void SetNonBlock(SOCKET fd);
    static void SetBlock(SOCKET fd, int write_timeout=0);
    static void SetReuseAddr(SOCKET fd);
    static void SetReusePort(SOCKET sockfd);
    static void SetNoDelay(SOCKET sockfd);
    static void SetKeepAlive(SOCKET sockfd);
    static void SetNoSigpipe(SOCKET sockfd);
    static void SetSendBufSize(SOCKET sockfd, int size);
    static void SetRecvBufSize(SOCKET sockfd, int size);
    static std::string GetPeerIp(SOCKET sockfd);
    static std::string GetSocketIp(SOCKET sockfd);
    static int GetSocketAddr(SOCKET sockfd, struct sockaddr_in* addr);
    static uint16_t GetPeerPort(SOCKET sockfd);
    static int GetPeerAddr(SOCKET sockfd, struct sockaddr_in *addr);
    static void Close(SOCKET sockfd);
    static bool Connect(SOCKET sockfd, std::string ip, uint16_t port, int timeout=0);
    static int CreateUdpSocket();
};
