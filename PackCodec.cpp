#include "PackCodec.h"
#include "HttpServer.h"
#include "Utils.h"

PackCodec::PackCodec(): _OnRecvPack(nullptr),_OnRouteRawBuf(nullptr)
{

}

PackCodec::~PackCodec()
{
}

char* PackCodec::OnSendPack(PackHead& aoPackHead, IPacket* pPack,  uint32_t& aiBufSize)
{
    uint32_t liHeadSize = sizeof (PackHead);

    uint32_t liLen =DEF_PACK_MAX + liHeadSize;
    char*pszBuf = new char[liLen];

    uint32_t liDataLen = DEF_PACK_MAX;
   // printf("--- %d\n", pPack->packTypeId);

    pPack->Pack(pszBuf + liHeadSize, liDataLen);
    if(DEF_PACK_MAX < liDataLen)
    {
        delete[] pszBuf;
        LOG_ERR("Pack Bufsize= %d > Max size=%d",  liDataLen, DEF_PACK_MAX);
        return nullptr;
    }

    aoPackHead.packTypeId = pPack->PackTypeId();
    aoPackHead.packLen = liDataLen;
    aoPackHead.Pack(pszBuf,  liHeadSize);

    liLen = liDataLen + liHeadSize;
    aiBufSize = liLen;
    return pszBuf;
}

int PackCodec::OnRawBuf(TcpConnPtr pConnPtr, NetBuffer* pNetBuffer, uint32_t& aiPackCount)
{
    PackHead loPackHead;
    DataStream loStream;
    aiPackCount = 0;
    int liSize = pNetBuffer->ReadableSize();
    int liHeadSize = sizeof(PackHead);
    printf("PackCodec::OnRawBuf buffsized =%d, h=%d \n",  liSize, liHeadSize);

    while (liSize > liHeadSize)
    {
        int liRet =  pNetBuffer->ParsePacket(loStream, loPackHead);
        if (0 == liRet)
        {
            if (_OnRecvPack)
            {
                _OnRecvPack(pConnPtr, &loPackHead, loStream.Buf(), loStream.DataLen());
                aiPackCount++;
            }
        }
        else
        {
            if(-3 == liRet)
                return -2;
            break;
        }
    }
    return 0;
}

//HttpCodec-------------------
int HttpCodec::OnRawBuf(TcpConnPtr pConnPtr, NetBuffer* pNetBuffer, uint32_t& aiPackCount)
{
    HttpReq* lpReq = static_cast<HttpReq*>(pConnPtr->GetContext());
    aiPackCount = 0;
    if(lpReq == nullptr)
    {
        pConnPtr->Shutdown();
        return 0;
    }
    int liRet = lpReq->ParseReq(pNetBuffer);
    if(liRet == enum_psComplete)
    {
        lpReq->Reset();
        std::string& lsAction = lpReq->GetAction();
        if(_OnHttpReq)
        {
            _OnHttpReq(lsAction,  pConnPtr,  lpReq);
            aiPackCount++;
        }
        if(!lpReq->IsKeepAlive())
                 pConnPtr->Shutdown();
    }
    else
    {
         LOG_INFO("ParseReq ret=%d", liRet);
    }
    return 0;
}

//WebSktCodec------------------------
WebSktCodec::WebSktCodec()
{
    _OnJsonHttpReq = nullptr;
     _iPongPackSize = _oWebSktPack.EncodePack(0xa, nullptr, 0,  _szPongPack, sizeof(_szPongPack));

}

WebSktCodec::~WebSktCodec()
{

}

void WebSktCodec::Response(TcpConnPtr& pConnPtr, uint32_t aiCode)
{
      const char* szFmt ="HTTP/1.1 %d %s\r\n"\
                           "Content-Type:text/html\r\n" \
                            "Connection:close\r\n"\
                            "\r\n";
      char szBuf[256] = {0};
      uint32_t liSize = sprintf(szBuf, szFmt, aiCode, GetRespStatusDesc(aiCode));
      pConnPtr->SendBuf(szBuf, liSize);
}

 int WebSktCodec::OnRawBuf(TcpConnPtr pConnPtr, NetBuffer* pNetBuffer, uint32_t&aiPackCount)
{
    HttpReq* lpReq = static_cast<HttpReq*>(pConnPtr->GetContext());
    if(lpReq == nullptr)
    {
        pConnPtr->Shutdown();
        return -1;
    }

    if(!lpReq->GetHandShake())
    {
        int liRet = lpReq->ParseReq(pNetBuffer);
        if(liRet == enum_psBadLine)
        {
             Response(pConnPtr, 400);
              pConnPtr->Shutdown();
              return -1;
        }
        else if(liRet == enum_psComplete)
        {
            if(lpReq->IsUpgrade())
            {
                std::string lsResp;
                std::string lsReqKey;
                lpReq->GetReqKey(lsReqKey);
                if(lsReqKey.empty())
                {
                    Response(pConnPtr, 1002);
                    pConnPtr->Shutdown();
                    return 0;
                }
                _oWebSktPack.GetHandShakeResp(lsReqKey.c_str(), 101, lsReqKey);
                if(!lsReqKey.empty())
                {
                    std::string lsWebSktProto;
                    if(lpReq->GetWebSktProto(lsWebSktProto))
                        lsReqKey += lsWebSktProto;
                    lsReqKey += "\r\n";
                    pConnPtr->SendBuf(lsReqKey.c_str(), lsReqKey.size());
                    lpReq->SetHandShake(true);
                }
            }
            else
            {
                Response(pConnPtr, 1002);
                pConnPtr->Shutdown();
                return 0;
            }
        }
        else
            LOG_INFO("ParseReq ret=%d", liRet);
    }
    else
    {
        char* lpBuf = pNetBuffer->BeginRead();
        uint32_t liLen = pNetBuffer->ReadableSize();
        int liPackSize = _oWebSktPack.DecodePack(lpBuf, liLen);
        if(liPackSize > 0)
        {
            pNetBuffer->AddReadPos(liPackSize);
            Stru_WebSktPack& loPack = _oWebSktPack.GetPack();
            if(loPack._iOpcode == 0x8)
            {
                pConnPtr->Shutdown();
                return 0;
            }
            else if(loPack._iOpcode == 0x9)
                  pConnPtr->SendBuf(_szPongPack, _iPongPackSize);
            else if(loPack._iOpcode == 0x1)
            {
                if(_OnJsonHttpReq)
                    _OnJsonHttpReq((char*)loPack._pPayload, loPack._iPayloadLen,  pConnPtr);
            }
            else if(loPack._iOpcode == 0x2)
            {
                 Response(pConnPtr, 1003);
            }
        }
    }
    return 0;
}

char* WebSktCodec::OnEncodeWebSktBuf(const char* apBuf, uint32_t aiDataLen,  uint32_t & aiOutSize)
{
    int liDataLen = aiDataLen + 20;
    char* lpBuf = new char[liDataLen];
    aiOutSize = _oWebSktPack.EncodePack(0x01, (char*)apBuf, aiDataLen,  lpBuf,  liDataLen);
    return lpBuf;
}
