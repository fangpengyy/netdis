#include "NetBuffer.h"
#include "Utils.h"
#include "Log.h"

#include <string.h>
#include <stdlib.h>
#include <sys/uio.h>




uint32_t NetBuffer::BUF_LEN = 1024;


NetBuffer::NetBuffer() :_iReadPos(0), _iWritePos(0),_iSize(BUF_LEN)
{
	_pBuf = new char[_iSize];
}

NetBuffer::~NetBuffer()
{
	if (_pBuf)
	{
		delete[] _pBuf;
		_pBuf = NULL;
	}
}

int NetBuffer::ReadFd(int sockFd)
{
	char pExtraBuf[65536];
	struct iovec loVec[2];

	const uint32_t liWriteSize = WriteableSize();
	loVec[0].iov_base = BeginWrite();
	loVec[0].iov_len = liWriteSize;
	loVec[1].iov_base = pExtraBuf;
	loVec[1].iov_len = sizeof pExtraBuf;

	const int liVecCnt = (liWriteSize < sizeof pExtraBuf) ? 2 : 1;
	const ssize_t liRecvSize = ::readv(sockFd, loVec, liVecCnt);

	if (liRecvSize < 0)
	{
		LOG_ERR("readv err=%d", errno);
		return liRecvSize;
	}

	if (liRecvSize <= liWriteSize)
	{
		_iWritePos += liRecvSize;
	}
	else
	{
		_iWritePos =_iSize;
		Append(pExtraBuf, liRecvSize - liWriteSize);
	}
	return liRecvSize;
}

int NetBuffer::ParsePacket(DataStream& dataStream,  PackHead& aoPackHead)
{
    uint32_t liReadable = ReadableSize();
    if(liReadable < sizeof(PackHead) )
		return -1;

    char* lpBuf = BeginRead();

    aoPackHead.UnPack(lpBuf,  sizeof(PackHead) );
    if(!aoPackHead.IsValid())
	{
        return -3;
	}

    uint16_t  liPackLen = aoPackHead.packLen;
    if(liPackLen > DEF_PACK_MAX)
		return -4;

    if (liPackLen > liReadable)
         return -2;

    dataStream.CopyData(lpBuf + sizeof( PackHead),  liPackLen - sizeof( PackHead));
    AddReadPos(liPackLen);
    return 0;
}


int NetBuffer::Append(const char* apBuf, uint32_t aiDataLen)
{
	if (aiDataLen == 0 || apBuf == NULL)
		return -1;

	uint32_t liWSize = WriteableSize();

	if (liWSize < aiDataLen)
	{
		_iWritePos = ReadableSize();

		if (liWSize + _iReadPos < aiDataLen)
		{
			_iSize += aiDataLen + BUF_LEN;
			char* lpBuf = new char[_iSize];

			if (_iWritePos > 0)
				memcpy(lpBuf, _pBuf + _iReadPos, _iWritePos);
			_iReadPos = 0;

			delete[] _pBuf;
			_pBuf = lpBuf;
		}
		else
		{
			if (_iWritePos > 0)
				memmove(_pBuf, _pBuf + _iReadPos, _iWritePos);
			_iReadPos = 0;
		}
	}

	memcpy(_pBuf + _iWritePos, apBuf, aiDataLen);
	_iWritePos += aiDataLen;

	return 0;
}
