#ifndef __NETBUFFER_H__
#define __NETBUFFER_H__
#include <stdint.h>
#include "DataStream.h"
#include "Pack.h"


class NetBuffer
{
public:
	static uint32_t BUF_LEN;
	NetBuffer();
	~NetBuffer();

	int ReadFd(int sockFd);
	int ParsePacket(DataStream& dataStream, PackHead& aoPackHead);

	uint32_t Size(){ return _iSize; }
	uint32_t WriteableSize(){ return _iSize - _iWritePos; }
	uint32_t ReadableSize(){ return _iWritePos - _iReadPos; }
	uint32_t ReadPos(){ return _iReadPos; }
	uint32_t WritePos(){ return _iWritePos; }

	char* BeginRead(){ return _pBuf + _iReadPos; }
	char* BeginWrite(){ return _pBuf + _iWritePos; }

	void AddReadPos(uint32_t aiPos){ _iReadPos += aiPos; }
	void AddWritePos(uint32_t aiPos){ _iWritePos += aiPos; }

	int Append(const char* apBuf, uint32_t aiDataLen);

private:
	uint32_t _iReadPos;
	uint32_t _iWritePos;
	uint32_t _iSize;
	char* _pBuf;

};





#endif
