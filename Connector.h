#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include "Utils.h"
#include "Callbk.h"
#include <thread>


class EventObj;
class EventLoop;


class Connector
{
public:
    enum Enum_State{cltConnecting, cltConnected, cltDisConnect};
    Connector(IBaseCodec* apCodec, EventLoop* apLoop, struct sockaddr_in svrAddr);
    ~Connector();

    void Connect();
    void Close();
    void CloseSockFd();
    bool IsConnected(){return cltConnected == _state; }

    int SendPack(PackHead& aoPackHead, IPacket* pPack);
    //callback
    void SetOnClose(TOnClose onClose){_OnClose = onClose;}
    void SetOnConnect(TOnConnect onConnect){_OnConnect = onConnect;}
    void SetOnError(TOnError onError){_OnError = onError;}

private:
    //连接
    void OnEnableCheckConnect();
    void OnEnableTickHeart();

    void OnTickHeart();
    void OnCheckConn();

private:
  // void CloseSockFd();
    void  DoConnect();

   void OnNewTcpConn();
   void DisConnect();
   void Connecting(int sockFd);

private://释放
   void OnDisableTickHeart();
   void  OnDisableCheckConn();

   void DisableCheckConn();
   void DisableTickHeart();

private:
  //timer
   int OnWriteEvent();

   //TcpConn callback
   void OnConnected(TcpConnPtr pConn);
   void OnClose(TcpConnPtr pConn);
   void OnError(TcpConnPtr pConn, int aiError);

   void WaitExit();
   void NotifyExit();
private:
     //应用层数据接口
   IBaseCodec* _pCodec;
   EventLoop* _pEventLoop;
   struct sockaddr_in _svrAddr;
   int _sockFd;
   Enum_State _state;

   std::unique_ptr<EventObj> _pEventObjConn;
   TcpConnPtr _pConnPtr;

   int _iConnId;
   int _checkConnTimerId;
   int _heartTimerId;

   std::mutex _mutExit;
   std::condition_variable  _condExit;
    bool _bExit;
    std::mutex _mutSend;
    NotifyWait _oNotifyWait;
private:
   TOnConnect _OnConnect;
   TOnClose _OnClose;
   TOnError _OnError;
};

#endif
