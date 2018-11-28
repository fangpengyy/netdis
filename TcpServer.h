#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include <map>
#include <functional>
#include <vector>
#include <atomic>

#include "Utils.h"
#include "TcpConn.h"
#include "PackCodec.h"



class TcpConn;
class EventLoop;
class EventObj;
class Acceptor;


class TcpServer
{
public:
	TcpServer(IBaseCodec*apCodec,  int aiPort);
	~TcpServer();
	int Init(int iIoThreadNum);
	void Run();
	void Uinit();

	void SetOnConnect(TOnConnect pOnConnect){ _OnConnect = pOnConnect; }
	void SetOnClose(TOnClose pOnClose){ _OnClose = pOnClose; }
	void SetOnError(TOnError pOnError){ _OnError = pOnError; }

   //原始数据层
    void SetOnRouteRawBuf(TOnRouteRawBuf pOnRouteRawBuf){   _OnRouteRawBuf = pOnRouteRawBuf;}
   //应用层数据
    void SetOnHttpReq(TOnHttpReq pOnHttpReq){ _pCodec->SetOnHttpReq(pOnHttpReq); }
	void SetOnRecvPack(TOnRecvPack pOnRecvPack){ _pCodec->SetOnRecvPack(pOnRecvPack); }

private:
	int OnRead(Timestamp now);

private:
	EventLoop* GetEventLoop();
    void OnStatus();
    void OnDecConnNum(uint64_t aiNum);
private:
    IBaseCodec*_pCodec;
	uint32_t _iConnId;
	int _iIoThreadNum;
	int _listenFd;
	int _iLoopIndex;

    std::unique_ptr<EventObj> _pEventObj;
	struct sockaddr_in _listenAddr;

	std::unique_ptr<Acceptor> _pAcceptor;
	std::unique_ptr<EventLoop> _pEventLoop;
	std::vector<EventLoop*> _vecEventLoop;
private:
	TOnConnect _OnConnect;
	TOnClose _OnClose;
	TOnError _OnError;
    TOnRouteRawBuf _OnRouteRawBuf ;

    std::atomic<uint64_t> _iCurConnCount;
    std::atomic<uint64_t>_iAcceptErrorConn;
     std::atomic<uint64_t>_iAcceptConnCount;
    bool _bThStatus;
};



#endif
