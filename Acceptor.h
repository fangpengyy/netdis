#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include "Utils.h"
#include <functional>


class EventObj;
class EventLoop;


typedef std::function<void(int sockfd, struct sockaddr_in& addr)> NewConnectionFunc;

class Acceptor
{
public:
	Acceptor(EventLoop* apEventLoop);
	~Acceptor();

	int Listen(int listenFd, struct sockaddr_in& addr);
	int Accept(struct sockaddr_in& cltAddr);

private:
	//void OnReadEvent(Timestamp now);

private:
	EventObj* _eventObj;
	EventLoop* _pEventLoop;
	int _listenFd;
	int _idleFd;

//	NewConnectionFunc _OnConnection;
};




#endif
