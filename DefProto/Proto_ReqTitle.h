struct Stru_GetTitle
{
static const uint32_t PackTypeId = 106;
int32_t id;
std::string sTitle;

};


struct Stru_SetTitle
{
 static const uint32_t PackTypeId = 105;
int32_t id;
std::string sTitle;

};

struct Stru_Msg
{
    static const uint32_t PackTypeId = 104;
    std::string sMsg;
};

struct Stru_MsgEx
{
          static const uint32_t PackTypeId = 103;
    uint32_t msgId;
    std::string sMsgEx;
};

struct Stru_ReqAddValue
{
         static const uint32_t PackTypeId = 101;
    int32_t a1;
    int32_t a2;
};

struct Stru_RespAddValue
{
     static const uint32_t PackTypeId = 102;
    int32_t val;
};

struct Stru_TestResponse
{
    static const uint32_t PackTypeId = 200;
    std::string  sVal;
};
