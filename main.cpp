#include "TcpServer.h"

#include "Log.h"
#include "Connector.h"
#include "EventLoop.h"
#include "NetSock.h"
#include "Utils.h"
#include "TcpConn.h"
#include "HttpServer.h"
#include "./DefPack/RoutePack.h"
#include  "./DefPack/Pack_Include.h"
#include "RpcClient.h"
#include "WebSktServer.h"

//-----------------------------
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<execinfo.h>
//-----------------------------
#include <iostream>
#include <algorithm>



int g_k = 0;

void OnConnect(TcpConnPtr pConn)
{
    std::cout<<"conn"<<std::endl;

    TcpConnPtr p = pConn;

    PackHead loHead;
    loHead.SetHead(202, 0);
    Pack_Msg msg;
    const char* pBuf = "from client, hello world 123";
    msg.sMsg =pBuf;

     printf(" sendPack\n");

    p->SendPack(loHead, &msg);
}

int  Recv(TcpConnPtr pConn,  PackHead* pPackHead,  char* apBuf, uint32_t  len)
{
      std::cout<<"   packTypeId=" << pPackHead->packTypeId<<"   data:"<<apBuf<<"\n";
    IPacket* lpPack = GetPackObj(pPackHead->packTypeId);
    if(lpPack)
    {
         pConn->SetSessionId(pPackHead->sessionId);
         pConn->SetDataFlag(pPackHead->dataFlag);
         pConn->SetUserId(pPackHead->userId);

        lpPack->OnHandle(pConn, pPackHead->packTypeId, apBuf, len);
        delete lpPack;
    }
    else
           std::cout<<"  lpPack=null\n";
/*
    printf("-----send msg pack\n");
    PackHead loHead;
    loHead.SetHead(2, 1, 1);
    Pack_Msg msg;
    const char* p1=" from server , ask hello 1234567890 1234567890 A";
    msg.sMsg = p1;
    pConn->SendPack(loHead, &msg);
*/
    return len;
}

int  ClientRecv(TcpConnPtr pConn,  PackHead* pPackHead,  char* apBuf, uint32_t  len)
{
     std::cout<<"   packTypeId=" << pPackHead->packTypeId<<"\n";

    IPacket* lpPack = GetPackObj(pPackHead->packTypeId);
    if(lpPack)
    {
        lpPack->OnHandle(pConn, pPackHead->packTypeId, apBuf, len);
        delete lpPack;
    }
    else
        printf("lpPack=null\n");

   printf("send time\n");

    PackHead loHead;
    loHead.SetHead(203, 0);
    Pack_Msg msg;
    char szBuf[128] = {0};
    sprintf(szBuf, "from client  time = %ld",  time(0));
    msg.sMsg = szBuf;

    pConn->SendPack(loHead, &msg);

    return len;
}

void  ClientOnError(TcpConnPtr pConn, int err)
{
    std::cout<<"error\n";
}

void  ClientOnClose(TcpConnPtr pConn)
{
    std::cout<<"close\n";
}

void SystemErrorHandler(int signum)
{
    const int len=1024;
    void *func[len];
    size_t size;

    char **funs;

    signal(signum,SIG_DFL);
    size=backtrace(func,len);
    funs=(char**)backtrace_symbols(func,size);
    fprintf(stderr,"System error, Stack trace:\n");
    for(int i=0; i< (int)size; ++i) fprintf(stderr,"%d %s \n",i,funs[i]);
    free(funs);
    //exit(1);
}

#include "WebSktPack.h"

int main(int argc, char** arg)
{
    signal(SIGSEGV,SystemErrorHandler); //Invaild memory address
    signal(SIGABRT,SystemErrorHandler); // Abort signal

//test--------begin
{
    WebSktPack web;
    std::string str;
    web.GetHandShakeResp("423Fs2423423SF", 101, str);
}

//test -----------end




    std::string sFile="test";
    if(argc > 1)
    {
        sFile+="-" ;
        sFile+= arg[1];
        sFile+=arg[1];
        sFile+="-";
    }


   LOG_INIT("/home", sFile.c_str(), 1024*1024* 30);

    if(strcmp(arg[1] ,"h")==0)
    {
        std::cout<<" http server start...\n";
        //http测试服务，
        HttpServer httpServer;
        httpServer.Start(8090, 1);

        sleep(600000);
        std::cout<<"end\n";
        return 0;
    }
    else if(strcmp(arg[1] ,"s")==0) //tcp服务器
    {
        LOG_INFO("tcp server...");
        std::cout<<"server------\n";

        IBaseCodec* lpCodec = new PackCodec();
        TcpServer loSvr(lpCodec,  8091);

        loSvr.SetOnRecvPack(std::bind(Recv, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

        loSvr.Init(2);
        loSvr.Run();

        delete lpCodec;
    }
    else  if(strcmp(arg[1] ,"c")==0) //tcp客户端
    {
        printf("client-----head =%d--%d\n",
               (int) sizeof(PackHead), (int)sizeof (uint32_t));

       int liConnNum = atoi(arg[2]);

        EventLoop loop;
        loop.StartThread();

        struct sockaddr_in addr;
        NetAddr::FromIpPort( "192.168.7.40", 8091, &addr);
        IBaseCodec* lpCodec = new PackCodec();

        lpCodec->SetOnRecvPack(std::bind(ClientRecv, std::placeholders::_1,
                                std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

        Connector* loConns[100000];
        for(int i = 0; i < liConnNum; i++)
        {
            Connector* lpConn = new  Connector(lpCodec, &loop, addr);
            loConns[i] = lpConn;
            lpConn->SetOnConnect(std::bind(OnConnect, std::placeholders::_1));
            lpConn->SetOnError(std::bind(ClientOnError,std::placeholders::_1, std::placeholders::_2));
            lpConn->SetOnClose(std::bind(ClientOnClose,std::placeholders::_1));
            lpConn->Connect();
        }

        std::cout<<"wait----\n";
        sleep(600000);

        for(int i = 0; i < liConnNum; i++)
        {
            loConns[i]->Close();
        }

        std::cout<<"exit\n";
    }
     else  if(strcmp(arg[1] ,"r")==0) //rpc客户端
     {
            printf("rpc客户端\n");

         struct sockaddr_in addr;
        NetAddr::FromIpPort( "192.168.7.40", 8091, &addr);
         RpcClient loRpcClient(2, addr);
         if(!loRpcClient.Init())
         {
             printf(" loRpcClient.Init() failed\n ");
             return 0;
         }

        //测试
        for(int i =0; i < 3; i++)
         {
                Pack_ReqAddValue req;
                req.a1 = 40 + i *10;
                req.a2 = 50 + i * 20;

                 loRpcClient.Invoke(&req);
         }

         sleep(100);

         printf(" exit\n");

     }
     else  if(strcmp(arg[1] ,"w")==0) //websocket 服务器
     {
          printf("websocket 服务器\n");

        WebSktServer  webHttpServer;
        webHttpServer.Start(8092, 2);

        sleep(600000);
        webHttpServer.Stop();

        std::cout<<"end\n";
        return 0;
    }

    LOG_UNINIT();


    return 0;
}
