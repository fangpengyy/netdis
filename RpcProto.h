#ifndef __RPCPROTO_H__
#define __RPCPROTO_H__
#include <string>
#include <stdint>


struct StruReqName
{
  const static uint32_t packTypeId = 112;
  int id;

};

struct StruRespName
{
  std::string sName;
};





#endif


