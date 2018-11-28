#include "RpcClient.h"
#include "EventLoop.h"
#include "PackCodec.h"
#include "NetBytes.h"
#include <stdarg.h>
#include "./DefPack/Pack_ReqName.h"
#include "./DefPack/RoutePack.h"


RpcClient::RpcClient(uint32_t aiChannelNum, struct sockaddr_in& aoSvrAddr):
     _iChannelNum(aiChannelNum),  _oSvrAddr(aoSvrAddr),   _iSeq(0), _iChannelIndex(0)
{
    _lpEvent = new EventLoop();
    _lpEvent->StartThread();

    _pCodec = new PackCodec();
    _pCodec->SetOnRecvPack(std::bind(&RpcClient::OnRecvPack,  this,
                                     std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

bool RpcClient::Init()
{
    for(uint32_t i = 0; i < _iChannelNum; i++)
    {
        Connector* lpConnector = new Connector(_pCodec , _lpEvent,  _oSvrAddr);
        if(lpConnector == nullptr)
            return false;

        _vecConnctor.push_back(lpConnector);
        lpConnector->Connect();
    }

    uint32_t liConnSucc = 0;
    for(uint32_t i = 0; i < _iChannelNum; i++)
    {
        Connector* lpConnector = _vecConnctor[i];
        if(lpConnector->IsConnected())
            liConnSucc++;
    }
    return liConnSucc == _iChannelNum;
}

RpcClient:: ~RpcClient()
{
    _lpEvent->Stop();

    for(int i= 0; i < (int)_vecConnctor.size(); i++)
    {
       Connector* lpConnector = _vecConnctor[i];
       if(lpConnector)
       {
           lpConnector->CloseSockFd();
           delete lpConnector;
       }
    }
    _vecConnctor.clear();

    if(_pCodec)
        delete _pCodec;
}

 void RpcClient::Invoke(IPacket* apPack)
{
    {
        std::lock_guard<std::mutex> loLocker(_mutIndex);
        if(_iChannelIndex >= _iChannelNum)
            _iChannelIndex = 0;
        else
            _iChannelIndex++;
    }
    Connector* lpConnector =_vecConnctor[_iChannelIndex];

    PackHead  loPackHead;
    loPackHead.SetHead(204,    0);

    lpConnector->SendPack(loPackHead,  apPack);
}

int RpcClient::OnRecvPack(TcpConnPtr apConn, PackHead*apPackHead,  char* apBuf, uint32_t aiLen)
{
    printf("%s packTypeId=%d\n",  __func__, apPackHead->packTypeId);

    IPacket* lpPack = GetPackObj(apPackHead->packTypeId);
    if(lpPack)
    {
        lpPack->OnHandle(apConn, apPackHead->packTypeId, apBuf, aiLen);
        delete lpPack;
    }

    return 0;
}



/*
void RpcClient::Pack(char* szBuf, int aiSize,  const std::string asMethod, const char* szParamType, ...)
{
    NetBytes loNetBytes(szBuf, aiSize);
    loNetBytes.Append(asMethod.c_str(), asMethod.size());
    loNetBytes.Append(szParamType, strlen(szParamType));

    int liLen = strlen(szParamType);
    va_list loArgs;
    va_start(loArgs, szParamType);

    for(int i = 0; i < liLen; i++)
    {
        switch(szParamType[i])
        {
        case 's':
        case 'S':
        {
            const char* szValue = va_arg(loArgs, char*);
            int liLen = strlen(szValue);
            loNetBytes.Append(szValue, liLen);
            break;
        }

        case 'i':
        {
            int32_t liValue = va_arg(loArgs, int32_t);
            loNetBytes<<liValue;
            break;
        }
        case 'I':
        {
            int64_t liValue = va_arg(loArgs, int64_t);
            loNetBytes<<liValue;
            break;
        }
        case 'u':
        {
            uint32_t liValue = va_arg(loArgs, uint32_t);
            loNetBytes<<liValue;
            break;
        }
        case 'U':
        {
            uint64_t liValue = va_arg(loArgs, uint64_t);
            loNetBytes<<liValue;
            break;
        }
        } //switch
    }//for
     va_end(loArgs);
}
*/


