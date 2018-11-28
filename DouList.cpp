#include "DouList.h"
#include <iostream>



DouList::DouList()
{
    _pTail = nullptr;
    _pHead = nullptr;
    _filterFunc = nullptr;
}

DouList::~DouList()
{

}

void DouList::SetFilterFunc(TFilterFunc filterFunc)
{
    _filterFunc = filterFunc;
}

void DouList::AddNode(TcpConn* apNode)
{
    if (apNode == nullptr)
        return;

    apNode->next = nullptr;
    if (_pTail == nullptr)
    {
        apNode->prev = nullptr;
        _pHead = apNode;
        _pTail = _pHead;
    }
    else
    {
        _pTail->next = apNode;
        apNode->prev = _pTail;
        _pTail = apNode;
    }
}

TcpConn* DouList::DelNode(TcpConn* apNode)
{
    if (apNode == nullptr)
        return nullptr;

    if (_pHead == apNode)
    {
        apNode->prev = nullptr;
        if(apNode->next == nullptr)
        {
            _pHead = nullptr;
            _pTail = nullptr;
            return nullptr;
        }
        else
        {
            _pHead = apNode->next;
            apNode->next = nullptr;
            return _pHead;
        }
    }

    if (_pTail == apNode)
    {
        _pTail = apNode->prev;
        if (_pTail)
            _pTail->next = nullptr;

        apNode->prev = nullptr;
        apNode->next = nullptr;
        return nullptr;
    }

    TcpConn* lpPrevNode = apNode->prev;
    TcpConn* lpNextNode = apNode->next;

    if(lpNextNode)
        lpNextNode->prev = lpPrevNode;
    if (lpPrevNode)
        lpPrevNode->next = lpNextNode;
    apNode->prev = nullptr;
    apNode->next = nullptr;

    return lpNextNode;
}

int DouList::CheckNodeList(uint64_t now)
{
    TcpConn* lpNextNode = _pHead;
    int liCount = 0;
    while (lpNextNode)
    {
        if (_filterFunc(lpNextNode, now))
        {
            TcpConn* lpNode = lpNextNode;
            lpNextNode = DelNode(lpNode);
            lpNode->HandleClose();
            liCount++;
        }
        else
            break;
    }
    return liCount;
}

void DouList::ViewNodeInfo()
{
    TcpConn* lpNode = _pHead;
    while (lpNode)
    {
#ifdef DEF_COUT
        std::cout << __FUNCTION__ << "  " << lpNode << "\n";
#endif
        lpNode = lpNode->next;
    }
}

void DouList::UpdateNode(TcpConn* apNode)
{
    DelNode(apNode);
    AddNode(apNode);
}

//TurnTimer-------------------------
TurnTimer::TurnTimer()
{
    _iDeltaMs = 0;
    _iSize = 0;
    _iCurIndex = 1;
}

TurnTimer::~TurnTimer()
{
      Clear();
}

void TurnTimer::Init(uint16_t  aiSize)
{
    _vecSetTcpConn.resize(aiSize);
    _iSize = aiSize;
}

void TurnTimer::CheckTimeout(uint64_t aiDeltaMs)
{
    _iDeltaMs += aiDeltaMs;
    if(_iDeltaMs > 1000)
    {
          _iDeltaMs = 0;
          _iCurIndex++;
          if(_iCurIndex >= _iSize)
            _iCurIndex = 0;

          TSetTcpConn::iterator it = _vecSetTcpConn[_iCurIndex].begin();
          while(it != _vecSetTcpConn[_iCurIndex].end())
          {
                (*it)->ForceClose();
                 it++;
          }
         _vecSetTcpConn[_iCurIndex].clear();
    }
}

void TurnTimer::Update(TcpConn* apTcpConn)
{
    int  liIndex = apTcpConn->GetTurnIndex();
    int  liCurIndex = _iCurIndex -1;
    if(liCurIndex < 0)
        liCurIndex = _iSize - 1;

    if(liIndex < _iSize)
    {
        TSetTcpConn& loSetTcpConn = _vecSetTcpConn[liIndex];
        TSetTcpConn::iterator it = loSetTcpConn.find(apTcpConn);
        if(it != loSetTcpConn.end())
            loSetTcpConn.erase(it);
    }
    _vecSetTcpConn[liCurIndex].insert(apTcpConn);
    apTcpConn->SetTurnIndex(liCurIndex);
}

void TurnTimer::Remove(TcpConn* apTcpConn)
{
    int  liIndex = apTcpConn->GetTurnIndex();
    if(liIndex  >= _iSize)
        return;

    TSetTcpConn& loSetTcpConn = _vecSetTcpConn[liIndex];
    TSetTcpConn::iterator it = loSetTcpConn.find(apTcpConn);
    if(it != loSetTcpConn.end())
        loSetTcpConn.erase(it);
}

void TurnTimer::Clear()
{
    for(int i = 0; i < _iSize; i++)
        _vecSetTcpConn[i].clear();
}

