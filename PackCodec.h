#ifndef  __PACKCODEC__H__
#define  __PACKCODEC__H__

#include "Callbk.h"
#include "NetBuffer.h"
#include "WebSktPack.h"
#include <functional>



class PackCodec:public IBaseCodec
{
public:
    PackCodec();
    virtual ~PackCodec();

    void SetOnRouteRawBuf(TOnRouteRawBuf pOnRouteRawBuf){   _OnRouteRawBuf = pOnRouteRawBuf;}
    void SetOnRecvPack(TOnRecvPack pOnRecvPack) {_OnRecvPack = pOnRecvPack;  }

    int OnRawBuf(TcpConnPtr pConnPtr, NetBuffer* pNetBuffer, uint32_t&aiPackCount);
    char* OnSendPack(PackHead& aoPackHead, IPacket* pPack,  uint32_t& aiBufSize);

private:
    TOnRecvPack _OnRecvPack;
    TOnRouteRawBuf _OnRouteRawBuf;
};

class HttpCodec:public IBaseCodec
{
public:
    HttpCodec():_OnHttpReq(nullptr){}
    virtual ~HttpCodec(){}

     int OnRawBuf(TcpConnPtr pConnPtr, NetBuffer* pNetBuffer, uint32_t&aiPackCount);

     void SetOnHttpReq(TOnHttpReq pOnHttpReq) {_OnHttpReq = pOnHttpReq; }
private:
    TOnHttpReq  _OnHttpReq;
};


class WebSktCodec:public IBaseCodec
{
public:
    WebSktCodec();
    virtual ~WebSktCodec();
    void SetOnJsonHttpReq(TOnJsonHttpReq pOnJsonHttpReq){_OnJsonHttpReq =  pOnJsonHttpReq;}

    int OnRawBuf(TcpConnPtr pConnPtr, NetBuffer* pNetBuffer, uint32_t&aiPackCount);
    char* OnEncodeWebSktBuf(const char* apBuf, uint32_t aiDataLen,  uint32_t & aiOutSize);

 private:
    void Response(TcpConnPtr& pConnPtr, uint32_t aiCode);
private:
    TOnJsonHttpReq  _OnJsonHttpReq;

    WebSktPack _oWebSktPack;
    char _szPongPack[128];
    uint32_t _iPongPackSize;
};



#endif
