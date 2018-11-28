#include "HttpReq.h"


#include "Utils.h"
#include <algorithm>


const char* szResponse="HTTP/1.1 %d %s\r\n"\
                       "Content-Type:text/html\r\n" \
                       "Content-Length:%d\r\n"\
                       "Connection:keep-alive\r\n"\
                       "\r\n";




//HttpResponse-------------
HttpResp::HttpResp()
{
    _iDataMaxLen = 2048;
    _pData = new char[_iDataMaxLen];
    _iDataLen  = 0;
}

HttpResp::~HttpResp()
{
    if(_pData)
    {
        delete[] _pData;
        _pData = NULL;
    }
}

void HttpResp::ResetData(uint32_t aiSize)
{
    if(_pData)
    {
        delete[] _pData;
        _pData = NULL;
    }
    _pData = new char[aiSize];
    _iDataMaxLen = aiSize;
    _iDataLen = 0;
}

void HttpResp::SetRespCode(uint32_t  aiCode)
{
    _iCode = aiCode;
}

void HttpResp::SetBody(const char* szBody, uint32_t aiLen)
{
    _iBodyLen = aiLen;
    int liLen = sprintf(_pData,  szResponse,  _iCode,   GetRespStatusDesc(_iCode),  _iBodyLen);
    if(aiLen == 0)
    {
        _iDataLen = liLen;
        return;
    }
    uint32_t liSize = liLen + _iBodyLen;
    if(liSize > _iDataMaxLen)
        ResetData(liSize);

    _iDataLen = liSize;
    memcpy(_pData + liLen,  szBody,  _iBodyLen);
}

const char* HttpResp::GetRawData(uint32_t& aiDataLen)
{
    aiDataLen = _iDataLen;
    return _pData;
}


//HttpReq----------------------
HttpReq::HttpReq()
{
    Reset();
    _iState = stReqLine;
    _iBodyLength = 0;
    _bZip = false;
    _bHandShake = false;
}

HttpReq::~HttpReq()
{

}

void HttpReq::Reset()
{
    _bHostFlag = false;
    _bReqKeyFlag = false;
    _bUpgradeFlag = false;
    _bVersionFlag = false;
    _bZipFlag = false;
    _bLengthFlag = false;
    _bConnFlag = false;
    _bWebSocket = false;
     _bWebSktProtoFlag = false;
}

int HttpReq::GetLine(char* szText,  int aiLen)
{
    for(int i = 0; i < aiLen; i++)
    {
        if(szText[i] == '\r')
        {
            if(szText[i + 1] == '\n')
            {
                szText[i] = '\0';
                szText[i + 1] = '\0';
                return i + 2;
            }
        }
        else if(szText[i] == '\n')
        {
            szText[i] = '\0';
            return i + 1;
        }
    }
    return 0;
}

bool HttpReq::IsComplete()
{
    return  _iState == stComplete;
}

bool HttpReq::IsKeepAlive()
{
    if(_sConnection.empty())
        return false;
    return _sConnection.compare("Keep-Alive") == 0 ||  _sConnection.compare("Upgrade") == 0;
}

 bool HttpReq::GetWebSktProto(std::string& asWebSktProto)
{
    if(!_bWebSktProtoFlag)
        return false;

     asWebSktProto = "Sec-WebSocket-Protocol:";
     asWebSktProto +=" "+_sWebSktProto + "\r\n";
     return true;
}

Enum_ParseState HttpReq::ParseReq(NetBuffer* pNetBuffer)
{
    while(_iState != stComplete)
    {
        char* lpBuf = pNetBuffer->BeginRead();
        if(_iState == stReqLine)
        {
            int liLineSize = GetLine(lpBuf, strlen(lpBuf));
            if (liLineSize == 0)
                return enum_psBadLine;

            char* lpCmd = lpBuf;
            if (0 == strncmp(lpCmd, "GET", 3))
            {
                lpCmd += 3;
                _iMethod = mtGet;
            }
            else
                return enum_psBadLine;
            lpCmd++;

            char* lpFunName = strstr(lpCmd, "?");
            if (lpFunName == NULL)
                return  enum_psBadLine;
            lpFunName++;

            char* lpProtoc = strstr(lpFunName, " ");
            if (lpProtoc)
                _sFunction.assign(lpFunName, lpProtoc);
            else
                return  enum_psBadLine;

            lpProtoc++;
            _sProtoc.assign(lpProtoc);
            pNetBuffer->AddReadPos(liLineSize);
            _iState = stHeader;
        }
        else  if(_iState == stHeader)
        {
            int liLineSize = GetLine(lpBuf, strlen(lpBuf));
            if (liLineSize == 0)
                return  enum_psBadLine;

            if(liLineSize == 2 ||   liLineSize == 1)
            {
                pNetBuffer->AddReadPos(liLineSize);
                if(_iBodyLength > 0)
                {
                    _iState = stBody;
                    return enum_psNextLine;
                }
                else
                {
                    _iState = stComplete;
                    return enum_psComplete;
                }
            }
            if(!_bZipFlag && strncmp(lpBuf, "Accept-Encoding:",  16) == 0)
            {
                _bZipFlag = true;
                char* lp = lpBuf + 16;
                //int liNum = strspn(lp,  " ");
               // if(strncmp(lp + liNum, "gzip", 4) == 0)
               if(strstr(lp + 1, "gzip"))
                    _bZip = true;
            }
            else if(!_bHostFlag && strncmp(lpBuf, "Host:", 5) == 0)
            {
                _bHostFlag = true;
                char* lp = lpBuf + 5;
                //int liNum = strspn(lp,  " ");
                _sHost = lp + 1; //liNum;
            }
            else if(!_bConnFlag && strncmp(lpBuf, "Connection:", 11) == 0)
            {
                _bConnFlag = true;
                char* lp = lpBuf + 11;
               // int liNum = strspn(lp,  " ");
                _sConnection = lp + 1; //liNum;
            }
            else if(!_bUpgradeFlag && strncmp(lpBuf, "Upgrade:", 8) == 0)
            {
                _bWebSocket = true;
                _bUpgradeFlag = true;
                char* lp = lpBuf + 8;
              //  int liNum = strspn(lp,  " ");
                _sUpgrade = lp + 1; //liNum;
            }
            else if(!_bReqKeyFlag && strncmp(lpBuf, "Sec-WebSocket-Key:", 18) == 0)
            {
                _bReqKeyFlag = true;
                char* lp = lpBuf + 18;
                //int liNum = strspn(lp,  " ");
                _sReqKey = lp + 1; //liNum;
            }
            else if(!_bVersionFlag && strncmp(lpBuf, "Sec-WebSocket-Version:", 22) == 0)
            {
                _bVersionFlag = true;
                char* lp = lpBuf + 22;
              //  int liNum = strspn(lp,  " ");
                _sVersion = lp + 1; // liNum;
            }
            else if(!_bLengthFlag && strncmp(lpBuf, "Content-Length:", 15) == 0)
            {
                _bLengthFlag = true;
                char* lp = lpBuf + 15;
               // int liNum = strspn(lp,  " ");
                _iBodyLength = atol(lp + 1); //liNum);
            }
            else if(!_bWebSktProtoFlag && strncmp(lpBuf, "Sec-WebSocket-Protocol:", 23) == 0)
           {
               _bWebSktProtoFlag = true;
               char* lp = lpBuf + 23;
               // int liNum = strspn(lp,  " ");
                _sWebSktProto = lp + 1;
           }
            pNetBuffer->AddReadPos(liLineSize);
        }
        else  if(_iState == stBody)
        {
            if(pNetBuffer->ReadableSize() < _iBodyLength)
                return enum_psNotFullBody;

            _sReqBody.assign(lpBuf, lpBuf + _iBodyLength);
            pNetBuffer->AddReadPos(_iBodyLength);
            _iState = stComplete;
        }
    }
    return   _iState == stComplete ? enum_psComplete : enum_psNextLine;
}








