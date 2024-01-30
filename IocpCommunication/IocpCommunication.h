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

	friend class IocpCommunicationManager;

public:
	IocpSocketHandler();
	~IocpSocketHandler();

	inline const bool			isSocketConnected()	const { return _isSocketConnected; }
	inline const bool			isIocpConnected()	const { return _isIocpConnected; }
	inline const UINT32			getIndex()			const { return _index; }
	inline SOCKET&				getSocket()			{ return _iocpSocket; }
	inline OverlappedIOBuffer&	getRecvBuffer()		{ return _overlappedRecvBuffer; }
	inline OverlappedIOBuffer&	getSendBuffer()		{ return _overlappedSendBuffer; }

	bool initialize(const UINT32 index);
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

	HANDLE				_IOCPHandle = INVALID_HANDLE_VALUE;//To do delete
	SOCKET				_iocpSocket = INVALID_SOCKET;

	INT32				_index = -1;//To do delete
	bool				_isSocketConnected = false;
	bool				_isIocpConnected = false;
};

class IocpCommunicationManager
{
	//클래스 명세
	// Iocp 통신을 도와주는 매니저
	// 여러개의 소켓을 생성하고 관리한다.
	// -> 소켓을 생성하는건 외부에서 하고 여기선 컨트롤만한다
	// 통신 결과에 따라 콜백 함수가 실행된다.
	// 

public:
	IocpCommunicationManager();
	~IocpCommunicationManager();

	IocpErrorCode	createIocp(const UINT32 maxIOThreadCount);
	IocpErrorCode	connectIocpSocketHandler(IocpSocketHandler& connectSocketHandler);

	IocpErrorCode	bindAndListen(IocpSocketHandler& connectSocketHandler, const int bindPort);
	IocpErrorCode	connectSocket(IocpSocketHandler& connectSocketHandler,const std::string& ipAddress, const int bindPort);

	//IOCP 작업을 하나씩 꺼내서 처리하는 함수
	const OverlappedIOInfo* getIocpTask(const DWORD timeoutMilliseconds);
	//각 작업 종류에 따라 콜백함수를 호출하는 함수
	IocpErrorCode	workIocpQueue(const DWORD timeoutMilliseconds);

	//IocpErrorCode	runThread();

private:
	WSADATA							_wsaData;

	std::function<void(IocpSocketHandler& socketIocpController, bool isForce)>	_callBack_closeSocket;
	std::function<void(IocpSocketHandler& socketIocpController, bool isForce)>	_callBack_accept;
	std::function<void(IocpSocketHandler& socketIocpController, bool isForce)>	_callBack_send;
	std::function<void(IocpSocketHandler& socketIocpController, bool isForce)>	_callBack_receive;

	//SOCKET							_listenSocket;
	HANDLE							_iocpHandle = nullptr;
	UINT32							_maxIOThreadCount = 0;


	bool							_wsaStartupResult = false;
	bool							_isCreateIOCP = false;
};