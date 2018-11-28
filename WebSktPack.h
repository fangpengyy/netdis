#ifndef   __WEBSKTCODEC_H__
#define  __WEBSKTCODEC_H__
#include <string>


struct Stru_WebSktPack
{
    uint8_t _iFin;
    uint8_t _iOpcode;
    uint64_t _iPayloadLen;
    void* _pPayload;
};

class WebSktPack
{
public:
    void GetHandShakeResp (const char* szReqKey,  uint32_t aiCode,  std::string& asResp);
    int DecodePack(char* apBuf, uint32_t aiBufLen);
    int EncodePack(uint8_t aiOpcode, char* apData/*数据*/, uint32_t aiDataLen,
                   char* apOutBuf/*预先申请缓存*/,  uint32_t aiSize/*缓存大小*/);

    Stru_WebSktPack& GetPack()
    {
        return _oWebPack;
    }
private:
    void GetRespKey(const char* szReqKey, std::string& asRespKey);
    void GetDate(std::string& asDate);
    static bool IsRightOpcode(uint8_t aiOpcode);

    Stru_WebSktPack _oWebPack;
};




#endif
