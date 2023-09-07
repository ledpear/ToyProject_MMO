#pragma once
#include <WinSock2.h>
#include <mutex>

#include "ServerDefine.h"

struct threadSafeBuffer
{
	char _buffer[CHAT_BUF_SIZE];
	std::mutex	_mutex;
	OverlappedIOInfo _overlappedIOInfo;
};

class ClientIOController
{
public:
	ClientIOController(const UINT32 index);
	~ClientIOController();

	inline const bool			isConnected()	const	{ return _isConnected; }
	inline const UINT32			getIndex()		const	{ return _index; }
	inline SOCKET				getSocket()				{ return _clientSock; }
	inline threadSafeBuffer&	getRecvBuffer()			{ return _recvBuffer; }
	inline threadSafeBuffer&	getSendBuffer()			{ return _sendBuffer; }

	void initialize(const UINT32 index, HANDLE iocpHandle);
	bool connectIOCP();

	bool onAccept(SOCKET listenSock);
	bool AcceptCompletion();

	bool bindRecv();
	bool sendMsg(const UINT32 dataSize, const std::string& msgStirng);
	bool sendCompletion(OverlappedIOInfo* sendOverlappedIOInfo);
	void close(bool isForce = false);

private:
	threadSafeBuffer	_sendBuffer;
	threadSafeBuffer	_recvBuffer;

	char				_acceptBuffer[64];
	OverlappedIOInfo	_acceptIOInfo;

	HANDLE				_IOCPHandle = INVALID_HANDLE_VALUE;
	SOCKET				_clientSock = INVALID_SOCKET;

	INT32				_index = -1;
	bool				_isConnected = false;
};