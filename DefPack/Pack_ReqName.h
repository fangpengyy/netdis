#ifndef __REQNAME_H__
#define __REQNAME_H__
#include <unistd.h>
#include <stdint.h>
#include "../Pack.h"
#include "HandleCustomPackMgr.h"
#include "DefPackNo.h"
#include "../Utils.h"


class Pack_TickHeart: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
public:
   Pack_TickHeart()
    {
       packTypeId =DEF_FUNC_TickHeart;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_TickHeartHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_TickHeart(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_ReqName: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
    int32_t id;
    uint32_t flag;
    std::string sName;
public:
   Pack_ReqName()
    {
       packTypeId =DEF_FUNC_ReqName;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_ReqNameHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_ReqName(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_ReqName_2: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
    int32_t id;
    uint32_t flag;
    uint16_t flag16;
    bool kk;
    uint8_t flag8;
    int16_t soce;
    int8_t up;
    std::string sName;
public:
   Pack_ReqName_2()
    {
       packTypeId =DEF_FUNC_ReqName_2;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_ReqName_2Handle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_ReqName_2(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_GetVal: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
  int64_t id;
  int32_t order;
public:
   Pack_GetVal()
    {
       packTypeId =DEF_FUNC_GetVal;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_GetValHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_GetVal(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_SetVal: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
  uint32_t id;
  bool bOk;
public:
   Pack_SetVal()
    {
       packTypeId =DEF_FUNC_SetVal;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_SetValHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_SetVal(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_Test: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
      std::string sTalk;
      uint16_t seq;
public:
   Pack_Test()
    {
       packTypeId =DEF_FUNC_Test;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_TestHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_Test(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};


#endif