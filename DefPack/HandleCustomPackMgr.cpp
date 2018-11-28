#include "HandleCustomPackMgr.h"
#include "Pack_Include.h"
#include "../TcpConn.h"
#include "../NetBytes.h"


HandleCustomPackMgr*  HandleCustomPackMgr::_pHandlePackMgr = 0;

HandleCustomPackMgr* HandleCustomPackMgr::GetInstance()
{
   if(_pHandlePackMgr == 0)
         _pHandlePackMgr = new HandleCustomPackMgr();
   return _pHandlePackMgr;
}
//-----------
void HandleCustomPackMgr::OnPack_GetTitleHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_GetTitle* lpPack = dynamic_cast<Pack_GetTitle*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
}
void HandleCustomPackMgr::OnPack_SetTitleHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_SetTitle* lpPack = dynamic_cast<Pack_SetTitle*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
}
void HandleCustomPackMgr::OnPack_MsgHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_Msg* lpPack = dynamic_cast<Pack_Msg*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
    printf("HandleCustomPackMgr::OnPack_MsgHandle   %s\n", lpPack->sMsg.c_str());
}
void HandleCustomPackMgr::OnPack_ReqName_2Handle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_ReqName_2* lpPack = dynamic_cast<Pack_ReqName_2*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
}
void HandleCustomPackMgr::OnPack_GetValHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_GetVal* lpPack = dynamic_cast<Pack_GetVal*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
}
void HandleCustomPackMgr::OnPack_SetValHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_SetVal* lpPack = dynamic_cast<Pack_SetVal*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
}
void HandleCustomPackMgr::OnPack_TickHeartHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_TickHeart* lpPack = dynamic_cast<Pack_TickHeart*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
}
void HandleCustomPackMgr::OnPack_ReqNameHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_ReqName* lpPack = dynamic_cast<Pack_ReqName*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
}
void HandleCustomPackMgr::OnPack_MsgExHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_MsgEx* lpPack = dynamic_cast<Pack_MsgEx*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
}
void HandleCustomPackMgr::OnPack_TestHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_Test* lpPack = dynamic_cast<Pack_Test*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
}
void HandleCustomPackMgr::OnPack_ReqAddValueHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_ReqAddValue* lpPack = dynamic_cast<Pack_ReqAddValue*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...

     printf("----------\n");
    int32_t  liValue = lpPack->a1 + lpPack->a2;

    printf("%s %d +%d=%d\n", __func__, lpPack->a1, lpPack->a2, liValue);

    Pack_RespAddValue loResp;
    loResp.val = liValue;

    PackHead loHead;
    loHead.SetHead(201, 0);
    apConn->SendPack(loHead, &loResp);

}
void HandleCustomPackMgr::OnPack_RespAddValueHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_RespAddValue* lpPack = dynamic_cast<Pack_RespAddValue*>(apPack);
   if(lpPack == NULL)
           return;
    //here add code ...
    printf(" resp  val=%d\n", lpPack->val);
}
void HandleCustomPackMgr::OnPack_TestResponseHandle(TcpConnPtr& apConn, IPacket* apPack)
{
   Pack_TestResponse* lpPack = dynamic_cast<Pack_TestResponse*>(apPack);
   if(lpPack == NULL)
           return;

    std::cout<<__PRETTY_FUNCTION__<<"  "<<  lpPack->sVal
    <<"  sessionid= " <<apConn->GetSessionId()<<"\n";

       char szBuf[2048];
       PackHead loPackHead;
       loPackHead.packTypeId = lpPack->PackTypeId();
       loPackHead.userId = apConn->GetUserId();
       loPackHead.sessionId = apConn->GetSessionId();
       loPackHead.dataFlag = apConn->GetDataFlag();

    //组包
        NetBytes loNetBytes(szBuf + sizeof(loPackHead),  sizeof(szBuf) - sizeof(loPackHead));
        loNetBytes<<lpPack->sVal;
        int liDataSize = loNetBytes.GetDataLen();
        loPackHead.packLen = sizeof(loPackHead) + liDataSize;

       //包头
        unsigned int liHeadLen =sizeof(loPackHead);
        loPackHead.Pack(szBuf, liHeadLen);

        apConn->SendBuf(szBuf, loPackHead.packLen);

    //here add code ...
}
