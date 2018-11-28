#include "HttpServer.h"
#include "Utils.h"
#include <algorithm>


HttpServer::HttpServer()
{
      _pTcpServer = nullptr;
      _iReqCount = 0;
      _iCloseReqCount = 0;
      _iErrorReqCount = 0;
      _iConnCount = 0;
      _pCodec = new HttpCodec();
}

HttpServer::~HttpServer()
{
    if(_pTcpServer)
    {
        delete _pTcpServer;
        _pTcpServer = nullptr;
    }
    if(_pCodec)
    {
        delete _pCodec;
        _pCodec = nullptr;
    }
}

void HttpServer::Start(int aiPort, uint32_t aiThreadNum)
{
       if(_pTcpServer )
              return;

       _pTcpServer = new TcpServer(_pCodec, aiPort);
       _pTcpServer->SetOnHttpReq(std::bind(&HttpServer::OnHttpReq, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

       _pTcpServer->SetOnConnect(std::bind(&HttpServer::OnConnect, this, std::placeholders::_1));
       _pTcpServer->SetOnClose(std::bind(&HttpServer::OnClose, this, std::placeholders::_1));
       _pTcpServer->SetOnError(std::bind(&HttpServer::OnError, this, std::placeholders::_1));

       _pTcpServer->Init(aiThreadNum);
       _pTcpServer->Run();
}

void HttpServer::Stop()
{
    if(_pTcpServer)
        _pTcpServer->Uinit();
}

int HttpServer::OnHttpReq(std::string& aoAction,  TcpConnPtr apConnPtr ,  HttpReq* apHttpReq)
{
       Response(apConnPtr, apHttpReq->Resp(), 200, aoAction.c_str(), aoAction.size());
       return 0;
}

void  HttpServer::OnConnect(TcpConnPtr pConnPtr)
{
      HttpReq* lpReq = new HttpReq() ;
      pConnPtr->SetContext(lpReq);

    //  LOG_INFO("Conn Count=%d",  ++_iConnCount);
   //  std::cout<<"Conn Count="<< ++_iConnCount<<"\n";
}

void  HttpServer::OnClose(TcpConnPtr pConnPtr)
 {
         HttpReq* lpReq = static_cast<HttpReq*>(pConnPtr->GetContext());
         if(lpReq == nullptr)
         {
             return;
         }
         Response(pConnPtr, lpReq->Resp(), 404);

        pConnPtr->SetContext(NULL);
        delete lpReq;
 }

 void  HttpServer::OnError(TcpConnPtr pConnPtr)
 {
      LOG_INFO("ErrorCount=%d",  ++_iErrorReqCount);
      //std::cout<<" ErrorCount="<< ++_iErrorReqCount<<"\n";

       HttpReq* lpReq = static_cast<HttpReq*>(pConnPtr->GetContext());
       if(lpReq == nullptr)
        {
             pConnPtr->Shutdown();
             return;
        }
       Response(pConnPtr, lpReq->Resp(), 404);

       pConnPtr->SetContext(NULL);
       delete lpReq;
 }

void  HttpServer::Response(TcpConnPtr& pConnPtr, HttpResp& httpResp, uint32_t aiCode,
                           const char*  apBody, uint32_t aiBodySize)
{
     httpResp.SetRespCode(aiCode);
     httpResp.SetBody(apBody, aiBodySize);

     uint32_t liLen = 0;
     const char* lpData = httpResp.GetRawData(liLen);
     pConnPtr->SendBuf(lpData,  liLen);
}
