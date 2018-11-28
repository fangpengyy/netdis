#include "Pack_ReqName.h"
#include "../NetBytes.h"
#include <iostream>



void Pack_TickHeart::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes;
   aiLen = loBytes.GetDataLen();
}

void Pack_TickHeart::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes;
}

void Pack_TickHeart::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
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



void Pack_ReqName::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<id<<flag<<sName;
   aiLen = loBytes.GetDataLen();
}

void Pack_ReqName::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>id>>flag>>sName;
}

void Pack_ReqName::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
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



void Pack_ReqName_2::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<id<<flag<<flag16<<kk<<flag8<<soce<<up<<sName;
   aiLen = loBytes.GetDataLen();
}

void Pack_ReqName_2::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>id>>flag>>flag16>>kk>>flag8>>soce>>up>>sName;
}

void Pack_ReqName_2::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
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



void Pack_GetVal::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<id<<order;
   aiLen = loBytes.GetDataLen();
}

void Pack_GetVal::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>id>>order;
}

void Pack_GetVal::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
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



void Pack_SetVal::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<id<<bOk;
   aiLen = loBytes.GetDataLen();
}

void Pack_SetVal::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>id>>bOk;
}

void Pack_SetVal::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
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



void Pack_Test::Pack(char* szBuf, uint32_t& aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes<<sTalk<<seq;
   aiLen = loBytes.GetDataLen();
}

void Pack_Test::UnPack(char* szBuf, uint32_t aiLen)
{
   NetBytes loBytes(szBuf, aiLen);
   loBytes>>sTalk>>seq;
}

void Pack_Test::OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen)
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



