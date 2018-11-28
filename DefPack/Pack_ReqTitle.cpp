#include "Pack_ReqTitle.h"
#include "../NetBytes.h"
#include <iostream>



void Pack_GetTitle::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<id<<sTitle;
   aiLen = loBytes.GetDataLen();
}

void Pack_GetTitle::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>id>>sTitle;
}

void Pack_GetTitle::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
{
   UnPack(szBuf, aiLen);
   if (aipackTypeId != packTypeId)
   {
      LOG_ERR(" recv packTypeId=%d, unpack packTypeId=%d", aipackTypeId, packTypeId);
      return;
   }
   // here write code ...
   if(_OnCustomPackHandle)
        _OnCustomPackHandle(apConn, this);
}



void Pack_SetTitle::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<id<<sTitle;
   aiLen = loBytes.GetDataLen();
}

void Pack_SetTitle::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>id>>sTitle;
}

void Pack_SetTitle::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
{
   UnPack(szBuf, aiLen);
   if (aipackTypeId != packTypeId)
   {
      LOG_ERR(" recv packTypeId=%d, unpack packTypeId=%d", aipackTypeId, packTypeId);
      return;
   }
   // here write code ...
   if(_OnCustomPackHandle)
        _OnCustomPackHandle(apConn, this);
}



void Pack_Msg::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<sMsg;
   aiLen = loBytes.GetDataLen();
}

void Pack_Msg::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>sMsg;
}

void Pack_Msg::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
{
   UnPack(szBuf, aiLen);
   if (aipackTypeId != packTypeId)
   {
      LOG_ERR(" recv packTypeId=%d, unpack packTypeId=%d", aipackTypeId, packTypeId);
      return;
   }
   // here write code ...
   if(_OnCustomPackHandle)
        _OnCustomPackHandle(apConn, this);
}



void Pack_MsgEx::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<msgId<<sMsgEx;
   aiLen = loBytes.GetDataLen();
}

void Pack_MsgEx::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>msgId>>sMsgEx;
}

void Pack_MsgEx::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
{
   UnPack(szBuf, aiLen);
   if (aipackTypeId != packTypeId)
   {
      LOG_ERR(" recv packTypeId=%d, unpack packTypeId=%d", aipackTypeId, packTypeId);
      return;
   }
   // here write code ...
   if(_OnCustomPackHandle)
        _OnCustomPackHandle(apConn, this);
}



void Pack_ReqAddValue::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<a1<<a2;
   aiLen = loBytes.GetDataLen();
}

void Pack_ReqAddValue::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>a1>>a2;
}

void Pack_ReqAddValue::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
{
   UnPack(szBuf, aiLen);
   if (aipackTypeId != packTypeId)
   {
      LOG_ERR(" recv packTypeId=%d, unpack packTypeId=%d", aipackTypeId, packTypeId);
      return;
   }
   // here write code ...
   if(_OnCustomPackHandle)
        _OnCustomPackHandle(apConn, this);
}



void Pack_RespAddValue::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<val;
   aiLen = loBytes.GetDataLen();
}

void Pack_RespAddValue::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>val;
}

void Pack_RespAddValue::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
{
   UnPack(szBuf, aiLen);
   if (aipackTypeId != packTypeId)
   {
      LOG_ERR(" recv packTypeId=%d, unpack packTypeId=%d", aipackTypeId, packTypeId);
      return;
   }
   // here write code ...
   if(_OnCustomPackHandle)
        _OnCustomPackHandle(apConn, this);
}



void Pack_TestResponse::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<sVal;
   aiLen = loBytes.GetDataLen();
}

void Pack_TestResponse::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>sVal;
}

void Pack_TestResponse::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
{
   UnPack(szBuf, aiLen);
   if (aipackTypeId != packTypeId)
   {
      LOG_ERR(" recv packTypeId=%d, unpack packTypeId=%d", aipackTypeId, packTypeId);
      return;
   }
   // here write code ...
   if(_OnCustomPackHandle)
        _OnCustomPackHandle(apConn, this);
}



