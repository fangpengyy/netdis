#include "Utils.h"
#include "Log.h"
#include <sys/time.h>
#include <iostream>
#include <chrono>


namespace Global
{
uint32_t GetCurSvrType()
{
    return  DEF_SVRTYPE_TCPSVER;
}

}

static const int kMicroSecondsPerSecond = 1000 * 1000;

//微妙
Timestamp GetTimestamp()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}


struct timespec MicTimeValue(Timestamp when)
{
    int64_t microseconds =  when - GetTimestamp();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % kMicroSecondsPerSecond) * 1000);
    return ts;
}

int CreateTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_ERR("Failed in timerfd_create");
    }
    return timerfd;
}

void ReadTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    //LOG_INFO("TimerQueue::handleRead() " << howmany << " at " << now.toString();
    if (n != sizeof howmany)
        LOG_ERR("read ret=%d err=%d", n, errno);
}

void ResetTimeFd(int timerfd, Timestamp stamp)
{
    struct itimerspec loValue, loOldValue;
    bzero(&loValue, sizeof loValue);
    bzero(&loOldValue, sizeof loOldValue);

    //std::cout<<"stamp=" <<stamp << "GetTimestamp="<<GetTimestamp()<<  "  aiSec=" <<aiSec<<"  nsec="<<liNsec <<" ----\n";

    loValue.it_value = MicTimeValue(stamp);

    int ret = ::timerfd_settime(timerfd, 0, &loValue, &loOldValue);
    if (ret)
    {
        LOG_INFO("timerfd_settime");
    }
}


void trim(std::string &s)
{
    if (s.empty())
        return;
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
}


const char* GetRespStatusDesc(uint32_t aiStatus)
{
    switch (aiStatus)
    {
    case 100:
        return "Continue";
    case 101:
        return "Switching Protocols";
    case 102:
        return "Processing";
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 203:
        return "Non-Authoritative Information";
    case 204:
        return "No Content";
    case 205:
        return "Reset Content";
    case 206:
        return "Partial Content";
    case 207:
        return "Multi-Status";
    case 208:
        return "Already Reported";
    case 226:
        return "IM Used";
    case 300:
        return "Multiple Choices";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 305:
        return "Use Proxy";
    case 307:
        return "Temporary Redirect";
    case 308:
        return "Permanent Redirect";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 402:
        return "Payment Required";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 406:
        return "Not Acceptable";
    case 407:
        return "Proxy Authentication Required";
    case 408:
        return "Request Timeout";
    case 409:
        return "Conflict";
    case 410:
        return "Gone";
    case 411:
        return "Length Required";
    case 412:
        return "Precondition Failed";
    case 413:
        return "Payload Too Large";
    case 414:
        return "URI Too Long";
    case 415:
        return "Unsupported Media Type";
    case 416:
        return "Range Not Satisfiable";
    case 417:
        return "Expectation Failed";
    case 421:
        return "Misdirected Request";
    case 422:
        return "Unprocessable Entity";
    case 423:
        return "Locked";
    case 424:
        return "Failed Dependency";
    case 426:
        return "Upgrade Required";
    case 428:
        return "Precondition Required";
    case 429:
        return "Too Many Requests";
    case 431:
        return "Request Header Fields Too Large";
    case 451:
        return "Unavailable For Legal Reasons";
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "Gateway Timeout";
    case 505:
        return "HTTP Version Not Supported";
    case 506:
        return "Variant Also Negotiates";
    case 507:
        return "Insufficient Storage";
    case 508:
        return "Loop Detected";
    case 510:
        return "Not Extended";
    case 511:
        return "Network Authentication Required";
    //---------------
    case 1002:
        return "protocol error";
    case 1003:
        return "send data type error";
    }
    return "Unknown";
}


//NotifyWait---------------------------
void NotifyWait::Notify()
{
    std::unique_lock<std::mutex> loLocker(_oMutex);
    _bReady = true;
    _oCond.notify_one();
}

bool NotifyWait::Wait(int aiMsTime)
{
    auto const liTimeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(aiMsTime);
    std::unique_lock<std::mutex> loLocker(_oMutex);
    while(!_bReady)
    {
        if(_oCond.wait_until(loLocker, liTimeout) == std::cv_status::timeout)
            return false;
    }
    return true;
}

TimePoint NowTime()
{
    return std::chrono::high_resolution_clock::now();
}

uint64_t DiffTime(TimePoint& aiBeginTime,   TimePoint& aiEndTime)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(aiEndTime - aiBeginTime).count();
}


