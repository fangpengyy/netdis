#ifndef __REQTITLE_H__
#define __REQTITLE_H__
#include <unistd.h>
#include <stdint.h>
#include "../Pack.h"
#include "HandleCustomPackMgr.h"
#include "DefPackNo.h"
#include "../Utils.h"


class Pack_GetTitle: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
  int32_t id;
  std::string sTitle;
public:
   Pack_GetTitle()
    {
       packTypeId =DEF_FUNC_GetTitle;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_GetTitleHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_GetTitle(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_SetTitle: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
  int32_t id;
  std::string sTitle;
public:
   Pack_SetTitle()
    {
       packTypeId =DEF_FUNC_SetTitle;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_SetTitleHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_SetTitle(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_Msg: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
      std::string sMsg;
public:
   Pack_Msg()
    {
       packTypeId =DEF_FUNC_Msg;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_MsgHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_Msg(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_MsgEx: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
      uint32_t msgId;
      std::string sMsgEx;
public:
   Pack_MsgEx()
    {
       packTypeId =DEF_FUNC_MsgEx;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_MsgExHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_MsgEx(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_ReqAddValue: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
      int32_t a1;
      int32_t a2;
public:
   Pack_ReqAddValue()
    {
       packTypeId =DEF_FUNC_ReqAddValue;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_ReqAddValueHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_ReqAddValue(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_RespAddValue: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
      int32_t val;
public:
   Pack_RespAddValue()
    {
       packTypeId =DEF_FUNC_RespAddValue;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_RespAddValueHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_RespAddValue(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};

class Pack_TestResponse: public IPacket
{
 private:
   TOnCustomPackHandle _OnCustomPackHandle;
public:
      std::string  sVal;
public:
   Pack_TestResponse()
    {
       packTypeId =DEF_FUNC_TestResponse;
       _OnCustomPackHandle = std::bind(&HandleCustomPackMgr::OnPack_TestResponseHandle, HandleCustomPackMgr::GetInstance(), std::placeholders::_1, std::placeholders::_2);
   };
   virtual ~Pack_TestResponse(){};
  void Pack(char* szBuf, uint32_t& aiLen);
   void UnPack(char* szBuf, uint32_t aiLen);
   void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen);
};


#endif