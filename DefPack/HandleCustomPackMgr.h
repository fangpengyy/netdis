
#ifndef __HANDLECUSTOMPACKMGR_H__
#define __HANDLECUSTOMPACKMGR_H__
#include "../Pack.h"




class HandleCustomPackMgr
{
public:
   HandleCustomPackMgr()
   {
   }
   static HandleCustomPackMgr* GetInstance();

   void OnPack_TickHeartHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_GetTitleHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_ReqName_2Handle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_GetValHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_SetTitleHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_MsgHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_MsgExHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_ReqNameHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_SetValHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_TestHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_ReqAddValueHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_RespAddValueHandle(TcpConnPtr& apConn, IPacket* apPack);
   void OnPack_TestResponseHandle(TcpConnPtr& apConn, IPacket* apPack);
private:
   static HandleCustomPackMgr* _pHandlePackMgr;
};

#endif
