#ifndef __NETBYTES_H__
#define __NETBYTES_H__
#include <stdint.h>
#include <unistd.h>
#include <string>




extern uint64_t hton64(uint64_t value);
extern uint64_t ntoh64(uint64_t value);



class NetBytes
{
public:
	NetBytes(char* szBuf,  uint32_t aiBufSize);

    char* GetBuf(){return _pBuf;}
    uint32_t GetDataLen(){return _iWtPos;}

   // NetBytes& operator << (unsigned char value);
    NetBytes& operator << (uint8_t  value);
    NetBytes& operator << (uint16_t  value);
    NetBytes& operator << (uint32_t  value);
    NetBytes& operator << (uint64_t  value);
    int Append (const char*  value, uint32_t aiLen);
    NetBytes& operator << (std::string value);

    //NetBytes& operator >> (unsigned char& value);
    NetBytes& operator >> (uint8_t&  value);
    NetBytes& operator >> (uint16_t&  value);
    NetBytes& operator >> (uint32_t&  value);
    NetBytes& operator >> (uint64_t&  value);
    int GetBuf (char* value, uint32_t& aiLen);
    NetBytes& operator >> (std::string& value);

    //NetBytes& operator << (char value);
    NetBytes& operator << (int8_t  value);
    NetBytes& operator << (int16_t  value);
    NetBytes& operator << (int32_t  value);
    NetBytes& operator << (int64_t  value);
    NetBytes& operator << (bool&  value);



  //  NetBytes& operator >> (char& value);
    NetBytes& operator >> (int8_t&  value);
    NetBytes& operator >> (int16_t&  value);
    NetBytes& operator >> (int32_t&  value);
    NetBytes& operator >> (int64_t&  value);
    NetBytes& operator >> (bool&  value);


private:
	char* _pBuf;
	uint32_t _iBufSize;

    uint32_t _iWtPos;
    uint32_t _iRdPos;

};
#endif
