#include "EventLoop.h"
#include "Log.h"
#include "Utils.h"
#include <sys/eventfd.h>
#include <sys/epoll.h>

//-------------------

EventLoop::EventLoop() :_bStop(false), _pThread(NULL),
    _wakeupFd(-1), _threadId(-1), _oTimerList(this)
{
    _OnDecConnNum = nullptr;
    _douList.SetFilterFunc(std::bind(&EventLoop::FilterDisTcpConn, this, std::placeholders::_1, std::placeholders::_2));
    _threadId = gettid();
    _epollor.Init(DEF_MAX_EVENT);
    _pWakeUpEvent = nullptr;
    _oTimerList.EnableRead();
    _iTimeoutConnCount = 0;

    _OnClose = nullptr;
    _OnError = nullptr;
    _OnConnect = nullptr;

    _bAllowWakuUp = false;

   // _oTurnTimer.Init(90);

    for(int i = 0; i < stMaxNum; i++)
        _statRecv[i] = 0;
}

EventLoop::~EventLoop()
{
    if(_pWakeUpEvent)
    {
        delete _pWakeUpEvent;
        _pWakeUpEvent = nullptr;
    }
}

void EventLoop::StartThread()
{
    _pThread = new std::thread(std::bind(&EventLoop::Run, this));
    if (_pThread == NULL)
        LOG_ERR("create thread err %d", errno);
}

void EventLoop::Stop()
{
    _bStop = true;
    WakeUp();

    if (_pThread)
        _pThread->join();
}

bool EventLoop::IsSelfThread()
{
    return _threadId == gettid();
}

int EventLoop::RunAfterTime(int aiMs, TimerCbk timerCbk)
{
    return _oTimerList.RunAt(timerCbk, aiMs);
}

int EventLoop::RunEveryTime(int aiMs,  int aiInterval, TimerCbk timerCbk)
{
    return _oTimerList.RunEvery(timerCbk, aiMs, aiInterval);
}

void EventLoop::DelTimer(uint32_t timerId)
{
    _oTimerList.DelTimer(timerId);
}

void EventLoop::RunInLoop(Functor lpFunctor)
{
    if (IsSelfThread())
    {
        lpFunctor();
    }
    else
    {
        {
            std::lock_guard<std::mutex> loLocker(_pendingMutex);
            _vecFunctor.push_back(lpFunctor);
        }
        if(!_bDoEvent)
            WakeUp();
    }
}

void EventLoop::WakeUp()
{
    if(!_bAllowWakuUp)
        return;

    uint64_t liVal = 1;
    int liSize = ::write(_wakeupFd, &liVal, sizeof liVal);
    if (liSize != sizeof liVal)
        LOG_ERR("wakeup() write size= %d", liSize);
    LOG_INFO("wakeup %d", _wakeupFd);
}

int EventLoop::ReadWakeUp(Timestamp now)
{
    uint64_t liVal = 0;
    int liRet = ::read(_wakeupFd, &liVal, sizeof liVal);
    LOG_INFO("wakeup ret=%d fd=%d", liRet, _wakeupFd);
    return 0;
}

uint32_t EventLoop::DoPendingFunctor()
{
    std::vector<Functor> loVecFunctor;
    {
        std::lock_guard<std::mutex> loLocker(_pendingMutex);
        if(!_vecFunctor.empty())
        {
            std::copy(_vecFunctor.begin(), _vecFunctor.end(),  std::back_inserter(loVecFunctor));
            _vecFunctor.clear();
        }
    }
    uint32_t liCount = (uint32_t)loVecFunctor.size();
    if(liCount > 0)
    {
        for (uint32_t  i = 0; i <  liCount; i++)
            loVecFunctor[i]();
    }
    return liCount;
}

bool EventLoop::UpdateEvent(EventObj* lpEventObj)
{
    if (IsSelfThread())
        return _epollor.UpdateEvent(lpEventObj);
    else
        RunInLoop( std::bind(&Epoll::UpdateEvent, &_epollor, lpEventObj) );
    return true;
}

void EventLoop::UpdateTimeTcpConn(TcpConn* apConn, Timestamp now)
{
    if (apConn == NULL)
        return;

   apConn->SetLastTimestamp(now);
    _douList.UpdateNode(apConn);
    //_oTurnTimer.Update(apConn);
}

bool EventLoop::FilterDisTcpConn(TcpConn* apNode, Timestamp now)
{
    if(apNode == nullptr)
        return false;

    if(now >  apNode->GetLastTimestamp()  + DEF_CONN_TIME_OUT)
    {
        _iTimeoutConnCount++;
        return true;
    }
    return false;
}

int EventLoop::CheckConn(Timestamp aiTime)
{
    return _douList.CheckNodeList(aiTime);
}

void EventLoop::CheckConnTimeout(uint64_t  aiDelta)
{
//       _oTurnTimer.CheckTimeout(aiDelta);
}

void EventLoop::OnDestoryConn(TcpConnPtr  apConnPtr)
{
    if (IsSelfThread())
        DestoryConn(apConnPtr);
    else
        RunInLoop(std::bind(&EventLoop::DestoryConn, this, apConnPtr));
}

void EventLoop::DestoryConn(TcpConnPtr  apConnPtr)
{
    _douList.DelNode(apConnPtr.get());
    //_oTurnTimer.Remove(apConnPtr.get());

    _mapConn.erase(apConnPtr->GetId());
    if(_OnDecConnNum)
        _OnDecConnNum(1);
     OnStatRecv(stDestory, 1);
}

void EventLoop::OnClose(TcpConnPtr apConnPtr)
{
      if(_OnClose)
        _OnClose(apConnPtr);
    _vecFree.push_back(apConnPtr) ;
    OnStatRecv(stClosed, 1);
}

void EventLoop::OnError(TcpConnPtr apConnPtr, int aiErr)
{
    if(_OnError)
        _OnError(apConnPtr, aiErr);
    OnStatRecv(stRecvError, 1);
}

void EventLoop::FreeVecConn()
{
    if(_vecFree.size() == 0)
        return;

    for(int i = 0; i < (int)_vecFree.size(); i++)
        _vecFree[i]->Destory();
    _vecFree.clear();
}

void EventLoop::OnConnect(TcpConnPtr apConnPtr)
{
    if(_OnConnect)
        _OnConnect(apConnPtr);
    OnStatRecv(stConn, 1);
}

int EventLoop::EnableWakeUp()
{
    _wakeupFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (_wakeupFd < 0)
    {
        LOG_ERR("eventfd err=%d", errno);
        return -1;
    }
    _pWakeUpEvent = new EventObj(this, _wakeupFd);
    _pWakeUpEvent->SetReadEventCbk(std::bind(&EventLoop::ReadWakeUp, this, std::placeholders::_1));
    _bAllowWakuUp = _pWakeUpEvent->EnableRead();
    return 0;
}

void EventLoop::OnStatRecv(uint16_t statType, uint64_t aiNum)
{
    _statRecv[statType] += aiNum;
}

void EventLoop::AcceptRun()
{
    _threadId = gettid();
    int32_t liMsTimeout = 1000;

    while (!_bStop)
    {
        int liEventNum = _epollor.Wait(liMsTimeout);
        Timestamp liNowTime = GetTimestamp();

        if (liEventNum > 0)
        {
            for (int i = 0; i < liEventNum; i++)
            {
                struct epoll_event* lpEvent = _epollor.GetEvent(i);
                EventObj* lpEventObj = (EventObj*)lpEvent->data.ptr;
                lpEventObj->SetRetEvents(lpEvent->events);
                lpEventObj->HandleEvent(liNowTime) ;
            }
        }
    }
}

void EventLoop::Run()
{
    _threadId = gettid();
    EnableWakeUp();

    int32_t liMsTimeout = 10000;

    uint64_t liHandleEventTime = 0;
    uint32_t liPendingCount = 0;
    uint64_t liPendingtime = 0;
    uint32_t liCheckConnCount = 0;
    uint64_t liCheckConnTime = 0;
    uint32_t liDoEventCount = 0;
    Timestamp t1, t2, t3;

    while (!_bStop)
    {
        int liEventNum = _epollor.Wait(liMsTimeout);
        _bDoEvent = true;
        Timestamp liNowTime = GetTimestamp();

        if (liEventNum > 0)
        {
            for (int i = 0; i < liEventNum; i++)
            {
                struct epoll_event* lpEvent = _epollor.GetEvent(i);
                EventObj* lpEventObj = (EventObj*)lpEvent->data.ptr;
                lpEventObj->SetRetEvents(lpEvent->events);
                lpEventObj->HandleEvent(liNowTime) ;
            }
        }
        _bDoEvent = false;
        liDoEventCount += liEventNum;

        t1 = GetTimestamp();
        liHandleEventTime = t1 - liNowTime;

        liPendingCount = DoPendingFunctor();
        t2 = GetTimestamp();
        liPendingtime = t2 - t1;

        liCheckConnCount = CheckConn(liNowTime);
        //CheckConnTimeout(t2 - liNowTime);
        t3 = GetTimestamp();
        liCheckConnTime = t3 - t2;

        FreeVecConn();
         //WorkStatus
        {
            std::lock_guard<std::mutex> loLocker(_oMutStatus);

            _oWorkStatus._iTotalDoEventCount = liDoEventCount;
            _oWorkStatus._iEventCount = liEventNum;
            _oWorkStatus._iDoEventTime = liHandleEventTime;

            _oWorkStatus._iCheckConnNum = liCheckConnCount;
            _oWorkStatus._iDoCheckConnTime = liCheckConnTime;
            _oWorkStatus._iPendingCount = liPendingCount;
            _oWorkStatus._iDoPendingTime = liPendingtime;

             _oWorkStatus._iTimeoutConnCount = _iTimeoutConnCount;
            _oWorkStatus._iTotalConnCount = _iTotalConnCount;

            _oWorkStatus._iRecvError  = _statRecv[stRecvError];
            _oWorkStatus._iRecvPack =  _statRecv[stRecvPack];
            _oWorkStatus._iSendPack =  _statRecv[stSendPack];
            _oWorkStatus._iConnEnable = _statRecv[stConn];
            _oWorkStatus._iConnClosed = _statRecv[stClosed];
            _oWorkStatus._iDestoryConnCount = _statRecv[stDestory];
       }
    }//while
}

void EventLoop::GetWorkStatus(WorkRunStatus& aoStatus)
{
    std::lock_guard<std::mutex> loLocker(_oMutStatus);
    aoStatus = _oWorkStatus;
}

void EventLoop::EnableConnect(TcpConnPtr& apConnPtr)
{
    _iTotalConnCount++;
    _mapConn.insert(std::make_pair(apConnPtr->GetId(), apConnPtr));

    apConnPtr->SetOnDestoryConn(std::bind(&EventLoop::OnDestoryConn, this, std::placeholders::_1));
    apConnPtr->SetOnClose(std::bind(&EventLoop::OnClose, this, std::placeholders::_1));
    apConnPtr->SetOnError(std::bind(&EventLoop::OnError, this, std::placeholders::_1, std::placeholders::_2));
    apConnPtr->SetOnConnect(std::bind(&EventLoop::OnConnect, this, std::placeholders::_1));
    apConnPtr->SetOnStatRecvCbk(std::bind(&EventLoop::OnStatRecv, this, std::placeholders::_1, std::placeholders::_2));

    RunInLoop(std::bind(&TcpConn::OnEnableConnect, apConnPtr.get()));
}

uint32_t  EventLoop::GetConnNum()
{
    return _mapConn.size();
}

pid_t EventLoop::GetThreadId()
{
    return _threadId;
}

