#include  "tcpServer.h"
#include  "rtspServer.h"
#include  "h264Source.h"
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>



void SendFrameThread(std::shared_ptr<rtspServer> rtsp_server, int session_id);

int main()
{
    std::shared_ptr<rtspServer> server = std::make_shared<rtspServer>("192.168.0.109",8554);
    mediaSession *session = mediaSession::CreateNew("live");
    session->addSource(channel_0, h264Source::CreateNew("test.h264"));//use absolute path is better
	//session->AddSource(xop::channel_1, xop::AACSource::CreateNew(2));.
	int session_id = server->addSession(session);
	std::thread thread(SendFrameThread, server, session_id);
	thread.detach();

    server->start();
}

void SendFrameThread(std::shared_ptr<rtspServer> rtsp_server, int session_id)
{
    while(1)
    {
        std::shared_ptr<mediaSession> sessionPtr = rtsp_server->lookMediaSession(session_id);
        if (sessionPtr!=nullptr && sessionPtr->GetNumClient()!=0)
        {
            sessionPtr->sendFrame(channel_0);
        }
        usleep(20000);
    }

}

