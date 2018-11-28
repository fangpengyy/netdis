#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include <functional>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

#include "Timer.h"
#include "Utils.h"
#include "DouList.h"
//#include "TimeoutConns.h"


typedef  std::function<void (uint32_t aiConnNum)>   TDecConnNumCbk;
typedef  std::function<void()> Functor;

struct WorkRunStatus
{
    WorkRunStatus()
    {
         _iDoEventTime = 0;
         _iEventCount = 0;
         _iDoPendingTime = 0;
         _iPendingCount = 0;
         _iDoCheckConnTime = 0;
         _iCheckConnNum = 0;

         _iRecvError = 0;
         _iRecvPack = 0;
         _iSendPack = 0;
         _iConnEnable = 0;
         _iConnClosed = 0;

         _iTimeoutConnCount = 0;

         _iTotalDoEventCount = 0;
         _iTotalConnCount = 0;
         _iDestoryConnCount = 0;
    }

    uint64_t _iDoEventTime;
    uint32_t _iEventCount;
    uint64_t _iDoPendingTime;
    uint32_t _iPendingCount;
    uint64_t _iDoCheckConnTime;
    uint32_t _iCheckConnNum;

    uint64_t _iRecvError;
    uint64_t _iRecvPack;
    uint64_t _iSendPack;
    uint64_t _iConnEnable;
    uint64_t _iConnClosed;

    uint32_t _iTimeoutConnCount;

    uint32_t _iTotalDoEventCount;
    uint32_t _iTotalConnCount;
    uint64_t _iDestoryConnCount;
 };



class EventLoop
{
public:
    EventLoop();
  ~EventLoop();
  void StartThread();
  void Stop();

  void DelTimer(uint32_t timerId);
  int RunEveryTime(int aiMs,  int aiInterval, TimerCbk timerCbk);
  int RunAfterTime(int aiMs, TimerCbk timerCbk);
  void RunInLoop(Functor lpFunctor);

  void Run();
  void AcceptRun();

  bool IsSelfThread();
  bool UpdateEvent(EventObj* lpEventObj);
  void UpdateTimeTcpConn(TcpConn* apConn, Timestamp now);
  bool FilterDisTcpConn(TcpConn* apNode, Timestamp now);

  void EnableConnect(TcpConnPtr& apConnptr);

  uint32_t GetConnNum();
  pid_t GetThreadId();
  void GetWorkStatus(WorkRunStatus& aoStatus);

//callback
//  void SetOnRawBufCbk(TOnRawBuf pOnRawBuf){_OnRawBuf = pOnRawBuf;}
  void SetDecConnNumCbk(TDecConnNumCbk pOnDecConnNum){_OnDecConnNum = pOnDecConnNum;}

  void SetOnCloseCbk(TOnClose apOnClose){_OnClose = apOnClose;}
  void SetOnErrorCbk(TOnError apOnError){_OnError = apOnError;}
  void SetOnConnectCbk(TOnConnect apOnConnect){_OnConnect = apOnConnect;}

  void OnClose(TcpConnPtr apConnPtr);
  void OnError(TcpConnPtr apConnPtr, int aiErr);
  void OnConnect(TcpConnPtr apConnPtr);

private:
  void WakeUp();
  int ReadWakeUp(Timestamp now);

  uint32_t DoPendingFunctor();
  int CheckConn(Timestamp aiTime);
  void CheckConnTimeout(uint64_t  aiDelta);

  int EnableWakeUp();

  void OnDestoryConn(TcpConnPtr  apConnPtr);
  void DestoryConn(TcpConnPtr  apConnPtr);
  void OnStatRecv(uint16_t statType, uint64_t aiNum);

  void FreeVecConn();
private:
  bool _bStop;
  std::thread* _pThread;
  int _wakeupFd;
  pid_t _threadId;
  TimerList _oTimerList;

  Epoll _epollor;
  std::vector<Functor> _vecFunctor;
  std::mutex _pendingMutex;

  DouList _douList;
 //TurnTimer _oTurnTimer;

  std::atomic<bool> _bDoEvent;
  EventObj * _pWakeUpEvent;
  std::map<uint32_t, TcpConnPtr> _mapConn;
  TDecConnNumCbk _OnDecConnNum;

  uint64_t _iTimeoutConnCount;
  uint64_t _iTotalConnCount;

  WorkRunStatus _oWorkStatus;
  std::mutex _oMutStatus;

  std::vector<TcpConnPtr> _vecFree;
  TOnClose _OnClose;
  TOnError _OnError;
  TOnConnect _OnConnect;

   uint64_t _statRecv[stMaxNum];
   bool _bAllowWakuUp;
};



#endif

