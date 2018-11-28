#ifndef _TCPCONN_H__
#define _TCPCONN_H__

#include <memory>
#include <functional>
#include <stdint.h>
#include "Utils.h"
#include "NetBuffer.h"
#include "Callbk.h"
#include "DataStream.h"


class DouNode;
class EventLoop;
class EventObj;



class TcpConn:public std::enable_shared_from_this<TcpConn>
{
	friend class EventLoop;
	friend class DouList;
public:
	enum Enum_ConnState{csConnecting, csConnected, csDisConnecting, csDisConnected};
	TcpConn(EventLoop* apLoop, int sockFd, uint32_t aiId);
	~TcpConn();

	uint32_t GetId(){ return _iId; }
	int SendPack(PackHead& aoPackHead, IPacket* pPack);
	int SendBuf(const char* apBuf, uint32_t aiSize);
	int SendPack(IPacket* pPack);
	int SendWebSktBuf(const char* apBuf, uint32_t aiSize);

    void SetPackCodec(IBaseCodec* apCodec){_pCodec = apCodec;}

	void SetOnDestoryConn(TOnDestoryConn pOnDestoryConn){ _OnDestoryConn = pOnDestoryConn; }
	void SetOnConnect(TOnConnect pOnConnect){ _OnConnect = pOnConnect; }
	void SetOnClose(TOnClose pOnClose){ _OnClose = pOnClose; }
	void SetOnError(TOnError pOnError){ _OnError = pOnError; }

    void SetOnStatRecvCbk(TOnStatRecv pOnStatRecv){ _OnStatRecv = pOnStatRecv;}

    void OnEnableConnect();
    bool IsDisConnect();
    void Shutdown();
    void ForceClose();

    void SetLastTimestamp(Timestamp now){_lastAcTimestamp = now;}
    Timestamp GetLastTimestamp(){return _lastAcTimestamp;}

    void SetContext(void* apContext){_pContext = apContext;}
    void* GetContext(){return _pContext;}

    uint16_t GetTurnIndex(){return _iTurnIndex;}
    void SetTurnIndex(uint16_t aiTurnIndex){_iTurnIndex = aiTurnIndex;}

    uint64_t GetSessionId(){return _iSessionId;}
    uint8_t GetDataFlag(){return _iDataFlag;}
    void  SetSessionId(uint64_t aiSessionId){_iSessionId = aiSessionId;}
    void SetDataFlag(uint8_t aiDataFlag){_iDataFlag = aiDataFlag;}
    uint64_t GetUserId(){return _iUserId;}
    void SetUserId(uint64_t aiUserId){ _iUserId = aiUserId;}

private:
    void HandleClose();
    void Destory();
	bool CheckConnect(Timestamp now);
    bool IsSelfThread();

	int OnReadEvent(Timestamp now);
	int OnWriteEvent();
	void OnCloseEvent();
	void OnErrorEvent(int aiErr);
	void OnShutdown();

	void SendInLoop(const char* pszBuf, int aiDataLen);
	void CloseSocket();
    void SetUpdateTimeTcpConn(TUpdateTimeTcpConn updateTimeTcpConn);

private:
	Enum_ConnState _state;
	EventLoop* _pEventLoop;
	int _sockFd;
    IBaseCodec* _pCodec;

	NetBuffer _oRcvBuffer;
	NetBuffer _oSndBuffer;
	uint32_t _iId;

	std::unique_ptr<EventObj> _pEventObj;
    Timestamp _lastAcTimestamp;
    DataStream _packRecv;

    TcpConn* prev;
    TcpConn* next;
    void* _pContext;
    uint16_t _iTurnIndex;
    uint32_t _iSeq;

    uint64_t _iUserId;
    uint64_t _iSessionId;
    uint8_t _iDataFlag;
private:
	TOnConnect _OnConnect;
	TOnClose _OnClose;
	TOnError _OnError;
	TOnDestoryConn _OnDestoryConn;
    TUpdateTimeTcpConn _updateTimeTcpConn;
    TOnStatRecv _OnStatRecv;
};


#endif
