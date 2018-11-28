#ifndef __HTTP_REQ_H__
#define __HTTP_REQ_H__
#include <functional>
#include <map>
#include <string>
#include <atomic>
#include "NetBuffer.h"


#define DEF_CRLF    "\r\n"


enum Enum_Method{mtInvalid, mtGet, mtPost};


class HttpResp
{
public:
    HttpResp();
    ~HttpResp();

    void SetRespCode(uint32_t  aiCode);
    void SetBody(const char* szBody, uint32_t aiSize);
    const char* GetRawData(uint32_t& aiDataLen);
private:
    void ResetData(uint32_t aiSize);

private:
    char* _pData;
    uint32_t _iDataMaxLen;
    uint32_t _iDataLen;
    uint32_t _iBodyLen;
    uint32_t _iCode;
};



enum Enum_ParseState{enum_psComplete,  enum_psNextLine, enum_psBadLine,  enum_psNotFullBody, };

class HttpReq
{
public:
    enum Enum_State{ stNone, stReqLine, stHeader, stBody, stComplete};
     HttpReq();
     ~HttpReq();
     HttpResp& Resp(){return _httpResp;}

     Enum_ParseState ParseReq(NetBuffer* pNetBuffer);
     bool IsKeepAlive();
     bool IsComplete();
     std::string& GetAction(){return _sFunction;}
     void Reset();

     bool IsUpgrade(){return _bWebSocket;}
     void GetReqKey(std::string& asReqKey){asReqKey = _sReqKey;}
     void SetHandShake(bool abShake){_bHandShake = abShake;}
     bool GetHandShake(){return  _bHandShake;}
     bool GetWebSktProto(std::string& asWebSktProto);
private:
    int GetLine(char* szText,  int aiLen);

private:
     Enum_Method _iMethod;
     std::string _sFunction;
     std::string _sProtoc;

    std::string _sReqBody;
    uint32_t _iBodyLength;
    std::string  _sVersion;
    std::string _sReqKey;
    std::string _sUpgrade;
    std::string _sHost;
    std::string _sConnection;
    bool _bZip;
    std::string _sWebSktProto;

    Enum_State _iState;
    HttpResp _httpResp;
    bool _bHandShake;
private:
     bool _bHostFlag;
     bool _bReqKeyFlag;
     bool _bUpgradeFlag;
     bool _bVersionFlag;
     bool  _bZipFlag;
     bool _bLengthFlag;
     bool _bConnFlag;
     bool _bWebSktProtoFlag;

     bool _bWebSocket;
};











#endif
