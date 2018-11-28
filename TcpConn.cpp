#include "TcpConn.h"
#include "DouList.h"
#include "EventLoop.h"
#include "Epoll.h"
#include "NetSock.h"
#include <iostream>


TcpConn::TcpConn(EventLoop* apLoop, int sockFd, uint32_t aiId) :
    _state(csConnecting), _pEventLoop(apLoop), _sockFd(sockFd), _iId(aiId)
{
    _OnConnect = nullptr;
    _OnClose = nullptr;
    _OnError = nullptr;
    _OnDestoryConn = nullptr;
    _OnStatRecv = nullptr;
    _pCodec = nullptr;

    prev = nullptr;
    next = nullptr;
    _lastAcTimestamp = 0;
    _pContext = nullptr;
    _iTurnIndex = 0;
    _iSeq = 0;
    _iSessionId = 0;
    _iDataFlag = 0;
    _iUserId = 0;

    _pEventObj = std::unique_ptr<EventObj>(new EventObj(apLoop, sockFd));
    _pEventObj->SetWriteEventCbk(std::bind(&TcpConn::OnWriteEvent, this));
    _pEventObj->SetReadEventCbk(std::bind(&TcpConn::OnReadEvent, this, std::placeholders::_1));
    _pEventObj->SetCloseEventCbk(std::bind(&TcpConn::OnCloseEvent, this));
    _pEventObj->SetErrorEventCbk(std::bind(&TcpConn::OnErrorEvent, this, std::placeholders::_1));

    SetUpdateTimeTcpConn(std::bind(&EventLoop::UpdateTimeTcpConn, apLoop, std::placeholders::_1, std::placeholders::_2));
}

TcpConn::~TcpConn()
{
    LOG_INFO(" state=%d  sockfd=%d", _state, _sockFd);
#ifdef DEF_COUT
    std::cout<<__FUNCTION__<<"  state="<<_state<<" sockfd="<<_sockFd<<"\n";
#endif
    CloseSocket();
}

void TcpConn::SetUpdateTimeTcpConn(TUpdateTimeTcpConn updateTimeTcpConn)
{
    _updateTimeTcpConn = updateTimeTcpConn;
}

void TcpConn::OnEnableConnect()
{
    Socket::SetTcpNoDelay(_sockFd, true);
    if(!_pEventObj->EnableRead())
    {
         LOG_INFO(" EnableRead fd=%d", _sockFd);
         return;
    }
    _state = csConnected;
    if (_OnConnect)
        _OnConnect(shared_from_this());

    if(_updateTimeTcpConn)
        _updateTimeTcpConn(this, GetTimestamp());
}

int TcpConn::OnReadEvent(Timestamp now)
{
    if (0 == _oRcvBuffer.ReadFd(_sockFd))
    {
        HandleClose();
        return 0;
    }

    if(_updateTimeTcpConn)
        _updateTimeTcpConn(this, now);

    if(_pCodec)
    {
        uint32_t liPackCount = 0;
        int liRet =_pCodec->OnRawBuf(shared_from_this(),  &_oRcvBuffer, liPackCount);
        if(_OnStatRecv && liPackCount > 0)
            _OnStatRecv(stRecvPack,  liPackCount);

        if(liRet == -2)
        {
            HandleClose();
            return -2;
        }
    }
    return 0;
}

int TcpConn::OnWriteEvent()
{
    if(_state == csDisConnected)
        return 0;

    int liSize = _oSndBuffer.ReadableSize();
    if (liSize > 0)
    {
        int liLen = ::send(_sockFd, _oSndBuffer.BeginRead(), liSize, 0);
        if (liLen > 0)
        {
            _oSndBuffer.AddReadPos(liLen);
            if (_oSndBuffer.ReadableSize() == 0)
            {
                _pEventObj->DisableWrite();
                if(_OnStatRecv)
                     _OnStatRecv(stSendPack, 1);

                if(_state == csDisConnecting)
                    _pEventLoop->RunInLoop(std::bind(&TcpConn::OnShutdown, this));
            }
            //return 0;
        }
        else if (liLen == 0)
        {
        }
        else
        {
            LOG_ERR("send liLen=%d err=%d", liLen, errno);
        }
    }
    return 0;
}

void TcpConn::OnCloseEvent()
{
    HandleClose();
}

void TcpConn::OnErrorEvent(int aiErr)
{
    if (_OnError)
        _OnError(shared_from_this(), aiErr);
    HandleClose();
}

int TcpConn::SendWebSktBuf(const char* apBuf, uint32_t aiSize)
{
    if (_state != csConnected)
        return -1;
    if(aiSize == 0 || apBuf == nullptr)
        return -2;

    uint32_t liBufSize = 0;
    char* lpBuf = _pCodec->OnEncodeWebSktBuf(apBuf,  aiSize, liBufSize);
    if(lpBuf == nullptr)
    {
        LOG_INFO("OnEncodeWebSktBuf ret =null");
        return -3;
    }

    if (_pEventLoop->IsSelfThread())
        SendInLoop(lpBuf, liBufSize);
    else
        _pEventLoop->RunInLoop(std::bind(&TcpConn::SendInLoop, this, lpBuf, liBufSize));

    return 0;
}

int TcpConn::SendBuf(const char* apBuf, uint32_t aiSize)
{
    if (_state != csConnected)
        return -1;

    if(aiSize == 0 || apBuf == nullptr)
        return -2;

    char*pszBuf = new char[aiSize];
    memcpy(pszBuf, apBuf, aiSize);

    if (_pEventLoop->IsSelfThread())
        SendInLoop(pszBuf, aiSize);
    else
        _pEventLoop->RunInLoop(std::bind(&TcpConn::SendInLoop, this, pszBuf, aiSize));

    return 0;
}

int TcpConn::SendPack(PackHead& aoPackHead, IPacket* pPack)
{
    if (_state != csConnected)
        return -1;
    if(_pCodec == nullptr)
        return -2;

  //  aoPackHead.seqId = _iSeq++;
    uint32_t liLen = 0;
    char* pszBuf = _pCodec->OnSendPack(aoPackHead,  pPack, liLen);
    if(pszBuf == nullptr)
    {
        LOG_ERR("%s", "OnSendPack ");
        return -3;
    }

    if (_pEventLoop->IsSelfThread())
        SendInLoop(pszBuf, liLen);
    else
        _pEventLoop->RunInLoop(std::bind(&TcpConn::SendInLoop, this, pszBuf, liLen));

    return 0;
}

int TcpConn::SendPack(IPacket* pPack)
{
  if (_state != csConnected)
        return -1;
    if(_pCodec == nullptr)
        return -2;

    PackHead loPackHead;
    loPackHead.SetHead(Global::GetCurSvrType(),  _iSeq++);

    uint32_t liLen = 0;
    char* pszBuf = _pCodec->OnSendPack(loPackHead,  pPack, liLen);
    if(pszBuf == nullptr)
    {
        LOG_ERR("%s", "OnSendPack ");
        return -3;
    }

    if (_pEventLoop->IsSelfThread())
        SendInLoop(pszBuf, liLen);
    else
        _pEventLoop->RunInLoop(std::bind(&TcpConn::SendInLoop, this, pszBuf, liLen));
    return 0;
}

void TcpConn::SendInLoop(const char* pszBuf, int aiDataLen)
{
    if (_state != csConnected || aiDataLen == 0)
    {
        if(pszBuf)
            delete[] pszBuf;
        return;
    }

    if (_oSndBuffer.ReadableSize() > 0)
    {
        _oSndBuffer.Append(pszBuf, aiDataLen);
        _pEventObj->EnableWrite();
    }
    else
    {
        int liLen = ::send(_sockFd, pszBuf, aiDataLen, 0);

        int liLeftSize = 0;
        if (liLen > 0)
            liLeftSize = aiDataLen - liLen;
        else if (liLen == 0)
        {
             delete[] pszBuf;
            return;
        }
        else
        {
            if (errno == EWOULDBLOCK)
            {
                liLeftSize = aiDataLen;
                liLen = 0;
            }
            else
            {
                LOG_ERR("err %d", errno);
                delete[] pszBuf;
                return;
            }
        }

        if (liLeftSize > 0)
        {
            _oSndBuffer.Append(pszBuf + liLen, liLeftSize);
            _pEventObj->EnableWrite();
        }
        else
        {
            if(_OnStatRecv)
                     _OnStatRecv(stSendPack, 1);
        }
    }
    delete[] pszBuf;
}

void TcpConn::HandleClose()
{
#ifdef DEF_COUT
    std::cout<<__FUNCTION__<<"  state="<<_state<<" fd="<<_sockFd<<"\n";
#endif
    //if(_state == csConnected || _state == csDisConnecting)
    if(_state != csDisConnected)
    {
        _state = csDisConnected;
        _pEventObj->DisableAll();

        if (_OnClose)
            _OnClose(shared_from_this());
   }
}
 void TcpConn::ForceClose()
 {
      HandleClose();
 }

 void  TcpConn::Shutdown()
 {
     if(_state == csConnected)
     {
         _state = csDisConnecting;
         _pEventLoop->RunInLoop(std::bind(&TcpConn::OnShutdown, shared_from_this()));
     }
 }

void TcpConn::OnShutdown()
{
    if(!_pEventObj->IsWriteState())
        ::shutdown(_sockFd, SHUT_WR);
}

void TcpConn::Destory()
{
#ifdef DEF_COUT
    std::cout<<"TcpConn::Destory  state="<<_state<<" fd="<<_sockFd<<"\n";
#endif
    if (_OnDestoryConn)
        _OnDestoryConn(shared_from_this());
}

bool TcpConn::IsDisConnect()
{
    return _state == csDisConnected;
}

void TcpConn::CloseSocket()
{
   ::close(_sockFd);
}

/*
    uint32_t liHeadSize = sizeof (PackHead);

    uint32_t liLen =DEF_PACK_MAX + liHeadSize;
    char*pszBuf = new char[liLen];

    uint32_t liDataLen = DEF_PACK_MAX;
    pPack->Pack(pszBuf + liHeadSize, liDataLen);

    aoPackHead.packTypeId = pPack->packTypeId();
    aoPackHead.packLen = liDataLen;
    aoPackHead.Pack(pszBuf,  liHeadSize);

    liLen = liDataLen + liHeadSize;
    */


