#ifndef __RPC__CLIENT_H__
#define __RPC__CLIENT_H__
#include "Connector.h"
#include <mutex>
#include <vector>
#include <atomic>


class RpcClient
{
public:
    RpcClient(uint32_t aiChannelNum, struct sockaddr_in& aoSvrAddr);
    ~RpcClient();

    bool Init();
    void Invoke(IPacket* apPack);

private:
    int OnRecvPack(TcpConnPtr apConn, PackHead*apPackHead,  char* apBuf, uint32_t aiLen);

private:
    uint32_t _iChannelNum;
    struct sockaddr_in& _oSvrAddr;
    std::atomic<uint32_t> _iSeq;
    uint32_t _iChannelIndex;

    IBaseCodec* _pCodec;
    EventLoop* _lpEvent;
    std::mutex  _mutIndex;
    std::vector<Connector*> _vecConnctor;

};

#endif
