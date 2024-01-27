#pragma once
#include <WinSock2.h>
#include <functional>
#include "PacketDefine.h"
#include "IocpErrorCode.h"

class IocpSocketHandler
{
	//클래스 명세
	//Iocp 통신을 하기 위한 가장 하단의 객체
	//Iocp 소켓 그 자체
	//IocpSocketManager에서만 사용한다

public:
	IocpSocketHandler();
	~IocpSocketHandler();

	inline const bool			isConnected()	const { return _isConnected; }
	inline const UINT32			getIndex()		const { return _index; }
	inline SOCKET				getSocket()		{ return _mySock; }
	inline OverlappedIOBuffer&	getRecvBuffer() { return _overlappedRecvBuffer; }
	inline OverlappedIOBuffer&	getSendBuffer() { return _overlappedSendBuffer; }

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

class IocpCommunicationManager
{
	//클래스 명세
	// Iocp 통신을 도와주는 매니저
	// 여러개의 소켓을 생성하고 관리한다.
	// 통신 결과에 따라 콜백 함수가 실행된다.
	// 

public:
	IocpCommunicationManager();
	~IocpCommunicationManager();

	bool			createIOCP(const UINT32 maxIOThreadCount);
	IocpErrorCode	bindAndListen(const int bindPort);
	bool			connect(const std::string& ipAddress, const int bindPort);
	bool			runThread();
	void			shutdown();

private:
	WSADATA							_wsaData;
	SOCKET							_serverSock;
	HANDLE							_iocpHandle = nullptr;
	UINT32							_maxIOThreadCount = 0;

	bool							_wsaStartupResult = false;
	bool							_isCreateIOCP = false;
};