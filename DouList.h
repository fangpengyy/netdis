#ifndef __DOULIST_H__
#define __DOULIST_H__

#include <functional>
#include <stdint.h>
#include <map>
#include <set>
#include <vector>
#include "TcpConn.h"
#include "Utils.h"


typedef std::function<bool(TcpConn*, uint64_t)> TFilterFunc;


class DouList
{
public:
	DouList();
	~DouList();

	void SetFilterFunc(TFilterFunc filterFunc);
    void UpdateNode(TcpConn* apNode);
	int CheckNodeList(uint64_t now);
    void ViewNodeInfo();

    TcpConn* DelNode(TcpConn* apNode);
	void AddNode(TcpConn* apNode);

private:
	TcpConn* _pHead;
	TcpConn* _pTail;
	TFilterFunc _filterFunc;
};

typedef std::set<TcpConn*> TSetTcpConn;
class TurnTimer
{
public:
    TurnTimer();
    ~TurnTimer();

    void Init(uint16_t  aiSize);
    void CheckTimeout(uint64_t aiDeltaMs);
    void Update(TcpConn* apTcpConn);
    void Remove(TcpConn* apTcpConn);

private:
    void Clear();
    uint64_t _iDeltaMs;
    int _iSize;
    int _iCurIndex;
    std::vector<TSetTcpConn > _vecSetTcpConn;
};


#endif
