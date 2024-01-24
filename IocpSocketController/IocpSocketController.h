#pragma once
#include <WinSock2.h>
#include "PacketDefine.h"

class IocpSocketController
{
public:
	IocpSocketController();
	~IocpSocketController();

	inline const bool			isConnected()	const { return _isConnected; }
	inline const UINT32			getIndex()		const { return _index; }
	inline SOCKET				getSocket() { return _mySock; }
	inline OverlappedIOBuffer& getRecvBuffer() { return _overlappedRecvBuffer; }
	inline OverlappedIOBuffer& getSendBuffer() { return _overlappedSendBuffer; }

	bool initialize(const UINT32 index, HANDLE iocpHandle);
	bool connectIOCP();

	bool acceptAsync(SOCKET listenSock);
	bool acceptCompletion();

	bool bindRecv();
	bool sendMsg(const std::string& msgStirng);
	void close(bool isForce = false);

private:
	void resetBufferAndSetOverlappedIOInfo(OverlappedIOBuffer& overlappedIOBuffer, OperationType type);

private:
	OverlappedIOBuffer	_overlappedSendBuffer;
	OverlappedIOBuffer	_overlappedRecvBuffer;
	OverlappedIOBuffer	_overlappedAcceptBuffer;

	HANDLE				_IOCPHandle = INVALID_HANDLE_VALUE;
	SOCKET				_mySock = INVALID_SOCKET;

	INT32				_index = -1;
	bool				_isConnected = false;
};