#include "ProtoPack.h"
#include "NetBytes.h"
#include <iostream>



#define DEF_PACK(packName)\
case DEF_FUNC_##packName:\
    {\
        Packet* pPack = new Pack_##packName();\
        return pPack;\
    }


//PackHead-------------------------

 bool PackHead::IsValid()
 {
     return flag == TCP_HEAD_FLAG;
 }

void PackHead::SetHead(uint32_t aiSvrTypeId, uint32_t aiSeqId, /* uint32_t aiVersion,*/
                        uint8_t aiZipType, uint8_t aiEncrypt)
{
    svrTypeId = aiSvrTypeId;
    seqId = aiSeqId;
    //version = aiVersion;
    zipType = aiZipType;
    encrypt = aiEncrypt;
}

int PackHead::Pack(char* szBuf, uint32_t& aiLen)
{
    if(aiLen < sizeof(*this))
        return -1;

    NetBytes bytes(szBuf,  aiLen);
    bytes << flag<<svrTypeId<<seqId<<packLen<<funcId<<zipType<<encrypt;
    aiLen = bytes.GetDataLen();
    return 0;
}

void PackHead::UnPack(char* szBuf, uint32_t aiLen)
{
    NetBytes bytes(szBuf,  aiLen);
    bytes>>flag>>svrTypeId>>seqId>>packLen>>funcId>>zipType>>encrypt;
}



Packet* GetPackObj(uint16_t funcId)
{
    switch(funcId)
   {
        DEF_PACK(TickHeart)
        DEF_PACK(Msg)


   }
   return NULL;
}


//Stru_Pack_TickHeart--------

void Pack_TickHeart::Pack(char* szBuf, uint32_t& aiLen)
{
     NetBytes bytes(szBuf, aiLen);
     bytes<<funcId;
     aiLen = bytes.GetDataLen();
}

void Pack_TickHeart::UnPack(char* szBuf, uint32_t aiLen)
{
     NetBytes bytes(szBuf, aiLen);
     bytes>>funcId;
}
void Pack_TickHeart::OnHandle(TcpConnPtr& apConn, uint16_t aiFuncId, char* szBuf, uint32_t aiLen)
{
     UnPack(szBuf, aiLen);
     if(aiFuncId != funcId)
	 {
	 	 LOG_ERR(" recv funcId=%d, unpack funcId=%d", aiFuncId, funcId);
	 	 return;
	 }
#ifdef DEF_COUT
    std::cout<< __FUNCTION__<<"  funcId="<<funcId<<"\n";
#endif
}

///

Pack_Msg::~Pack_Msg()
{
    FreeBuf();
}

void Pack_Msg::SetMsg(const char* apBuf, uint16_t aiLen)
{
    if(apBuf == NULL || aiLen == 0)
        return;

    FreeBuf();

    _iDataLen = aiLen;
    _pBuf = new char[aiLen];
    memcpy(_pBuf, apBuf, aiLen);
}

void Pack_Msg::Pack(char* szBuf, uint32_t& aiLen)
{
     NetBytes bytes(szBuf, aiLen);
     bytes<<funcId<<_iDataLen;
     bytes.Append(_pBuf, _iDataLen);
     aiLen = bytes.GetDataLen();
}

void Pack_Msg::UnPack(char* szBuf, uint32_t aiLen)
{
     NetBytes bytes(szBuf, aiLen);
     uint32_t liDataLen = 0;
     bytes>>funcId>>liDataLen;

     _pBuf = NewBuf(liDataLen);
     bytes.GetBuf(_pBuf,  liDataLen);
     _iDataLen = liDataLen;
}

void Pack_Msg::OnHandle(TcpConnPtr& apConn, uint16_t aiFuncId, char* szBuf, uint32_t aiLen)
{
     UnPack(szBuf, aiLen);
     if(aiFuncId != funcId)
	 {
	 	//LOG_ERR(" recv funcId=%d, unpack funcId=%d", aiFuncId, funcId);
	 	return;
#ifdef DEF_COUT
    std::cout<< __FUNCTION__<<"  funcId="<<funcId<<"  len= " <<data_len<<"  ="<<pBuf<<"\n";
#endif
	 }
	//  if(_iDataLen > 0)
     //         printf("len=%d: %s\n", _iDataLen, _pBuf);
}

char* Pack_Msg::NewBuf(uint16_t aiLen)
{
    if(_iDataLen < aiLen)
    {
        FreeBuf();
        _pBuf = new char[aiLen];
    }
    return _pBuf;
}

void Pack_Msg::FreeBuf()
{
     if(_pBuf)
    {
         delete[] _pBuf;
         _pBuf = nullptr;
         _iDataLen = 0;
    }
}
