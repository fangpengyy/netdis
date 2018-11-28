#include "TimeoutConns.h"
#include "EventLoop.h"


//TimeoutConnMng---------------
 TimeoutConns::TimeoutConns()
 {
        _pEventLoop = nullptr;
 }

TimeoutConns::~TimeoutConns()
{
     _mapTimeout.clear();
}

void TimeoutConns::SetEventLoop(EventLoop* pEventLoop)
{
        _pEventLoop = pEventLoop;
}

void TimeoutConns::UpdateTimeout(TcpConn* apConn, Timestamp prevTimestamp)
{
        MapTimeoutConn::iterator it = _mapTimeout.begin();
        while(it != _mapTimeout.end())
        {
            MapTimeoutConn::iterator itCur = it++;
             if(itCur->first == prevTimestamp)
            {
                   if(apConn->GetId() == itCur->second->GetId())
                   {
                        std::cout<< __func__<<"  erase  conn id="<<itCur->second->GetId()<<"\n";
                         _mapTimeout.erase(itCur);
                         break;
                   }
            }
        }
        _mapTimeout.insert(std::make_pair(apConn->GetLastTimestamp(), apConn));
}

void TimeoutConns::CheckConn(Timestamp now)
{
        //std::cout<<__func__<<"  thid="<<gettid()<<"  conn size="<<_mapTimeout.size()<<"\n";
        if(_pEventLoop == nullptr)
            return;

        MapTimeoutConn::iterator it = _mapTimeout.begin();
        while(it != _mapTimeout.end())
        {
            MapTimeoutConn::iterator itCur = it++;
            TcpConn* lpConn = itCur->second;
            _mapTimeout.erase(itCur);

            if(lpConn->IsDisConnect())
	        {
		            _pEventLoop->RunInLoop(std::bind(&TcpConn::Destory,  lpConn)) ;
	        }
             else   if(now >  lpConn->GetLastTimestamp()  + DEF_CONN_TIME_OUT)
             {
                     lpConn->HandleClose();
                     _pEventLoop->RunInLoop(std::bind(&TcpConn::Destory,  lpConn)) ;
              }
             else
                  break;
        }
}
