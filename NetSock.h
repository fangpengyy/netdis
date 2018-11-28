#ifndef __NETSOCK_H__
#define __NETSOCK_H__

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <signal.h>



//#include <netinet/in.h>
//#include <sys/types.h>
//#include <sys/socket.h>


class InitApp
{
 public:
   InitApp()
   {
       signal(SIGPIPE, SIG_IGN);
   }

};


class NetAddr
{
public:
   NetAddr(const char* asIp, int aiPort);
   NetAddr(int aiPort);

   struct sockaddr_in AddrIn(){return _addrIn;}
   void SetAddrIn(struct sockaddr_in& addrIn);

public:
   struct sockaddr_in FromDomain(const char* asDomain, int aiPort);
   struct sockaddr_in ToAddrIn(const char* asIp, int aiPort);
   static struct sockaddr_in ToAddrIn(int aiPort);
   static void ToIpPortStr(char* buf, size_t size, const struct sockaddr_in& addr);
   static void ToIpStr(char* buf, size_t size, const struct sockaddr_in& addr);
   static void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
   static struct sockaddr_in getLocalAddr(int sockfd);
   static struct sockaddr_in GetPeerAddr(int sockfd);


private:
   struct sockaddr_in _addrIn;

};



class Socket
{
public:

   static int SetSockFdLimit(uint32_t iSize);
   static int Connect(int sockFd, struct sockaddr_in* svrAddr);
   static int CreateTcpSocket();
   static void SetTcpNoDelay(int sockFd, bool on);
   static void SetReuseAddr(int sockFd, bool on);
   static void SetReusePort(int sockFd, bool on);
   static void SetKeepAlive(int sockFd, bool on);
   static void SetNonBlock(int sockFd);

   static void SetRecvBufSize(int sockFd, uint32_t aiSize);
   static void SetSendBufSize(int sockFd, uint32_t aiSize);

   static void Shutdown(int sockFd);
   static void Close(int sockFd);
   static int GetSocketError(int sockfd);

};

#endif
