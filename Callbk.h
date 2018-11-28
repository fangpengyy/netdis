#ifndef __CALLBK_H__
#define __CALLBK_H__
#include "Utils.h"
#include "NetBuffer.h"
#include <string>



class HttpReq;
//class TcpConn;
//typedef std::shared_ptr<TcpConn> TcpConnPtr;

typedef std::function<void (TcpConnPtr)> TOnConnect;
typedef std::function<void(TcpConnPtr)> TOnClose;
typedef std::function<void (TcpConnPtr, int)> TOnError;
typedef std::function<void(TcpConnPtr)> TOnDestoryConn;
typedef std::function<void(TcpConn*, Timestamp)> TFilterTcpConn;
typedef std::function<void(TcpConn*, Timestamp)> TUpdateTimeTcpConn;

//0成功解析原始数据，<0 失败,  aiPackCount 返回解析包个数
typedef  std::function<int (TcpConnPtr, NetBuffer*, uint32_t&aiPackCount)>  TOnRawBuf;
typedef  std::function<int (TcpConnPtr, NetBuffer*, uint32_t&aiPackCount)>  TOnRouteRawBuf;

//统计
typedef   std::function<void (uint16_t statType, uint64_t aiNum)> TOnStatRecv;
//应用层回调接口
typedef std::function<int(TcpConnPtr, PackHead*apPackHead,  char* apBuf, uint32_t)> TOnRecvPack;
typedef  std::function<int (std::string&,  TcpConnPtr&,  HttpReq*)>  TOnHttpReq;
typedef  std::function<void (char*, uint32_t, TcpConnPtr&)> TOnJsonHttpReq;

//打包
typedef   std::function<char* (PackHead& aoPackHead, IPacket* pPack,  uint32_t& aiDataLen)>  TOnSendPack;


class IBaseCodec
{
public:
    virtual ~IBaseCodec(){}
    virtual void SetOnRecvPack(TOnRecvPack pOnRecvPack) { }
    virtual void SetOnHttpReq(TOnHttpReq pOnHttpReq){}
    virtual void SetOnJsonHttpReq(TOnJsonHttpReq pOnJsonHttpReq){}

    virtual void SetOnRouteRawBuf(TOnRouteRawBuf pOnRouteRawBuf){ }

    virtual char* OnSendPack(PackHead& aoPackHead, IPacket* pPack,  uint32_t& aiBufSize) { return 0; }
    virtual int OnRawBuf(TcpConnPtr pConnPtr, NetBuffer* pNetBuffer, uint32_t&aiPackCount)  { return 0; }

    virtual  char* OnEncodeWebSktBuf(const char* apBuf, uint32_t aiDataLen,  uint32_t & aiOutSize){return 0;}
};



#endif
