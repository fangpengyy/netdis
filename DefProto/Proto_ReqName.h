

struct Stru_TickHeart
{
 static const uint32_t PackTypeId = 120;
};

struct Stru_ReqName
{
 static const uint32_t PackTypeId = 107;
  int32_t id;
  uint32_t flag;

  std::string sName;
};

struct Stru_ReqName_2
{
  static const uint32_t PackTypeId = 108;
  int32_t id;
  uint32_t flag;
  uint16_t flag16;
  bool kk;
  uint8_t flag8;

  int16_t soce;
  int8_t up;

  std::string sName;
};


struct Stru_GetVal
{
static const uint32_t PackTypeId = 109;
int64_t id;
int32_t order;

};


struct Stru_SetVal
{
static const uint32_t PackTypeId = 110;
uint32_t id;
bool bOk;
};

struct Stru_Test
{
    static const uint32_t PackTypeId = 111;
    std::string sTalk;
    uint16_t seq;
};

