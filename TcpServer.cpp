#include "TcpServer.h"
#include "Acceptor.h"
#include "TcpConn.h"
#include "NetSock.h"
#include "Log.h"
#include "EventLoop.h"

#include <unistd.h>
#include <memory>
#include <iostream>
#include <thread>
#include <sstream>


#define DEF_MAX_SOCK_NUM   1000000
#define  DEF_MAX_CONN_NUM   999999


TcpServer::TcpServer(IBaseCodec* apCodec,  int aiPort) :
    _pCodec(apCodec), _iConnId(0), _iIoThreadNum(4), _listenFd(-1), _iLoopIndex(-1)
{
    _listenAddr = NetAddr::ToAddrIn(aiPort);
    _pEventObj = NULL;
    _OnClose = nullptr;
    _OnConnect = nullptr;
    _OnError = nullptr;
    _OnRouteRawBuf = nullptr;

    _iCurConnCount = 0;
    _iAcceptErrorConn = 0;
    _iAcceptConnCount = 0;
}

TcpServer::~TcpServer()
{
    _bThStatus = false;
    if (_listenFd >= 0)
        ::close(_listenFd);

}

int TcpServer::Init(int iIoThreadCount)
{
    _listenFd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_listenFd < 0)
    {
        LOG_ERR("socket err=%d", errno);
        return - 1;
    }
    Socket::SetSockFdLimit(DEF_MAX_SOCK_NUM);
    Socket::SetReuseAddr(_listenFd, true);

    _pEventLoop = std::unique_ptr<EventLoop>(new EventLoop());
    _pAcceptor = std::unique_ptr<Acceptor>(new Acceptor(_pEventLoop.get()));

    if (_pAcceptor->Listen(_listenFd, _listenAddr) != 0)
    {
        LOG_ERR("listen err=%d", errno);
        return -2;
    }

    _iIoThreadNum = iIoThreadCount;
    for (int i = 0; i < _iIoThreadNum; i++)
    {
        EventLoop* lpLoop = new EventLoop();
        lpLoop->SetDecConnNumCbk(std::bind(&TcpServer::OnDecConnNum, this, std::placeholders::_1));
        lpLoop->SetOnCloseCbk(_OnClose);
        lpLoop->SetOnErrorCbk(_OnError);
        lpLoop->SetOnConnectCbk(_OnConnect);

        _vecEventLoop.push_back(lpLoop);
        lpLoop->StartThread();
    }

    _pEventObj = std::unique_ptr<EventObj>(new EventObj(_pEventLoop.get(), _listenFd));
    _pEventObj->SetReadEventCbk(std::bind(&TcpServer::OnRead, this, std::placeholders::_1));
    _pEventObj->EnableRead();

    //std::thread  loThStatus(std::bind(&TcpServer::OnStatus, this));
    //loThStatus.detach();
    return 0;
}

void TcpServer::Uinit()
{
    for (int i = 0; i < (int)_vecEventLoop.size(); i++)
        _vecEventLoop[i]->Stop();
    _vecEventLoop.clear();
}

void TcpServer::OnStatus()
{
    _bThStatus = true;
    while(_bThStatus)
    {
        std::stringstream loStr;
        uint32_t liCount = 0;
        for(int i = 0; i < _iIoThreadNum; i++)
        {
            EventLoop* lpLoop = _vecEventLoop[i];
            liCount += lpLoop->GetConnNum();

            WorkRunStatus loStatus;
            lpLoop->GetWorkStatus(loStatus);

             loStr <<  "[thid="<<lpLoop->GetThreadId()<<"]\n Current:  ReqConnCount="<<  lpLoop->GetConnNum()
            <<"  Handle-EventCount=" <<loStatus._iEventCount<<" t="<<loStatus._iDoEventTime<<" Handle-PendingCount="<<loStatus._iPendingCount
            <<" t="<<loStatus._iDoPendingTime<<" Handle-CheckConnCount="<<loStatus._iCheckConnNum<<" t="<<loStatus._iDoCheckConnTime
            <<" \n  Total:   ReqConnCount="<< loStatus._iTotalConnCount <<" ConnEnableCount="<<loStatus._iConnEnable<<" ConnClosedCount="<<loStatus._iConnClosed
            <<" Destory-ConnCount="<<loStatus._iDestoryConnCount<<" Handle-EventCount="<<loStatus._iTotalDoEventCount
            <<" -- RecvPackCount="<<loStatus._iRecvPack<<" RecvErrorCount="<<loStatus._iRecvError<<" TimeoutConnCount="<<loStatus._iTimeoutConnCount
            <<" SendPackCount="<<loStatus._iSendPack
            <<"\n";
        }
        loStr<<"\t  Current: Sum thid-ConnCount="<<liCount<< " AcceptConnCount="<<_iCurConnCount
        <<" Total:  Accept (ConnCount="<<_iAcceptConnCount<<" ErrorCount="<<_iAcceptErrorConn<<") "
        <<"\n\n"<<std::endl;

      // LOG_INFO("%s",  loStr.str().c_str());
       std::cout<<loStr.str();

        usleep(30000);
    }
}

EventLoop* TcpServer::GetEventLoop()
{
    _iLoopIndex++;
    if (_iLoopIndex >= _iIoThreadNum)
        _iLoopIndex = 0;

    return _vecEventLoop[_iLoopIndex];
}

void TcpServer::Run()
{
    _pEventLoop->AcceptRun();
}

int TcpServer::OnRead(Timestamp now)
{
    struct sockaddr_in addr;
    int liConnFd = _pAcceptor->Accept(addr);
    if (liConnFd > 0)
    {
        _iAcceptConnCount++;
       _iCurConnCount++;

        if(_iCurConnCount  >  DEF_MAX_CONN_NUM)
        {
            ::close(liConnFd);
            _iCurConnCount--;
            return 0;
        }
         _iConnId++;

        EventLoop* lpLoop = GetEventLoop();
        TcpConnPtr pConnPtr = std::make_shared<TcpConn>(lpLoop, liConnFd, _iConnId);
        pConnPtr->SetPackCodec(_pCodec);
        lpLoop->EnableConnect(pConnPtr);
    }
    else
    {
        _iAcceptErrorConn++;
    }
    return 0;
}

void TcpServer::OnDecConnNum(uint64_t aiNum)
{
     std::atomic_fetch_sub(&_iCurConnCount, aiNum);
}

