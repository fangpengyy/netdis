#include "Epoll.h"
#include "Log.h"
#include "EventLoop.h"
#include <errno.h>
#include <sys/resource.h>
#include <string.h>


#define EPOLLNVAL   0x0020


EventObj::EventObj(EventLoop* pEventLoop, int sockFd) :_pEventLoop(pEventLoop), _sockFd(sockFd)
{
	_CloseCbk = NULL;
	_ErrorCbk = NULL;
	_ReadCbk = NULL;
	_WriteCbk = NULL;
	_opEventType = etNone;
	_iEvents = 0;
}


void EventObj::SetRetEvents(int aiRetEvents)
{
	_iRetEvents = aiRetEvents;
}

bool EventObj::IsWriteState()
{
     return _iRetEvents & EPOLLOUT;
}

void EventObj::SetCloseEventCbk(EventCbk eventCbk)
{
	_CloseCbk = eventCbk;
}

void EventObj::SetErrorEventCbk(EventErrorCbk eventCbk)
{
	_ErrorCbk = eventCbk;
}

void EventObj::SetWriteEventCbk(WriteEventCbk eventCbk)
{
	_WriteCbk = eventCbk;
}

void EventObj::SetReadEventCbk(ReadEventCbk eventCbk)
{
	_ReadCbk = eventCbk;
}

bool EventObj::EnableRead()
{
	if (!(_iEvents & EPOLLIN))
	{
		_iEvents = EPOLLIN;
		_opEventType = _opEventType == etNone ? etAdd : etMod;
		return  _pEventLoop->UpdateEvent(this);
	}
	return false;
}

void EventObj::EnableWrite()
{
	if (!(_iEvents & EPOLLOUT))
	{
		_iEvents = EPOLLOUT;
		_opEventType = _opEventType == etNone ? etAdd : etMod;
		_pEventLoop->UpdateEvent(this);
	}
}

void EventObj::DisableWrite()
{
	_iEvents &= ~EPOLLOUT;
	_opEventType = _opEventType == etNone ? etAdd : etMod;
	_pEventLoop->UpdateEvent(this);
}

void EventObj::DisableAll()
{
	_iEvents = 0;
	_opEventType = etDel;
	_pEventLoop->UpdateEvent(this);
}

int EventObj::SockFd()
{
	return _sockFd;
}

int EventObj::Events()
{
	return _iEvents;
}

Enum_OpEvent EventObj::GetOpEventType()
{
	return _opEventType;
}

void EventObj::HandleEvent(Timestamp aiTime)
{
	if ((_iRetEvents & EPOLLHUP) && !(_iRetEvents & EPOLLIN))
	{
		if (_CloseCbk)
			_CloseCbk();
        return ;
	}

	if (_iRetEvents & EPOLLNVAL)
	{
		LOG_ERR("Channel::handle_event() POLLNVAL");
	}

	if (_iRetEvents & (EPOLLERR | EPOLLNVAL))
	{
		if (_ErrorCbk)
			_ErrorCbk(errno);
		return;
	}
	if (_iRetEvents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
	{
		if(_ReadCbk)
            _ReadCbk(aiTime);
	}

	if (_iRetEvents & EPOLLOUT)
	{
		if (_WriteCbk)
            _WriteCbk();
	}
}

//Epoll--------------


Epoll::Epoll() :_epollFd(-1), _pEvents(NULL), _maxEvent(DEF_MAX_EVENT)
{

}

Epoll::~Epoll()
{
   if(_epollFd >= 0)
   {
      close(_epollFd);
      _epollFd = -1;
   }

   if(_pEvents)
   {
        delete[]  _pEvents;
        _pEvents = NULL;
   }
}

int Epoll::Init(uint32_t  maxEvent)
{
    _maxEvent = maxEvent;

    _epollFd = epoll_create(maxEvent);
    if(_epollFd < 0)
    {
       LOG(1, "epoll_create err=%d", errno);
       return -1;
    }
    _pEvents = new struct epoll_event[maxEvent];
    if(_pEvents == NULL)
    {
        LOG(1, "new epoll_event err=%d", errno);
        return -2;
    }

    return 0;
}

int Epoll::Wait(int32_t timeout)
{
   return epoll_wait(_epollFd, _pEvents, _maxEvent, timeout);
}

struct epoll_event* Epoll::GetEvent(uint32_t aiIndex)
{
   if (aiIndex >= 0 && aiIndex < DEF_MAX_EVENT)
      return &_pEvents[aiIndex];
   return NULL;
}

bool Epoll::UpdateEvent(EventObj* lpEventObj)
{
   	int liRet = 0;
   	int sockFd = lpEventObj->SockFd();
	Enum_OpEvent op = lpEventObj->GetOpEventType();

    if (op == etDel)
	{
	    liRet = epoll_ctl(_epollFd, EPOLL_CTL_DEL, sockFd, NULL);
	    if(liRet < 0)
            return false;
         return true;
	}
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = lpEventObj->Events();
	event.data.ptr = lpEventObj;

	if (op == etAdd)
		liRet = epoll_ctl(_epollFd, EPOLL_CTL_ADD, sockFd, &event);
	else if (op == etMod)
		liRet = epoll_ctl(_epollFd, EPOLL_CTL_MOD, sockFd, &event);

	if (liRet < 0)
	{
		LOG_ERR("op=%d sockfd=%d", op, sockFd);
		return false;
	}
	return true;
}


