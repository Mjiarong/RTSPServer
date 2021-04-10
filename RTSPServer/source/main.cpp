#include  "tcpServer.h"
#include  "rtspServer.h"
#include  "h264Source.h"
#include  "aacSource.h"
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>



void SendFrameThread(std::shared_ptr<rtspServer> rtsp_server, int session_id,int channel);

int main()
{
    std::shared_ptr<rtspServer> server = std::make_shared<rtspServer>("192.168.0.109",8554);
    mediaSession *session = mediaSession::CreateNew("live");
    session->addSource(channel_0, h264Source::CreateNew("../../mediaFile/test.h264"));//use absolute path is better
	session->addSource(channel_1, aacSource::CreateNew("../../mediaFile/test.aac"));
	int session_id = server->addSession(std::move(session));
    std::thread send_video_thread(SendFrameThread, server, session_id,channel_0);
	std::thread send_audio_thread(SendFrameThread, server, session_id,channel_1);
	send_video_thread.detach();
    send_audio_thread.detach();
    server->start();
}

void SendFrameThread(std::shared_ptr<rtspServer> rtsp_server, int session_id,int channel)
{
    while(1)
    {
        std::shared_ptr<mediaSession> sessionPtr = rtsp_server->lookMediaSession(session_id);
        if (sessionPtr!=nullptr && sessionPtr->GetNumClient()!=0)
        {
            sessionPtr->sendFrame(channel);
        }
    }

}

