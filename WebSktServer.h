#ifndef __WEBSOCKETSERVER_H__
#define __WEBSOCKETSERVER_H__

#include "TcpServer.h"
#include "NetBuffer.h"
#include "HttpReq.h"



class WebSktServer
{
public:
    WebSktServer();
    ~WebSktServer();

     void Start(int aiPort, uint32_t aiThreadNum);
     void Stop();

private:
    void OnJsonHttpReq(char* apBuf, uint32_t aiSize, TcpConnPtr& apConn);
    void  OnConnect(TcpConnPtr pConnPtr);
    void  OnClose(TcpConnPtr pConnPtr);
    void  OnError(TcpConnPtr pConnPtr);

private:
     TcpServer* _pTcpServer;
     IBaseCodec* _pCodec;

     std::atomic<int> _iReqCount;
     std::atomic<int> _iCloseReqCount;
     std::atomic<int> _iErrorReqCount;

     std::atomic<int> _iConnCount;
};




#endif
