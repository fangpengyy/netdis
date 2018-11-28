#ifndef __DATASTREAM_H__
#define __DATASTREAM_H__
#include <string.h>

class DataStream
{
public:
  DataStream():_pBuf(0), _iBufSize(0), _iDataLen(0)
  {
     
  }

  ~DataStream()
  {
     if(_pBuf)
     {
         delete[] _pBuf;
         _pBuf = nullptr;
     }
     _iDataLen = 0;
  }

  char* Buf(){return _pBuf;}
  uint32_t BufSize(){return _iBufSize;}  
  
  uint32_t DataLen(){return _iDataLen;}
 
  void CopyData(const char* pszBuf, uint32_t aiDataLen)
  {
      _iDataLen = 0;
      if(pszBuf == nullptr || aiDataLen == 0)
         return;     

      if(aiDataLen > _iBufSize)
      {
         _iBufSize = aiDataLen; 
         char* lpBuf = new char[_iBufSize];
         
         if(_pBuf)
         {
            delete[] _pBuf;
            _pBuf = nullptr;
         }
         _pBuf = lpBuf;
      }
      
      memcpy(_pBuf, pszBuf, aiDataLen);
      _iDataLen = aiDataLen;
  }

private:
  char* _pBuf;
  uint32_t _iBufSize;
  uint32_t _iDataLen;
};




#endif
