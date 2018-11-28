#include "NetSock.h"
#include "Log.h"
#include <sys/socket.h>
#include <netdb.h>
#include <sys/resource.h>


InitApp  g_InitApp;


NetAddr::NetAddr(const char* szIp, int aiPort)
{
   bzero(&_addrIn, sizeof(_addrIn));
   _addrIn.sin_family = AF_INET;
   _addrIn.sin_addr.s_addr = inet_addr(szIp);
   _addrIn.sin_port = htons(aiPort);
}

NetAddr::NetAddr(int aiPort)
{
   bzero(&_addrIn, sizeof(_addrIn));
   _addrIn.sin_family = AF_INET;
   _addrIn.sin_addr.s_addr = htonl(INADDR_ANY);
   _addrIn.sin_port = htons(aiPort);
}

struct sockaddr_in NetAddr::FromDomain(const char* asDomain, int aiPort)
{
	struct hostent* he = ::gethostbyname(asDomain);
	if (!he)
	{
		perror("gethostbyname");
		exit(1);
	}

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(aiPort);
	addr.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
	return addr;
}

struct sockaddr_in NetAddr::ToAddrIn(const char* asIp, int aiPort)
{
	struct sockaddr_in addrIn;
	bzero(&_addrIn, sizeof(addrIn));
	addrIn.sin_family = AF_INET;
	addrIn.sin_addr.s_addr = inet_addr(asIp);
	addrIn.sin_port = htons(aiPort);
	return addrIn;
}

struct sockaddr_in NetAddr::ToAddrIn(int aiPort)
{
	struct sockaddr_in addrIn;
	bzero(&addrIn, sizeof(addrIn));
	addrIn.sin_family = AF_INET;
	addrIn.sin_addr.s_addr = htonl(INADDR_ANY);
	addrIn.sin_port = htons(aiPort);
	return addrIn;
}

void NetAddr::SetAddrIn(struct sockaddr_in& addrIn)
{
   _addrIn = addrIn;
}

void NetAddr::ToIpPortStr(char* buf, size_t size, const struct sockaddr_in& addr)
{
	::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));

	size_t end = strlen(buf);
	uint16_t port = ntohs(addr.sin_port);
	snprintf(buf + end, size - end, ":%u", port);
}

void NetAddr::ToIpStr(char* buf, size_t size, const struct sockaddr_in& addr)
{
	::inet_ntop(AF_INET, &addr.sin_addr, buf, static_cast<socklen_t>(size));
}

void NetAddr::FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
		LOG_ERR("fromIpPort err=%d", errno);
}

struct sockaddr_in NetAddr::getLocalAddr(int sockfd)
{
	struct sockaddr_in localaddr;
	bzero(&localaddr, sizeof localaddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
	if (::getsockname(sockfd, (struct sockaddr*)&localaddr, &addrlen) < 0)
		LOG_ERR("getLocalAddr err=%d", errno);
	return localaddr;
}

struct sockaddr_in NetAddr::GetPeerAddr(int sockfd)
{
	struct sockaddr_in peeraddr;
	bzero(&peeraddr, sizeof peeraddr);
	socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
	if (::getpeername(sockfd, (struct sockaddr*)&peeraddr, &addrlen) < 0)
		LOG_ERR("getPeerAddr err=%d", errno);

	return peeraddr;
}



//Socket----------

int Socket::SetSockFdLimit(uint32_t iSize)
{
    struct rlimit limit_val;
    int32_t ret = getrlimit(RLIMIT_NOFILE, &limit_val);
    if (0 == ret && limit_val.rlim_cur < iSize && limit_val.rlim_cur < limit_val.rlim_max)
    {
        limit_val.rlim_cur = iSize + 1024;
        limit_val.rlim_cur = ((limit_val.rlim_cur > limit_val.rlim_max)
                              ? limit_val.rlim_max : limit_val.rlim_cur);
        ret = setrlimit(RLIMIT_NOFILE, &limit_val);
    }

    if (0 != ret)
    {
        LOG_ERR("file limit failed err=%d", errno);
        return -1;
    }
    return 0;
}

int Socket::Connect(int sockFd, struct sockaddr_in* svrAddr)
{
    return ::connect(sockFd, (struct sockaddr*)svrAddr, sizeof(*svrAddr));
}


int Socket::CreateTcpSocket()
{
    int sockFd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockFd < 0)
    {
        LOG_ERR("socket err=%d", errno);
	return - 1;
    }
    return sockFd;
}


void Socket::SetTcpNoDelay(int sockFd, bool on)
{
   int liVal = on ? 1 : 0;
   ::setsockopt(sockFd, IPPROTO_TCP, TCP_NODELAY, &liVal, static_cast<socklen_t>(sizeof liVal));
}

void Socket::SetReuseAddr(int sockFd, bool on)
{
   int liVal = on ? 1 : 0;
   ::setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &liVal, static_cast<socklen_t>(sizeof liVal));
}


void Socket::SetReusePort(int sockFd, bool on)
{
   int liVal = on ? 1 : 0;
   ::setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, &liVal, static_cast<socklen_t>(sizeof liVal));
}

void Socket::SetKeepAlive(int sockFd, bool on)
{
   int liVal = on ? 1 : 0;
   ::setsockopt(sockFd, SOL_SOCKET, SO_KEEPALIVE, &liVal, static_cast<socklen_t>(sizeof liVal));
}

void Socket::SetNonBlock(int sockfd)
{
   int flags = ::fcntl(sockfd, F_GETFL, 0);
   flags |= O_NONBLOCK;
   fcntl(sockfd, F_SETFL, flags);

   flags = ::fcntl(sockfd, F_GETFD, 0);
   flags |= FD_CLOEXEC;
   fcntl(sockfd, F_SETFD, flags);
}

void Socket::SetRecvBufSize(int sockFd, uint32_t aiSize)
{
   socklen_t loLen = sizeof aiSize;
   ::setsockopt(sockFd, SOL_SOCKET, SO_RCVBUF, &aiSize, loLen);
}

void Socket::SetSendBufSize(int sockFd, uint32_t aiSize)
{
   socklen_t loLen = sizeof aiSize;
   ::setsockopt(sockFd, SOL_SOCKET, SO_SNDBUF, &aiSize, loLen);
}

void Socket::Shutdown(int sockFd)
{
   if (::shutdown(sockFd, SHUT_WR) < 0)
   {
      LOG_ERR("shutdown err=%d", errno);
   }
}

void Socket::Close(int sockFd)
{
    if (::close(sockFd) < 0)
     {
	LOG_ERR("close err=%d", errno);
     }
}

int Socket::GetSocketError(int sockfd)
{
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);
	if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
		return errno;
	return optval;
}


