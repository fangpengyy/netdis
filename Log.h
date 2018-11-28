#ifndef __LOG_H__
#define __LOG_H__

#include <thread>
#include <string>
#include <stdio.h>
#include <mutex>
#include <condition_variable>
#include <list>


enum ENUM_LOG_LEVEL{INFO=0, ERROR, WARN};



class Log
{
public:
   Log();
   ~Log();

   int Init(const char* szPath,  const char* szFilePreFix, uint32_t aiFileSize);
   //void UnInit();

   int Open(const std::string asFile);   
   void Close();   
   
   int Write(const char* szBuf, uint32_t aiDataLen); 
   int GetFileName(std::string& sFileName);

   bool ChangeFile();
   void Flush();

   const std::string GetTimeStr();
 
private:
   std::string _sPath;
   std::string _sFilePreFix;
   uint32_t _iFileSize;
    
   FILE* _pFile;
   uint32_t _iCurSize;
};



class CBuffer
{
public:
  CBuffer();
  ~CBuffer();
  int Init(uint32_t aiSize);

  int Append(const char* szBuf, uint32_t aiDataLen);
  
  uint32_t LeftSize(){return _iSize - _iWritePos;}  
  uint32_t DataLen(){return _iWritePos;}
  const char* Buf(){return _pBuf;}
  
  void Reset();

private:
  uint32_t _iSize;
  uint32_t _iReadPos;
  uint32_t _iWritePos;
  char* _pBuf;

};


class CQueue
{
public:
  CQueue();
  ~CQueue();
  

  void Add(CBuffer* apBuffer);
  CBuffer* Get();
  uint32_t Size();
  CBuffer* GetWait();
  void Clean();

private:
   std::list<CBuffer*> _list;
   std::mutex _mutex;
   std::condition_variable _cond;
};



class Loger
{
public:
    static Loger& Instance()
   {
      static Loger m_loger;
      return m_loger;
   }

   Loger();
   ~Loger();
   
   int Init(const char* szPath,  const char* szFilePreFix, uint32_t aiFileSize);
   void UnInit();

   uint32_t FreeSize(){return _freeList.Size();}
   uint32_t WorkSize(){return _workList.Size();}

   int WLog(const char* szFile, int aiLen, const char* asFuncName, uint32_t aiLevel, const char* asFormat, ...);
   
private:
   void OnProc();
   int Write(const char* asBuf, uint32_t aiDataLen);
   

private:
   CQueue _freeList;
   CQueue _workList;

   CBuffer* _pCurBuffer;
   
   std::mutex _mutex;
   std::condition_variable _notify;    
   
   Log _log;
   uint32_t _iBufSize;
   bool _bStop;

   std::thread* _pThread;
};


#define LOG_INIT(path, prefix, fileSize)\
   Loger::Instance().Init(path, prefix, fileSize);

#define LOG(level, strFormat, ...)\
   Loger::Instance().WLog(__FILE__, __LINE__, __FUNCTION__, level, strFormat, ##__VA_ARGS__);

#define LOG_UNINIT()\
  Loger::Instance().UnInit();

#define LOG_INFO(strFormat, ...)\
   Loger::Instance().WLog(__FILE__, __LINE__, __FUNCTION__, 0, strFormat, ##__VA_ARGS__);

#define LOG_ERR(strFormat, ...)\
   Loger::Instance().WLog(__FILE__, __LINE__, __FUNCTION__, 1, strFormat, ##__VA_ARGS__);

#define LOG_WARN(strFormat, ...)\
   Loger::Instance().WLog(__FILE__, __LINE__, __FUNCTION__, 2, strFormat, ##__VA_ARGS__);



#endif
