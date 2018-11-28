
#ifndef   __UTILS_H__
#define   __UTILS_H__

#include <sys/syscall.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "Log.h"


//#define  DEF_COUT    1

//服务类型
#define  DEF_SVRTYPE_TCPSVER         1
#define  DEF_SVRTYPE_RPC         2
#define  DEF_SVRTYPE_HTTP          3
#define    DEF_SVRTYPE_WEBSOCKET          4

namespace Global{

  extern  uint32_t GetCurSvrType() ;

}


#define gettid()  syscall(__NR_gettid)

class TcpConn;
typedef std::shared_ptr<TcpConn> TcpConnPtr;




typedef uint64_t  Timestamp;

#define DEF_CONN_TIME_OUT  (1000 * 1000 *90) // 120)


 enum StatType{stConn=0/*激活连接*/, stClosed,  stDestory, stRecvPack/*接收一个包*/,  stSendPack, stRecvError, stMaxNum };

extern Timestamp GetTimestamp();
extern int CreateTimerfd();
extern void ReadTimerfd(int timerfd, Timestamp now);
extern void ResetTimeFd(int timerfd, Timestamp expiration);

extern void trim(std::string &s);
extern const char* GetRespStatusDesc(uint32_t aiStatus);

typedef std::chrono::high_resolution_clock::time_point  TimePoint;

extern TimePoint  NowTime();
extern uint64_t DiffTime(TimePoint& aiBeginTime,   TimePoint& aiEndTime);


class NotifyWait
{
public:
    NotifyWait():_bReady(false) {   }
    void Notify();
    bool Wait(int aiMsTime);
private:
    bool _bReady;
    std::mutex _oMutex;
    std::condition_variable  _oCond;
};


#endif
