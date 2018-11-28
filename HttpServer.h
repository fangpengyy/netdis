#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include "TcpServer.h"
#include "NetBuffer.h"
#include "HttpReq.h"




class HttpServer
{
public:
    HttpServer();
    ~HttpServer();

     void Start(int aiPort, uint32_t aiThreadNum);
     void Stop();

private:
    int OnHttpReq(std::string& aoAction,  TcpConnPtr apConnPtr ,  HttpReq* apHttpReq);
    void  OnConnect(TcpConnPtr pConnPtr);
    void  OnClose(TcpConnPtr pConnPtr);
    void  OnError(TcpConnPtr pConnPtr);

    void  Response(TcpConnPtr& pConnPtr, HttpResp& httpResp, uint32_t aiCode,
                   const char*  apBody = NULL, uint32_t aiBodySize=0);
private:
     TcpServer* _pTcpServer;
     IBaseCodec* _pCodec;

     std::atomic<int> _iReqCount;
     std::atomic<int> _iCloseReqCount;
     std::atomic<int> _iErrorReqCount;

     std::atomic<int> _iConnCount;
};




#endif
