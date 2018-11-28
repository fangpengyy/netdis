#include "RoutePack.h"
#include "Pack_Include.h"
#include "DefPackNo.h"


#define DEF_PACK(packName)\
case DEF_FUNC_##packName:\
    {\
        IPacket* pPack = new Pack_##packName();\
        return pPack;\
    }



IPacket* GetPackObj(uint32_t packTypeId)
{
    switch(packTypeId)
   {

        DEF_PACK(Msg)


   DEF_PACK(GetTitle)
   DEF_PACK(SetTitle)
   DEF_PACK(ReqName)
   DEF_PACK(ReqName_2)
   DEF_PACK(GetVal)
   DEF_PACK(SetVal)
   DEF_PACK(MsgEx)
   DEF_PACK(Test)
   DEF_PACK(TickHeart)
   DEF_PACK(ReqAddValue)
   DEF_PACK(RespAddValue)
   DEF_PACK(TestResponse)
}
    return NULL;
}
