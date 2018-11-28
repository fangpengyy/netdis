#include "Pack.h"
#include "NetBytes.h"
#include <iostream>


//PackHead-------------------------


//PackHead-------------------------

 bool PackHead::IsValid()
 {
     return flag == TCP_HEAD_FLAG;
 }

void PackHead::SetHead(uint32_t aiPackTypeId, uint8_t aiDataFlag)
{
    packTypeId = aiPackTypeId;
    dataFlag = aiDataFlag;
}

int PackHead::ParseRouteData(char* szBuf, uint32_t aiLen)
{
    if(aiLen < sizeof(*this))
        return -1;

    NetBytes bytes(szBuf,  aiLen);
    bytes>>flag>>packTypeId>>sessionId>>userId>>packLen;
    return 0;
}

void  PackHead::SetSessionId(char* apBuf, uint64_t aiSessionId)
{
     *(apBuf + 8) = hton64(aiSessionId);
}

int PackHead::Pack(char* szBuf, uint32_t& aiLen)
{
    if(aiLen < sizeof(*this))
        return -1;

    NetBytes bytes(szBuf,  aiLen);
    bytes << flag<<packTypeId<<sessionId<<userId<<packLen<<dataFlag<<crc32;
    aiLen = bytes.GetDataLen();
    return 0;
}

void PackHead::UnPack(char* szBuf, uint32_t aiLen)
{
    printf("PackHead::UnPack  %d, %s\n", aiLen, szBuf);
    NetBytes bytes(szBuf,  aiLen);
    bytes>>flag>>packTypeId>>sessionId>>userId>>packLen>>dataFlag>>crc32;
}
