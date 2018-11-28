
#include "Timer.h"
#include "EventLoop.h"



uint32_t Timer::_timerID = 0;


Timer::Timer(TimerCbk timerCbk, uint32_t after/*ms*/, uint32_t interval/*ms*/):
_timerId(++_timerID), _OnTimer(timerCbk), _after(after), _interval(interval)
{
    _timestamp = GetTimestamp() + _after * 1000; //微妙
}


Timer::~Timer()
{
}

void Timer::ReStart()
{
	if (_interval > 0)
	_timestamp = GetTimestamp() + _interval * 1000;
}

void Timer::OnTimer()
{
   if(_OnTimer)
      _OnTimer();
}

bool Timer::IsLoop()
{
	 return _interval > 0;
}

//TimerList

TimerList::TimerList(EventLoop* apLoop):
  _loop(apLoop), _timerFd(CreateTimerfd()), _eventObj(_loop, _timerFd), _firstTimestamp(0)
{

}

TimerList::~TimerList()
{
	Clean();
}

void TimerList::EnableRead()
{
    _eventObj.SetReadEventCbk(std::bind(&TimerList::OnTime, this, std::placeholders::_1));
    _eventObj.EnableRead();
}

void TimerList::Clean()
{
	_queTimer.clear();
	std::map<uint32_t, Timer*>::iterator it = _mapFindTimer.begin();
	for (; it != _mapFindTimer.end(); it++)
	{
		if (it->second)
		delete it->second;
	}
	_mapFindTimer.clear();
}

void TimerList::InsertTimer(Timer* pTimer)
{
	if(_queTimer.size() <= 0 )
	{
		_queTimer.insert(TPair(pTimer->Timestamp(), pTimer));
	    _mapFindTimer.insert(std::make_pair(pTimer->TimerId(), pTimer));

		 uint64_t stamp = pTimer->Timestamp();
		 ResetTimeFd(_timerFd, stamp);
	}
	else
	{
		 uint64_t stamp = _queTimer.begin()->second->Timestamp();

		 _queTimer.insert(TPair(pTimer->Timestamp(), pTimer));
	     _mapFindTimer.insert(std::make_pair(pTimer->TimerId(), pTimer));

		 if(pTimer->Timestamp() < stamp)
		       ResetTimeFd(_timerFd, pTimer->Timestamp());
	}
}

void TimerList::DoInsertTimer(Timer* pTimer)
{
		_queTimer.insert(TPair(pTimer->Timestamp(), pTimer));
	    _mapFindTimer.insert(std::make_pair(pTimer->TimerId(), pTimer));
}

void TimerList::DelTimer(uint32_t timerId)
{
	std::map<uint32_t, Timer*>::iterator it= _mapFindTimer.find(timerId);
	if (it != _mapFindTimer.end())
	{
		_queTimer.erase(TPair(it->second->Timestamp(), it->second));
		_mapFindTimer.erase(it);
	}
}

uint32_t TimerList::GetFirstMsTimeout()
{
	if (_firstTimestamp > 0)
		return (GetTimestamp() - _firstTimestamp) / 1000;

	return 100;
}

std::vector<TimerList::TPair> TimerList::GetExpired(uint64_t now)
{
	std::vector<TPair> expired;
	TPair sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
	std::set<TPair>::iterator end = _queTimer.lower_bound(sentry);

	std::copy(_queTimer.begin(), end, std::back_inserter(expired));
	_queTimer.erase(_queTimer.begin(), end);

	for (std::vector<TPair>::iterator it = expired.begin();	it != expired.end(); ++it)
	{
	    _mapFindTimer.erase(it->first);
	}
	return expired;
}

int TimerList::RunAt(TimerCbk timerCbk, uint32_t after)
{
	Timer* lpTimer = new Timer(timerCbk, after, 0);
	if (lpTimer)
	{
		InsertTimer(lpTimer);
		return lpTimer->TimerId();
	}
	return -1;
}

int TimerList::RunEvery(TimerCbk timerCbk, uint32_t after, uint32_t interval)
{
	Timer* lpTimer = new Timer(timerCbk, after, interval);
	if (lpTimer)
	 {
	 	InsertTimer(lpTimer);
	 	return lpTimer->TimerId();
	 }
	 return -1;
}

int TimerList::OnTime(Timestamp now)
{
	ReadTimerfd(_timerFd, now);

	std::vector<TPair> vecTimer = GetExpired(now);
	for (int i = 0; i < (int)vecTimer.size(); i++)
	{
        Timer* lpTimer = vecTimer[i].second;
		lpTimer->OnTimer() ;

		if(lpTimer->IsLoop() > 0)
		{
			lpTimer->ReStart();
			DoInsertTimer(lpTimer);
		}
	}

	vecTimer.clear();
	if (!_queTimer.empty())
	{
		uint64_t stamp = _queTimer.begin()->second->Timestamp();
		ResetTimeFd(_timerFd, stamp);

		_firstTimestamp = stamp;
	}
	else
		_firstTimestamp = 0;

	return 1;
}
