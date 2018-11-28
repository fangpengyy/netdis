#include "Log.h"
#include <time.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdarg.h>

#include "Utils.h"







Log::Log()
{
   _pFile = NULL;
}

Log::~Log()
{
  Close();
}

int Log::Init(const char* szPath, const char* szFilePreFix, uint32_t aiFileSize)
{
    if(szPath == NULL)
       return -1;
    if(szFilePreFix == NULL)
      return -2;

    _sPath = szPath;
    if(szPath[_sPath.size() - 1] != '/')
       _sPath += "/";

    _sFilePreFix = szFilePreFix;
    _iFileSize = aiFileSize;

    std::string lsFile;
    if(0 != GetFileName(lsFile))
       return -3;

    return Open(lsFile);
}


int Log::Open(const std::string asFile)
{
   if(_pFile)
   {
      fclose(_pFile);
      _pFile = NULL;
   }

   _pFile = fopen(asFile.c_str(), "wb+");
   if(_pFile==NULL)
   {
      return -1;
   }

   _iCurSize = 0;
   return 0;
}

void Log::Close()
{
   if(_pFile)
   {
      fclose(_pFile);
      _pFile = NULL;
   }
}

int Log::Write(const char* szBuf, uint32_t aiDataLen)
{
   if(_pFile == NULL || aiDataLen == 0)
      return -1;

   int liLen = fwrite(szBuf, sizeof(char), aiDataLen,  _pFile);
   if((uint32_t)liLen == aiDataLen)
   {
      _iCurSize += liLen;
      fflush(_pFile);
      return 0;
   }

   return -2;
}

const std::string Log::GetTimeStr()
{
   time_t t = time(0);
   struct tm* pTm = gmtime(&t);
   char szBuf[64] = {0};

   sprintf(szBuf, "%d%02d%02d-%02d%02d%02d", 1900 + pTm->tm_year, 1 + pTm->tm_mon, pTm->tm_mday,
      pTm->tm_hour, pTm->tm_min, pTm->tm_sec);

   return szBuf;
}

int Log::GetFileName(std::string& sFileName)
{
    sFileName = _sPath + _sFilePreFix + GetTimeStr() + ".log";
    return 0;
}

bool Log::ChangeFile()
{
   if(_iCurSize >= _iFileSize)
     return true;

   return false;
}

void Log::Flush()
{
  if(_pFile)
    fflush(_pFile);
}


//---------------------

CBuffer::CBuffer(): _iSize(0), _iReadPos(0), _iWritePos(0),_pBuf(0)
{

}

CBuffer::~CBuffer()
{
  if(_pBuf)
  {
     delete[] _pBuf;
     _pBuf = NULL;
     _iSize = 0;
  }
}

int CBuffer::Init(uint32_t aiSize)
{
   _iSize = aiSize;
   _pBuf = new char[aiSize];
   return _pBuf != NULL ? 0 : -1;
}

int CBuffer::Append(const char* szBuf, uint32_t aiDataLen)
{
    if(aiDataLen > LeftSize())
      return -1;

    memcpy(_pBuf + _iWritePos, szBuf, aiDataLen);
    _iWritePos += aiDataLen;

    return 0;
}

void CBuffer::Reset()
{
  _iWritePos = 0;
  _iReadPos = 0;
}

//CQueue---------------------------

CQueue::CQueue()
{

}

CQueue::~CQueue()
{
}

void CQueue::Clean()
{
    std::lock_guard<std::mutex> locker(_mutex);

    std::list<CBuffer*>::iterator it = _list.begin();
    for(; it != _list.end(); it++)
        delete *it;
    _list.clear();
}

void CQueue::Add(CBuffer* apBuffer)
{
    std::lock_guard<std::mutex> locker(_mutex);
    _list.push_back(apBuffer);
    _cond.notify_one();
}


CBuffer* CQueue::Get()
{
   std::lock_guard<std::mutex> locker(_mutex);
   if(_list.empty())
     return NULL;

   CBuffer* pBuf =  _list.front();
   _list.pop_front();
   return pBuf;
}

CBuffer* CQueue::GetWait()
{
   std::unique_lock<std::mutex> locker(_mutex);
   while(_list.empty())
     _cond.wait(locker);

   CBuffer* pBuf =  _list.front();
   _list.pop_front();
   return pBuf;
}

uint32_t CQueue::Size()
{
   return _list.size();
}


//Loger----------------------------

Loger::Loger()
{
   _pThread = NULL;
   _bStop = false;
}

Loger::~Loger()
{

}

int Loger::Init(const char* szPath,  const char* szFilePreFix, uint32_t aiFileSize)
{
   _log.Init(szPath, szFilePreFix, aiFileSize);
   _iBufSize = 1024 * 1024;

   for(int i = 0; i < 3; i++)
   {
      CBuffer* lpBuf = new CBuffer();
      if(lpBuf == NULL)
        return -1;

      lpBuf->Init(_iBufSize);
      _freeList.Add(lpBuf);
   }

   _pThread = new std::thread(&Loger::OnProc, this);
   if(_pThread)
     return 0;

   return -2;
}

void Loger::UnInit()
{
    if(_pThread)
    {
        _bStop = true;
        _pThread->join();

        delete _pThread;
        _pThread = NULL;
    }

    _freeList.Clean();
    _workList.Clean();

    {
       std::lock_guard<std::mutex> locker(_mutex);
       if(_pCurBuffer)
       {
            delete _pCurBuffer;
            _pCurBuffer = NULL;
       }
    }
}

void Loger::OnProc()
{
   int i = 1;

   uint32_t liTickMSec = 0;

   while(!_bStop)
   {
      if(_log.ChangeFile())
      {
         std::string sFile;
         _log.GetFileName(sFile);
         if(0 != _log.Open(sFile))
         {
             std::cout<<__FUNCTION__<<" open "<<sFile<<" err="<<errno<<std::endl;
             _bStop = true;
             break;
         }
      }

      CBuffer* lpBuf = _workList.Get();
      if(lpBuf)
      {
          if(0 !=  _log.Write(lpBuf->Buf(), lpBuf->DataLen()))
          {
             _bStop = true;
             break;
          }

          lpBuf->Reset();
          _freeList.Add(lpBuf);

          liTickMSec = 0;
          std::cout<<" work size=" <<_workList.Size()<<" freeList size=" <<_freeList.Size()<<"  write block index=" << i++ << std::endl;
      }
      else
      {
          liTickMSec++; //10ms
          if(liTickMSec >= 1000) //10sec
          {
             {
                std::lock_guard<std::mutex> locker(_mutex);
                if(_pCurBuffer && _pCurBuffer->DataLen())
                {
                    _log.Write(_pCurBuffer->Buf(), _pCurBuffer->DataLen());
                    _pCurBuffer->Reset();

                    //std::cout<<__FUNCTION__<<"  flush file ,work size=" <<_workList.Size()<<" freeList size=" <<_freeList.Size()<<"  write block index=" << i++ << std::endl;
                }
             }
             liTickMSec = 0;
          }
      }
      usleep(10000);
   }

}

int Loger::Write(const char* asBuf, uint32_t aiDataLen)
{
   if(_bStop)
     return -1;

   {
      std::lock_guard<std::mutex> locker(_mutex);

      do
      {
         if(_pCurBuffer == NULL)
             _pCurBuffer = _freeList.GetWait();

         int liRet = _pCurBuffer->Append(asBuf, aiDataLen);
         if(liRet == -1)
         {
            _workList.Add(_pCurBuffer);
            _pCurBuffer = NULL;
            continue;
         }
      } while(0);
   }

   return 0;
}


int Loger::WLog(const char* szFile, int aiLen, const char* asFuncName, uint32_t aiLevel, const char* asFormat, ...)
{
   va_list alist;
   va_start(alist, asFormat);

   char lpBuf[2048] = {0};
   int liSize = sprintf(lpBuf, "%s %s:%d %d %d %s ", _log.GetTimeStr().c_str(), szFile, aiLen, (int)gettid(), aiLevel, asFuncName);

   int liLeft = sizeof(lpBuf) - liSize;
   int liLen = vsnprintf(lpBuf + liSize, liLeft,  asFormat, alist);

   if(liLen > 0 && liLen < liLeft)
   {
       strcat(lpBuf, "\n");
       Write(lpBuf, liLen + liSize + 1);
   }
   else
   {
       const int def_size = 8192;

       char* lpNewBuf = new char[def_size];
       liSize = sprintf(lpNewBuf, "%s %s:%d %d %d %s", _log.GetTimeStr().c_str(), szFile, aiLen, (int)gettid(), aiLevel, asFuncName);

       liLen = vsnprintf(lpNewBuf + liSize, def_size - liSize, asFormat, alist);
       if(liLen > 0)
       {
          strcat(lpNewBuf, "\n");
          Write(lpNewBuf, liLen + liSize + 1);
       }

       delete[] lpNewBuf;
       lpNewBuf = NULL;
   }

   va_end(alist);

   return liLen > 0 ? 0 : -1;
}


