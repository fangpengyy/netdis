#include "WebSktServer.h"
#include "PackCodec.h"
#include <jansson.h>


WebSktServer::WebSktServer()
{
    _pCodec = new WebSktCodec();


}

WebSktServer::~WebSktServer()
{

}

void WebSktServer::Start(int aiPort, uint32_t aiThreadNum)
{
    _pTcpServer = new TcpServer(_pCodec, aiPort);
    _pTcpServer->SetOnError(std::bind(&WebSktServer::OnError, this, std::placeholders::_1));
    _pTcpServer->SetOnClose(std::bind(&WebSktServer::OnClose, this, std::placeholders::_1));
    _pTcpServer->SetOnConnect(std::bind(&WebSktServer::OnConnect, this, std::placeholders::_1));

    _pCodec->SetOnJsonHttpReq(std::bind(&WebSktServer::OnJsonHttpReq, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _pTcpServer->Init(aiThreadNum);
    _pTcpServer->Run();
}

void WebSktServer::Stop()
{
    if(_pTcpServer)
        _pTcpServer->Uinit();
}

void WebSktServer::OnJsonHttpReq(char* apBuf, uint32_t aiSize, TcpConnPtr& apConn)
{
    std::string lsBuf(apBuf, apBuf + aiSize);
      printf("OnJsonHttpReq len =%d , %s\n", aiSize, lsBuf.c_str());
    json_t* lpRoot;
    json_error_t liErr;
    lpRoot = json_loads(lsBuf.c_str(), aiSize, &liErr);
    if(lpRoot == nullptr)
    {
        printf("%d\n", liErr);

    }
    json_t* lp = json_object_get(lpRoot, "id");
    if(lp)
    {
       uint64_t liId = json_number_value(lp);
       lp = json_object_get(lpRoot, "msg");
       const char* szMsg = json_string_value(lp);

        lp = json_object_get(lpRoot, "response");
        bool b = json_boolean_value(lp);
        printf("id=%d %s %d\n", liId, szMsg, b) ;
    }
    json_decref(lpRoot);

      char szBuf[1024] = {0};
      static uint32_t liIndex = 0;
      liIndex++;
      int liLen = sprintf(szBuf, "{\"id\":%d, \"value\":\"%s\"}", liIndex, lsBuf.c_str());
      apConn->SendWebSktBuf(szBuf, liLen);
}

void  WebSktServer::OnConnect(TcpConnPtr pConnPtr)
{
      HttpReq* lpReq = new HttpReq() ;
      pConnPtr->SetContext(lpReq);
}

void  WebSktServer::OnClose(TcpConnPtr pConnPtr)
{
    HttpReq* lpReq = static_cast<HttpReq*>(pConnPtr->GetContext());
    if(lpReq)
    {
        delete lpReq;
        pConnPtr->SetContext(nullptr);
    }
}

void  WebSktServer::OnError(TcpConnPtr pConnPtr)
{
    pConnPtr->Shutdown();
    HttpReq* lpReq = static_cast<HttpReq*>(pConnPtr->GetContext());
    if(lpReq)
    {
        delete lpReq;
        pConnPtr->SetContext(nullptr);
    }
}

