#include "WebSktPack.h"
#include "./Utils/Base64.h"
#include "Utils.h"
#include "NetBytes.h"
#include <openssl/sha.h>

// "Sec-WebSocket-Protocol: Netdis\r\n"\

const char* szWsResponse ="HTTP/1.1 %d %s\r\n"\
                          "Upgrade: websocket\r\n" \
                          "Connection: Upgrade\r\n" \
                          "Sec-WebSocket-Accept: %s\r\n" \
                          "Content-Length: %d\r\n"\
                          "Date: %s\r\n"
                          ;


void WebSktPack::GetHandShakeResp (const char* szReqKey,  uint32_t aiCode,  std::string& asResp)
{
    std::string lsRespKey;
    GetRespKey(szReqKey, lsRespKey);

    char szBuf[1024] = {0};
    std::string lsDate;
    GetDate(lsDate);
    sprintf(szBuf, szWsResponse,  aiCode, GetRespStatusDesc(aiCode),  lsRespKey.c_str(), 0, lsDate.c_str());
    asResp = szBuf;
}

int WebSktPack::DecodePack(char* apBuf, uint32_t aiBufLen)
{
    if(aiBufLen < 2)
        return 0;

    uint8_t* lp = (uint8_t*)apBuf;
    _oWebPack._iFin = lp[0] & 0x80;
    _oWebPack._iOpcode = lp[0] & 0x0f;
    if(!IsRightOpcode(_oWebPack._iOpcode))
        return -1;
    uint8_t liMask = lp[1] & 0x80  ;
    if(liMask == 0)
        return -2;

    uint32_t liPackLen = 0;
    uint8_t liLen = lp[1] & 0x7f;

    if(liLen < 126)
    {
        liPackLen = 2;
        _oWebPack._iPayloadLen = liLen;
    }
    else if(liLen == 126)
    {
        liPackLen = 2 + 2;
        if(aiBufLen < liPackLen)
            return 0;
        _oWebPack._iPayloadLen = ntohs(*(uint16_t*)(lp + 2));
    }
    else if(liLen == 127)
    {
        liPackLen = 2 + 8;
        if(aiBufLen < liPackLen)
            return 0;
        _oWebPack._iPayloadLen = ntohs(*(uint64_t*)(lp + 2));
    }

    if(aiBufLen < liPackLen + 4)
        return 0;
    uint8_t liArrMask[4];
    memcpy(liArrMask, lp + liPackLen, sizeof(liArrMask));
    liPackLen += sizeof(liArrMask);
    _oWebPack._pPayload = lp + liPackLen;
    liPackLen += _oWebPack._iPayloadLen;

    if(aiBufLen < liPackLen)
        return 0;

    lp = (uint8_t*)_oWebPack._pPayload;
    for(size_t i = 0; i < _oWebPack._iPayloadLen; i++)
        lp[i] = lp[i] ^ liArrMask[i & 3];

    return liPackLen;
}

int WebSktPack::EncodePack(uint8_t aiOpcode, char* apData, uint32_t aiDataLen,
                           char* apOutBuf,  uint32_t aiSize)
{
    if(aiSize == 0 || apOutBuf == nullptr)
        return -1;
    if(aiDataLen + 10 > aiSize)
        return -2;

    uint32_t liPackLen = 0;
    uint8_t* lp = (uint8_t*)apOutBuf;
    lp[0] = 0;
    lp[0] |= 0x1 << 7;
    lp[0] |= aiOpcode;

    lp[1] = 0;
    if(aiDataLen < 126)
    {
        uint8_t liLen = aiDataLen;
        lp[1] |= liLen;
        liPackLen = 2;
    }
    else if(aiDataLen < 0xffff)
    {
        lp[1] |=126;
        uint16_t liLen = htons(aiDataLen);
        memcpy(lp + 2, &liLen, sizeof(liLen));
        liPackLen = 2 + sizeof(liLen);
    }
    else
    {
        lp[1] |=127;
        uint64_t liLen = hton64(aiDataLen);
        memcpy(lp + 2, &liLen, sizeof(liLen));
        liPackLen = 2 + sizeof(liLen);
    }
    if(apData && aiDataLen > 0)
    {
        memcpy(lp + liPackLen, apData, aiDataLen);
        liPackLen += aiDataLen;
    }
    return  liPackLen;
}

bool WebSktPack::IsRightOpcode(uint8_t aiOpcode)
{
    static uint8_t loCodeList[] = { 0x0, 0x1, 0x2, 0x8, 0x9, 0xa };
    for (size_t i = 0; i < sizeof(loCodeList); ++i)
    {
        if (aiOpcode == loCodeList[i])
            return true;
    }
    return false;
}

void WebSktPack::GetRespKey(const char* szReqKey, std::string& asRespKey)
{
    std::string lsKey = szReqKey;
    lsKey += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    size_t liSize = lsKey.size();
    unsigned char szHash[20] = {0};
    SHA1((const unsigned char*)lsKey.c_str(), liSize, szHash);

    char szB64text[512] = {0};
    int liLen = 0;
    base64_encode(szHash,  sizeof(szHash), szB64text, liLen);
    asRespKey = szB64text;
}

void WebSktPack::GetDate(std::string& asDate)
{
    time_t liNow = time(NULL);
    struct tm * lpTm = gmtime(&liNow);
    char szDate[100];
    strftime(szDate,  sizeof(szDate), "%a, %d %B %Y %T GMT",  lpTm);
    asDate = szDate;
}

