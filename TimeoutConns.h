#ifndef  __TIMEOUTCONNS_H__
#define  __TIMEOUTCONNS_H__

#include <stdint.h>
#include <map>
#include "TcpConn.h"
#include "Utils.h"

class EventLoop;

typedef   std::multimap<Timestamp, TcpConn*>   MapTimeoutConn;

class  TimeoutConns
{
public:
    TimeoutConns();
    ~TimeoutConns();
    void SetEventLoop(EventLoop* pEventLoop);
    void UpdateTimeout(TcpConn* apConn,  Timestamp prevTimestamp);
    void CheckConn(Timestamp now);

private:
     MapTimeoutConn  _mapTimeout;
     EventLoop* _pEventLoop;
};


#endif
