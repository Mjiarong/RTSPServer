#include "rtspClientConnection.h"
#include "rtspServer.h"
#include "rtpSession.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <sys/socket.h>

rtspClientConnection::rtspClientConnection(std::weak_ptr<tcpServer> ownServer, int clientSocket)
    :clientConnection(ownServer, clientSocket)
{

}

rtspClientConnection::~rtspClientConnection()
{

}

std::shared_ptr<rtpSession> rtspClientConnection::createClientSession()
{
    rtp_session_ = std::make_shared<rtpSession>(shared_from_this());
    return rtp_session_;
}

int rtspClientConnection::sendRtspMessage(int fd)
{
    std::shared_ptr<tcpServer> server = own_server.lock();
    if (!server) {
        perror("own_server ont found\n");
        return MS_ERR;
    }

    int nwrite, data_size = wbuff_.dataLength();
    int n = data_size;
    while (n > 0)
    {
        nwrite = write(fd, wbuff_.data() + data_size - n, n);
        if (nwrite < n)
        {
            if (nwrite == -1 && errno != EAGAIN)
            {
                perror("write error");
                server->deleteEvent(fd,MS_WRITABLE);
                close(fd);
                return nwrite;
            }
            break;
        }
        n -= nwrite;
    }
    server->deleteEvent(fd,MS_WRITABLE);
    return data_size-n;
}

bool rtspClientConnection::parseRTSPURL(char const* url)
{
    char* username,*password;
    do {
        // Parse the URL as "rtsp://[<username>[:<password>]@]<server-address-or-name>[:<port>][/<stream-name>]"
        char const* prefix = "rtsp://";
        unsigned const prefixLength = 7;
        if (strncasecmp(url, prefix, prefixLength) != 0) {
            break;
        }

        unsigned const parseBufferSize = 100;
        char parseBuffer[parseBufferSize];
        char const* from = &url[prefixLength];

        // Check whether "<username>[:<password>]@" occurs next.
        // We do this by checking whether '@' appears before the end of the URL, or before the first '/'.
        username = password = NULL; // default return values
        char const* colonPasswordStart = NULL;
        char const* p;
        for (p = from; *p != '\0' && *p != '/'; ++p) {
          if (*p == ':' && colonPasswordStart == NULL) {
        colonPasswordStart = p;
          } else if (*p == '@') {
        // We found <username> (and perhaps <password>).  Copy them into newly-allocated result strings:
        if (colonPasswordStart == NULL) colonPasswordStart = p;

        char const* usernameStart = from;
        unsigned usernameLen = colonPasswordStart - usernameStart;
        username = new char[usernameLen + 1] ; // allow for the trailing '\0'
        memcpy(username, usernameStart, usernameLen);
        username[usernameLen + 1] = '\0';
        rtsp_url_info_.username = username;
        delete[]username;

        char const* passwordStart = colonPasswordStart;
        if (passwordStart < p) ++passwordStart; // skip over the ':'
        unsigned passwordLen = p - passwordStart;
        password = new char[passwordLen + 1]; // allow for the trailing '\0'
        memcpy(password, passwordStart, passwordLen);
        password[usernameLen + 1] = '\0';
        rtsp_url_info_.password = password;
        delete[]password;

        from = p + 1; // skip over the '@'
        break;
          }
        }

        // Next, parse <server-address-or-name>
        char* to = &parseBuffer[0];
        unsigned i;
        for (i = 0; i < parseBufferSize; ++i) {
          if (*from == '\0' || *from == ':' || *from == '/') {
        // We've completed parsing the address
        *to = '\0';
        break;
          }
          *to++ = *from++;
        }
        if (i == parseBufferSize) {
          printf("URL is too long");
          break;
        }

        rtsp_url_info_.ip = parseBuffer;
        rtsp_url_info_.port = 554; // default value
        char nextChar = *from;
        if (nextChar == ':') {
            int portNumInt;
            if (sscanf(++from, "%d", &portNumInt) != 1) {
                printf("No port number follows\n");
                break;
            }
            if (portNumInt < 1 || portNumInt > 65535) {
                printf("Bad port number\n");
                break;
            }
            rtsp_url_info_.port = portNumInt;
            while (*from >= '0' && *from <= '9') ++from; // skip over port number
        }


        char const* preSuffixStart  = ++from;
        // The remainder of the URL is the suffix:
        char const* suffixStart = nullptr;
        for (p = from; *p != '\0'; ++p)
        {
            if (*p == '/') {
            suffixStart = ++p;// skip '\0'
          }
        }

        if(suffixStart != nullptr)
        {
            unsigned char preSuffixLen = suffixStart - preSuffixStart - 1;// skip '\0'
            auto preSuffix = new char[preSuffixLen + 1] ; // allow for the trailing '\0'
            if(preSuffix==nullptr)
            {
                perror("preSuffix==nullptr\n");
                return false;
            }
            memcpy(preSuffix, preSuffixStart, preSuffixLen);
            preSuffix[preSuffixLen] = '\0';
            rtsp_url_info_.preSuffix = preSuffix;
            rtsp_url_info_.suffix = suffixStart;
        }
        else
        {
            rtsp_url_info_.preSuffix = preSuffixStart;
        }
        return true;
    } while (0);

    return false;
}

int rtspClientConnection::handleCmdOption(unsigned int cseq)
{
    std::shared_ptr<tcpServer> tmp = own_server.lock();
	if (!tmp) {
		return MS_ERR;
	}

    std::shared_ptr<rtspServer> server = std::dynamic_pointer_cast<rtspServer>(tmp);

    wbuff_.clear();
    sprintf(wbuff_.data(), "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN\r\n"
                    "\r\n",
                    cseq);

    wbuff_.set_dataLength(strlen(wbuff_.data()));
    int ret = server->addEvent(sock_fd_,MS_WRITABLE,std::bind(&rtspClientConnection::sendRtspMessage, std::dynamic_pointer_cast<rtspClientConnection>(shared_from_this()),std::placeholders::_1),nullptr);
    return MS_OK;
}

int rtspClientConnection::handleCmdDescribe(unsigned int  cseq, char* url)
{
    std::shared_ptr<tcpServer> tmp = own_server.lock();
	if (!tmp) {
		return MS_ERR;
	}

    std::shared_ptr<rtspServer> server = std::dynamic_pointer_cast<rtspServer>(tmp);
    std::shared_ptr<mediaSession>  mediaSessionPtr = server->lookMediaSession(rtsp_url_info_.preSuffix);
	if(!mediaSessionPtr)  {
        perror("no mediaSessionPtr");
		return MS_ERR;
	}

    rtp_session_ = createClientSession();
    char sdp[500];
    char localIp[100];
    sscanf(url, "rtsp://%[^:]:", localIp);
    int offest = sprintf(sdp, "v=0\r\n"
                 "o=- 9%ld 1 IN IP4 %s\r\n"
                 "t=0 0\r\n"
                 "a=control:*\r\n",
                 time(NULL), localIp);

    auto source = mediaSessionPtr->lookSource(channel_0);
    if (source)
    {
        offest += sprintf(sdp + offest, "m=video 0 RTP/AVP 96\r\n"
                     "a=rtpmap:96 H264/90000\r\n"
                     "a=control:track0\r\n");
    }
    source = mediaSessionPtr->lookSource(channel_1);
    if (source)
    {
        sprintf(sdp + offest, "m=audio 0 RTP/AVP 97\r\n"
                     "a=rtpmap:97 mpeg4-generic/44100/2\r\n"
                     "a=fmtp:97 SizeLength=13;\r\n"
                     "a=control:track1\r\n");
    }

    wbuff_.clear();
    sprintf(wbuff_.data(), "RTSP/1.0 200 OK\r\nCSeq: %d\r\n"
                    "Content-Base: %s\r\n"
                    "Content-type: application/sdp\r\n"
                    "Content-length: %d\r\n\r\n"
                    "%s",
                    cseq,
                    url,
                    strlen(sdp),
                    sdp);

    wbuff_.set_dataLength(strlen(wbuff_.data()));
    int ret = server->addEvent(sock_fd_,MS_WRITABLE,std::bind(&rtspClientConnection::sendRtspMessage, std::dynamic_pointer_cast<rtspClientConnection>(shared_from_this()),std::placeholders::_1),nullptr);
    return MS_OK;
}

int rtspClientConnection::handleCmdSetup(unsigned int  cseq, unsigned int  clientRtpPort)
{
    std::shared_ptr<tcpServer> tmp = own_server.lock();
	if (!tmp) {
		return MS_ERR;
	}

    std::shared_ptr<rtspServer> server = std::dynamic_pointer_cast<rtspServer>(tmp);
    std::shared_ptr<mediaSession>  mediaSessionPtr = server->lookMediaSession(rtsp_url_info_.preSuffix);
	if(!mediaSessionPtr)  {
		return MS_ERR;
	}

    int channel;
    if(rtsp_url_info_.suffix.compare("track0") ==0)
    {
        channel = channel_0;  //video
        rtp_session_->SetSendFrameCallback(std::bind(&mediaSession::rtpSendH264Frame, mediaSessionPtr, std::placeholders::_1, std::placeholders::_2),channel);
    }
    else if(rtsp_url_info_.suffix.compare("track1") ==0)
    {
        channel = channel_1;
        rtp_session_->SetSendFrameCallback(std::bind(&mediaSession::rtpSendAACFrame, mediaSessionPtr, std::placeholders::_1, std::placeholders::_2),channel);
    }
    rtp_session_->rtpSessionSetup(channel,clientRtpPort,sock_fd_);
    mediaSessionPtr->addClient(sock_fd_,rtp_session_);


    wbuff_.clear();
    sprintf(wbuff_.data(), "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n"
                    "Session: 66334873\r\n"
                    "\r\n",
                    cseq,
                    clientRtpPort,
                    clientRtpPort+1,
                    server->rtp_port(),
                    server->rtcp_port());

    wbuff_.set_dataLength(strlen(wbuff_.data()));
    int ret = server->addEvent(sock_fd_,MS_WRITABLE,std::bind(&rtspClientConnection::sendRtspMessage, std::dynamic_pointer_cast<rtspClientConnection>(shared_from_this()),std::placeholders::_1),nullptr);

    return ret;
}

int rtspClientConnection::handleCmdPlay(unsigned int  cseq)
{
    std::shared_ptr<tcpServer> tmp = own_server.lock();
	if (!tmp) {
		return MS_ERR;
	}

    std::shared_ptr<rtspServer> server = std::dynamic_pointer_cast<rtspServer>(tmp);
    rtp_session_->SetConnectionState(rtp_session_->STATE_PLAY);
    wbuff_.clear();
    sprintf(wbuff_.data(), "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Range: npt=0.000-\r\n"
                    "Session: 66334873; timeout=60\r\n\r\n",
                    cseq);

    wbuff_.set_dataLength(strlen(wbuff_.data()));
    int ret = server->addEvent(sock_fd_,MS_WRITABLE,std::bind(&rtspClientConnection::sendRtspMessage, std::dynamic_pointer_cast<rtspClientConnection>(shared_from_this()),std::placeholders::_1),nullptr);
    return MS_OK;
}

int rtspClientConnection::handleCmdPause(unsigned int  cseq)
{
    std::shared_ptr<tcpServer> tmp = own_server.lock();
	if (!tmp) {
		return MS_ERR;
	}

    std::shared_ptr<rtspServer> server = std::dynamic_pointer_cast<rtspServer>(tmp);
    std::shared_ptr<mediaSession>  mediaSessionPtr = server->lookMediaSession(rtsp_url_info_.preSuffix);
	if(!mediaSessionPtr)  {
		return MS_ERR;
	}

    rtp_session_->SetConnectionState(rtp_session_->STATE_PAUSE);
	wbuff_.clear();
    sprintf(wbuff_.data(), "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Session: 66334873\r\n",
                    cseq);

    wbuff_.set_dataLength(strlen(wbuff_.data()));

    int ret = server->addEvent(sock_fd_,MS_WRITABLE,std::bind(&rtspClientConnection::sendRtspMessage, std::dynamic_pointer_cast<rtspClientConnection>(shared_from_this()),std::placeholders::_1),nullptr);
    return MS_OK;
}


int rtspClientConnection::handleCmdTeardown(unsigned int  cseq)
{
    std::shared_ptr<tcpServer> tmp = own_server.lock();
	if (!tmp) {
		return MS_ERR;
	}

    std::shared_ptr<rtspServer> server = std::dynamic_pointer_cast<rtspServer>(tmp);
    std::shared_ptr<mediaSession>  mediaSessionPtr = server->lookMediaSession(rtsp_url_info_.preSuffix);
	if(!mediaSessionPtr)  {
		return MS_ERR;
	}

    rtp_session_->SetConnectionState(rtp_session_->STATE_CLOSE);
	wbuff_.clear();
	time_t timep;
    time (&timep);

    sprintf(wbuff_.data(), "RTSP/1.0 200 OK\r\n"
                    "CSeq: %d\r\n"
                    "Date: %s\r\n"
                    "Session: 66334873\r\n",
                    cseq,ctime(&timep));

    wbuff_.set_dataLength(strlen(wbuff_.data()));

    int ret = server->addEvent(sock_fd_,MS_WRITABLE,std::bind(&rtspClientConnection::sendRtspMessage, std::dynamic_pointer_cast<rtspClientConnection>(shared_from_this()),std::placeholders::_1),nullptr);
    mediaSessionPtr->removeClient(sock_fd_);
    rtp_session_ = nullptr;
    server->removeConnection(sock_fd_);

    return MS_OK;
}

char* rtspClientConnection::getLineFromBuf(char* buf, char* line)
{
    while(*buf != '\n')
    {
        *line = *buf;
        line++;
        buf++;
    }

    *line = '\n';
    ++line;
    *line = '\0';

    ++buf;
    return buf;
}

int rtspClientConnection::handleRequestBytes(int bytesRead)
{
    std::shared_ptr<tcpServer> server = own_server.lock();
	if (!server) {
		return MS_ERR;
	}

    if(bytesRead == 0)
    {
        server->deleteEvent(sock_fd_,MS_READABLE);
        server->removeConnection(sock_fd_);
        return MS_ERR;
    }
    else if (bytesRead < 0)
    {
        printf("read error\n");
        server->deleteEvent(sock_fd_,MS_READABLE);
        server->removeConnection(sock_fd_);
        return MS_ERR;
    }

    char method[40];
    char url[100];
    char version[40];
    unsigned int cseq;
    unsigned int clientRtpPort, clientRtcpPort;
    char line[400];

    //std::cout<< rbuff_.get_data() << std::endl;
    /* 解析方法 */
    char *bufPtr = getLineFromBuf(rbuff_.data(), line);
    if(sscanf(line, "%s %s %s\r\n", method, url, version) != 3)
    {
        printf("parse err\n");
        return MS_ERR;
    }

    /* 解析序列号 */
    bufPtr = getLineFromBuf(bufPtr, line);
    if(sscanf(line, "CSeq: %d\r\n", &cseq) != 1)
    {
        printf("parse err\n");
        return MS_ERR;
    }

    parseRTSPURL(url);
    /* 如果是SETUP，那么就再解析client_port */
    if(!strcmp(method, "SETUP"))
    {
        while(1)
        {
            bufPtr = getLineFromBuf(bufPtr, line);
            if(!strncmp(line, "Transport:", strlen("Transport:")))
            {
                sscanf(line, "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n",
                                &clientRtpPort, &clientRtcpPort);
                break;
            }
        }
    }


    if(!strcmp(method, "OPTIONS"))
    {
        if(handleCmdOption(cseq))
        {
            printf("failed to handle options\n");
            return MS_ERR;
        }
    }
    else if(!strcmp(method, "DESCRIBE"))
    {
        if(handleCmdDescribe(cseq, url))
        {
            printf("failed to handle describe\n");
            return MS_ERR;
        }
    }
    else if(!strcmp(method, "SETUP"))
    {
        if(handleCmdSetup(cseq, clientRtpPort))
        {
            printf("failed to handle setup\n");
            return MS_ERR;
        }
    }
    else if(!strcmp(method, "PLAY"))
    {
        if(handleCmdPlay(cseq))
        {
            printf("failed to handle play\n");
            return MS_ERR;
        }
    }
        else if(!strcmp(method, "PAUSE"))
    {
        if(handleCmdPause(cseq))
        {
            printf("failed to handle play\n");
            return MS_ERR;
        }
    }
    else if(!strcmp(method, "TEARDOWN"))
    {
        if(handleCmdTeardown(cseq))
        {
            printf("failed to handle Teardown\n");
            return MS_ERR;
        }
    }
    else
    {
        return MS_ERR;
    }

    return MS_OK;

}
