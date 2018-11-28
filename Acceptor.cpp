#include "Acceptor.h"
#include "Utils.h"
#include "NetSock.h"


Acceptor::Acceptor(EventLoop* apEventLoop)
{

	_idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);


}

Acceptor::~Acceptor()
{

}

int Acceptor::Listen(int listenFd, struct sockaddr_in& addr)
{
	_listenFd = listenFd;
	int liRet = ::bind(listenFd, (struct sockaddr*)&addr, static_cast<socklen_t>(sizeof addr));
	if (liRet < 0)
	{
		LOG_ERR("bind err=%d", errno);
		return -1;
	}

	int ret = ::listen(_listenFd, 10240);
	if (ret < 0)
	{
		LOG_ERR("listen err=%d", errno);
		return -1;
	}
	return 0;
}

int Acceptor::Accept(struct sockaddr_in& cltAddr)
{
	bzero(&cltAddr, sizeof cltAddr);
	socklen_t liAddrLen = static_cast<socklen_t>(sizeof cltAddr);

	int liConnFd = ::accept(_listenFd, (struct sockaddr*)&cltAddr, &liAddrLen);
	if (liConnFd > 0)
	{
		LOG_INFO("fd=%d time=%ld", liConnFd, GetTimestamp());
		return liConnFd;
	}
	else
	{
		if (errno == EMFILE)
		{
			::close(_idleFd);
			_idleFd = ::accept(_listenFd, NULL, NULL);
			::close(_idleFd);
			_idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
		}
	}
	return -1;
}
/*
void Acceptor::OnReadEvent(Timestamp now)
{
	struct sockaddr_in cltAddr;
	int liConnFd = Accept(cltAddr);
	if (liConnFd >= 0)
	{
		if (_OnConnection)
		{
			Socket::SetNonBlock(liConnFd);
			_OnConnection(liConnFd, cltAddr);
			 LOG_INFO("fd=%d t2=%d", liConnFd, GetTimestamp());
		}
		else
			::close(liConnFd);
	}
}
*/
