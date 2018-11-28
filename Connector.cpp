#include "Connector.h"
#include "EventLoop.h"
#include "Epoll.h"
#include "NetSock.h"
#include "Pack.h"
#include <iostream>
#include "DefPack/Pack_ReqName.h"



#define DEF_TRY_MAX_CNT      10
#define DEF_MAX_DELAY_MS     30*1000


Connector::Connector(IBaseCodec* apCodec, EventLoop* apLoop, struct sockaddr_in svrAddr):
    _pCodec(apCodec), _pEventLoop(apLoop), _svrAddr(svrAddr), _state(cltDisConnect)
{
    _iConnId = 0;
    _checkConnTimerId = 0;
    _heartTimerId = 0;
    _bExit = false;
}

Connector::~Connector()
{
#ifdef DEF_COUT
    std::cout<<__FUNCTION__<<" exit...\n";
#endif
    Close();
    WaitExit();
#ifdef DEF_COUT
    std::cout<<__FUNCTION__<<" exit\n";
#endif
}

void Connector::Close()
{
    if(_state == cltConnected || _state == cltConnecting)
    {
        _pEventLoop->RunInLoop(std::bind(&Connector::DisConnect, this));
    }
    else
    {
        if(_sockFd > 0)
        {
            close(_sockFd);
            _sockFd = -1;
        }
    }
}

void Connector::CloseSockFd()
{
    if(_sockFd > 0)
    {
        close(_sockFd);
        _sockFd = -1;
    }
}

void Connector::DoConnect()
{
    // Close();
    _state = cltConnecting;

    _sockFd = Socket::CreateTcpSocket();
    if(_sockFd < 0)
    {
        LOG_ERR("create socket fd=%d err=%d", _sockFd, errno);
        return;
    }

    Socket::SetNonBlock(_sockFd);
    int liRet = Socket::Connect(_sockFd, &_svrAddr);
    int liErr = (liRet == 0) ? 0 : errno;

    switch(liErr)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
        _pEventLoop->RunInLoop(std::bind(&Connector::Connecting, this, _sockFd));
        break;
    default:
    {
        close(_sockFd);
        _state = cltDisConnect;
#ifdef DEF_COUT
        std::cout<<__FUNCTION__<<" --cltDisConnect\n";
#endif
        LOG_ERR("connect err=%d",errno);
    }
    }
}

void Connector::Connect()
{
     _pEventLoop->RunInLoop(std::bind(&Connector::OnEnableCheckConnect, this));
     _oNotifyWait.Wait(500);

     _pEventLoop->RunInLoop(std::bind(&Connector::OnEnableTickHeart, this));
}

void Connector::Connecting(int sockFd)
{
    //  _state = cltConnecting;
    _pEventObjConn.reset(new EventObj(_pEventLoop, sockFd));
    _pEventObjConn->SetWriteEventCbk(std::bind(&Connector::OnWriteEvent, this));
    _pEventObjConn->EnableWrite();
}

void Connector::DisConnect()
{
    DisableCheckConn();
    DisableTickHeart();

    if(_pConnPtr.get())
    {
        _pConnPtr->Shutdown();
    }

    if(_pEventObjConn.get())
    {
        _pEventObjConn->DisableAll();
    }
}

int Connector::SendPack(PackHead& aoPackHead, IPacket* pPack)
{
    printf("%s state=%d\n", __func__, _state);
    std::lock_guard<std::mutex> loLocker(_mutSend);

    if(_state == cltConnected)
    {
        if(_pConnPtr.get())
        {
            return _pConnPtr->SendPack(aoPackHead,  pPack);
        }
        else
        {
            LOG_ERR("_pConnPtr ==null");
            return -3;
        }
    }
    LOG_ERR("state =%d", _state);
    return -1;
}

void Connector::OnNewTcpConn()
{
  //  _pCodec->SetOnRecvPack(_OnRecvPack);

    _iConnId++;
    _pConnPtr = std::make_shared<TcpConn>(_pEventLoop, _sockFd, _iConnId);
    _pConnPtr->SetOnClose(std::bind(&Connector::OnClose, this, std::placeholders::_1));
    _pConnPtr->SetOnError(std::bind(&Connector::OnError, this,  std::placeholders::_1, std::placeholders::_2));
    _pConnPtr->SetOnConnect(std::bind(&Connector::OnConnected, this,  std::placeholders::_1));
    _pConnPtr->SetPackCodec(_pCodec);
    _pConnPtr->OnEnableConnect();
}

void Connector::OnConnected(TcpConnPtr pConn)
{
#ifdef DEF_COUT
    std::cout<<__FUNCTION__<<" --OnConnected\n";
#endif
//    _state = cltConnected;
    if(_OnConnect)
        _OnConnect(pConn);
}

void Connector::OnClose(TcpConnPtr pConn)
{
#ifdef DEF_COUT
    std::cout<<__FUNCTION__<<" --cltDisConnect\n";
#endif
    _state = cltDisConnect;
    if(_OnClose)
        _OnClose(pConn);

    NotifyExit();
}

void Connector::OnError(TcpConnPtr pConn, int aiError)
{
#ifdef DEF_COUT
    std::cout<<__FUNCTION__<<" --cltDisConnect\n";
#endif
    _state = cltDisConnect;
    if(_OnError)
        _OnError(pConn, aiError);
}

int Connector::OnWriteEvent()
{
    int liErr = Socket::GetSocketError(_sockFd);

    _pEventObjConn->DisableAll();
    if(liErr == 0)
    {
        _state = cltConnected;
#ifdef DEF_COUT
        std::cout<<__FUNCTION__<<" --OnConnected\n";
#endif
        OnNewTcpConn();
        _oNotifyWait.Notify();
    }
    else
    {
        _state = cltDisConnect;
#ifdef DEF_COUT
        std::cout<<__FUNCTION__<<" --cltDisConnect\n";
#endif
        LOG_ERR("err %d, try conn",liErr);
        return -1;
    }
    return 0;
}

void Connector::OnEnableTickHeart()
{
    if(_heartTimerId > 0)
    {
        _pEventLoop->DelTimer(_heartTimerId);
        _heartTimerId = 0;
    }
    _heartTimerId = _pEventLoop->RunEveryTime(1000* 10, 1000 *30 , std::bind(&Connector::OnTickHeart, this));
}

void Connector::OnDisableTickHeart()
{
    if(_heartTimerId > 0)
    {
        _pEventLoop->DelTimer(_heartTimerId);
        _heartTimerId = 0;
    }
}

void Connector::OnDisableCheckConn()
{
    if(_checkConnTimerId > 0)
    {
        _pEventLoop->DelTimer(_checkConnTimerId);
        _checkConnTimerId = 0;
    }
}

void Connector::DisableCheckConn()
{
    _pEventLoop->RunInLoop(std::bind(&Connector::OnDisableCheckConn, this));
}

void Connector::DisableTickHeart()
{
    _pEventLoop->RunInLoop(std::bind(&Connector::OnDisableTickHeart, this));
}


void Connector::OnEnableCheckConnect()
{
    if(_checkConnTimerId > 0)
    {
        _pEventLoop->DelTimer(_checkConnTimerId);
        _checkConnTimerId = 0;
    }
    _checkConnTimerId = _pEventLoop->RunEveryTime(0,  1000* 30,  std::bind(&Connector::OnCheckConn, this));
}

void Connector::OnCheckConn()
{
    if(_state != cltConnected && _state != cltConnecting)
    {
#ifdef DEF_COUT
        std::cout << " TryConnect\n";
#endif
        DoConnect();
    }
}

void Connector::OnTickHeart()
{
    if(_state != cltConnected)
        return;
    if(_pConnPtr.get())
    {
        Pack_TickHeart tickHeart;
        PackHead loHead;
        loHead.SetHead(200, 0);

        int liRet = _pConnPtr->SendPack(loHead,  &tickHeart);
       // printf("%s heart=%d\n", __func__, liRet);

        if(0 != liRet)
        {
            LOG_ERR("heart ret=%d",  liRet);
#ifdef DEF_COUT
            std::cout << " error----heart\n";
#endif
        }
    }
}

void Connector::WaitExit()
{
    std::unique_lock<std::mutex> locker(_mutExit);
    while(!_bExit)
        _condExit.wait(locker);
}

void Connector::NotifyExit()
{
    {
        std::unique_lock<std::mutex> locker(_mutExit);
        _bExit = true;
    }
    _condExit.notify_one();
}



