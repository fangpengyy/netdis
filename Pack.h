#ifndef __PROTO_PACK_H__
#define __PROTO_PACK_H__

#include <unistd.h>
#include <stdint.h>
#include "Utils.h"



#define TCP_HEAD_FLAG 0x2323
#define DEF_PACK_MAX  2048


enum EnumZip{z_none=0, z_1};
enum EnumEncrypt{en_none=0, en_1};

#pragma pack(1)
//33B
struct PackHead
{
        PackHead() :flag(TCP_HEAD_FLAG), packTypeId(0), sessionId(0),
        userId(0), packLen(0), dataFlag(0),crc32(0)
        {
        }
        uint16_t flag; //##
        uint32_t packTypeId; //服务类型id,包类型id
        uint64_t sessionId; //一次tcp连接有效
        uint64_t userId; //业务层，用户登录的id
        uint16_t packLen; //（包头+包体）=包大小
       // uint32_t packTypeId;   //功能编号
        uint8_t dataFlag;  // 按位操作数据，标记位，0位压缩，1位加密，2-4位版本
        uint32_t crc32; //0无校验,

        int ParseRouteData(char* szBuf, uint32_t aiLen);
        void SetSessionId(char* apBuf, uint64_t aiSessionId);

        bool IsValid();
        void SetHead(uint32_t aiPackTypeId,  uint8_t aiDataFlag);
        void SetCrc32(uint32_t aiCrc32){crc32 = aiCrc32;}
        uint32_t GetCrc32(){return crc32;}
        int Pack(char* szBuf, uint32_t& aiLen);
        void UnPack(char* szBuf, uint32_t aiLen);
};

#pragma pack()


class IPacket
{
public:
    uint32_t packTypeId;
   IPacket(){}
   virtual ~IPacket(){}
public:
	virtual uint32_t PackTypeId(){return packTypeId;}
	virtual void Pack(char* szBuf, uint32_t& aiLen) {}
	virtual void UnPack(char* szBuf, uint32_t aiLen){}
    virtual void OnHandle(TcpConnPtr& apConn, uint16_t aipackTypeId, char* szBuf, uint32_t aiLen){}

};

typedef   std::function< void (TcpConnPtr& apConn, IPacket* apPack)> TOnCustomPackHandle;

#endif
