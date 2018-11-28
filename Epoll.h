#ifndef __BASENET_H__
#define __BASENET_H__

#include <sys/epoll.h>
#include <stdint.h>
#include <unistd.h>
#include <functional>

#include "Utils.h"


//一次处理最大event数
#define DEF_MAX_EVENT   4096

#define DEF_SOCK_MAX_NUM  10000000



class EventLoop;



static const uint8_t CONNECT_ADDR = 0x04;   //主动连接
static const uint8_t LISTEN_ADDR = 0x02;    //监听
static const uint8_t ACCEPT_ADDR = 0x01;    //被动连接



enum ENUM_CONN_STATE{tcConnecting, tcConnected, tcDisconn};


typedef std::function<void(int)> EventErrorCbk;
typedef std::function<void()> EventCbk;
typedef std::function<int(Timestamp)> ReadEventCbk;
typedef std::function<int()> WriteEventCbk;

enum Enum_OpEvent{ etNone = 0, etAdd, etDel, etMod };


class EventObj
{
public:
	EventObj(EventLoop* pEventLoop, int sockFd);
	void SetRetEvents(int aiRetEvents);

	bool EnableRead();
	void EnableWrite();
	void DisableAll();
	void DisableWrite();

	int SockFd();
	Enum_OpEvent GetOpEventType();
	int Events();

	void SetCloseEventCbk(EventCbk evenCbk);
	void SetErrorEventCbk(EventErrorCbk evenCbk);
	void SetWriteEventCbk(WriteEventCbk evenCbk);
	void SetReadEventCbk(ReadEventCbk evenCbk);

	void HandleEvent(Timestamp aiTime);
    bool IsWriteState();

private:
	EventLoop* _pEventLoop;
	int _sockFd;
    int _iRetEvents;
	int _iEvents;
	Enum_OpEvent _opEventType;

	EventCbk _CloseCbk;
	EventErrorCbk _ErrorCbk;
	WriteEventCbk _WriteCbk;
	ReadEventCbk _ReadCbk;
};



class Epoll
{
public:
  Epoll();
  ~Epoll();

  int Init(uint32_t maxEvent);
  int Wait(int32_t timeout);
  struct epoll_event* GetEvent(uint32_t aiIndex);

  bool UpdateEvent(EventObj* lpEventObj);

private:
  int _epollFd;
  struct epoll_event* _pEvents;
  uint32_t _maxEvent;

};




#endif
