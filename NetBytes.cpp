#include "NetBytes.h"
#include <arpa/inet.h>
#include <string.h>



uint64_t hton64(uint64_t value)
{
    unsigned char* pBuf = (unsigned char*)&value;
    unsigned char bigBuf[8];
    bigBuf[0] = pBuf[7];
    bigBuf[1] = pBuf[6];
    bigBuf[2] = pBuf[5];
    bigBuf[3] = pBuf[4];
    bigBuf[4] = pBuf[3];
    bigBuf[5] = pBuf[2];
    bigBuf[6] = pBuf[1];
    bigBuf[7] = pBuf[0];
    return *(uint64_t*)bigBuf;
}

uint64_t ntoh64(uint64_t value)
{
    return hton64(value);
}


//NetBytes--------------


NetBytes::NetBytes(char* szBuf,  uint32_t aiBufSize)
{
    _pBuf = szBuf;
    _iBufSize = aiBufSize;
    _iWtPos = 0;
    _iRdPos = 0;
}


NetBytes& NetBytes::operator << (uint8_t value)
{
    *(uint8_t*)(_pBuf + _iWtPos) = value;
    _iWtPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator << (uint16_t  value)
{
    *(uint16_t*)(_pBuf + _iWtPos) = htons(value);
    _iWtPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator << (uint32_t  value)
{
    *(uint32_t*)(_pBuf + _iWtPos) = htonl(value);
    _iWtPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator << (uint64_t  value)
{
    *(uint64_t*)(_pBuf + _iWtPos) = hton64(value);
    _iWtPos += sizeof value;
    return *this;
}


int NetBytes::Append (const char*  value, uint32_t aiLen)
{
    if(_iWtPos + aiLen > _iBufSize)
        return -1;

    memcpy(_pBuf +_iWtPos + sizeof (uint32_t) ,  value,  aiLen);
    *(uint32_t*)(_pBuf + _iWtPos) = htonl(aiLen);
    _iWtPos += aiLen + sizeof (uint32_t);
    return 0;
}

NetBytes& NetBytes::operator << (std::string value)
{
    Append(value.c_str(), (uint32_t)value.size());
    return *this;
}

NetBytes& NetBytes::operator >> (uint8_t& value)
{
    value = *(uint8_t*)(_pBuf + _iRdPos);
    _iRdPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator >> (uint16_t & value)
{
    value =  ntohs(*(uint16_t*)(_pBuf + _iRdPos));
    _iRdPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator >> (uint32_t & value)
{
    value = ntohl(*(uint32_t*)(_pBuf + _iRdPos));
    _iRdPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator >> (uint64_t & value)
{
    value = ntoh64(*(uint64_t*)(_pBuf + _iRdPos));
    _iRdPos += sizeof value;
    return *this;
}

int NetBytes::GetBuf (char*  value, uint32_t& aiLen)
{
    aiLen = ntohl(*(uint32_t*)(_pBuf + _iRdPos));
    if(aiLen > 0)
    {
        memcpy(value, _pBuf + _iRdPos + sizeof(uint32_t), aiLen);
        _iRdPos += sizeof(uint32_t) + aiLen;
        return 0;
    }
    return -1;
}

NetBytes& NetBytes::operator >> (std::string& value)
{
    char szBuf[2048] = {0};
    uint32_t liLen = 0;
    if(0 == GetBuf(szBuf, liLen))
        value.assign(szBuf);
    return *this;
}

//-------------------------

NetBytes& NetBytes::operator << (int8_t value)
{
    *(int8_t*)(_pBuf + _iWtPos) = value;
    _iWtPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator << (int16_t  value)
{
    *(int16_t*)(_pBuf + _iWtPos) = htons(value);
    _iWtPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator << (int32_t  value)
{
    *(int32_t*)(_pBuf + _iWtPos) =  htonl(value);
    _iWtPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator << (int64_t  value)
{
    *(int64_t*)(_pBuf + _iWtPos) = hton64(value);
    _iWtPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator << (bool&  value)
{
    *(bool*)(_pBuf + _iWtPos) = value;
    _iWtPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator >> (int8_t& value)
{
    value = *(int8_t*)(_pBuf + _iRdPos);
    _iRdPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator >> (int16_t&  value)
{
    value =  ntohs(*(int16_t*)(_pBuf + _iRdPos));
    _iRdPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator >> (int32_t&  value)
{
    value =  ntohl(*(int32_t*)(_pBuf + _iRdPos));
    _iRdPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator >> (int64_t&  value)
{
    value =  ntoh64(*(int64_t*)(_pBuf + _iRdPos));
    _iRdPos += sizeof value;
    return *this;
}

NetBytes& NetBytes::operator >> (bool&  value)
{
    value = *(bool*)(_pBuf + _iRdPos);
    _iRdPos += sizeof value;
    return *this;
}
/*
NetBytes& NetBytes::operator >> (int&  value)
{
    value =  ntohl(*(int*)(_pBuf + _iRdPos));
    _iRdPos += sizeof value;
    return *this;
}
*/
