#ifndef __TIMER_H__
#define __TIMER_H__

#include <functional>
#include <unistd.h>
#include <map>
#include <set>
#include <vector>
#include "Epoll.h"


typedef std::function<void(void)> TimerCbk;


class Timer
{
public:
	Timer(TimerCbk timerCbk, uint32_t after, uint32_t interval);
	~Timer();

	uint32_t TimerId(){ return _timerId; }
	uint64_t Timestamp(){ return _timestamp; }

	void ReStart();
    bool IsLoop();

    void OnTimer();

private:
	uint32_t _timerId;
 	TimerCbk _OnTimer;
       uint32_t _after;
	uint32_t _interval;
	uint64_t _timestamp;
private:
	static uint32_t _timerID;


};


class EventLoop;

class TimerList
{
public:
	TimerList(EventLoop* apLoop);
	~TimerList();

	uint32_t GetFirstMsTimeout();

	int RunAt(TimerCbk timerCbk, uint32_t after);
	int RunEvery(TimerCbk timerCbk, uint32_t after, uint32_t interval);
	void DelTimer(uint32_t timerId);

	int OnTime(Timestamp now);
    void EnableRead();

	typedef std::pair<uint64_t, Timer*> TPair;
private:
	std::vector<TimerList::TPair> GetExpired(uint64_t now);
	void InsertTimer(Timer* pTimer);
	void Clean();

    void DoInsertTimer(Timer* pTimer);

private:
	EventLoop* _loop;
	int _timerFd;
	EventObj _eventObj;
	uint64_t _firstTimestamp;

	std::set<TPair> _queTimer;
	std::map<uint32_t, Timer*> _mapFindTimer;

};


#endif
