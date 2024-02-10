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
	//IocpCommunicationManager를 통해서 사용

	friend class IocpCommunicationManager;

public:
	IocpSocketHandler();
	~IocpSocketHandler();
	inline const bool			isSocketConnected()	const { return _isSocketConnected; }
	inline const bool			isIocpConnected()	const { return _isIocpConnected; }
	inline const UINT32			getIndex()			const { return _index; }

private:
	inline void					connectComplete()	{ _isSocketConnected = true; }
	inline SOCKET&				getSocket()			{ return _iocpSocket; }
	inline OverlappedIOBuffer&	getAcceptBuffer()	{ return _overlappedAcceptBuffer; }
	inline OverlappedIOBuffer&	getSendBuffer()		{ return _overlappedSendBuffer; }
	inline OverlappedIOBuffer&	getReceiveBuffer()	{ return _overlappedReceiveBuffer; }

	DWORD initialize(const UINT32 index = 0);
	DWORD acceptAsync(SOCKET listenSocket);
	DWORD connectSocketAsync(const std::string& ipAddress, const int bindPort);
	void getAcceptAddressInfo(_Out_ std::string* acceptIp = nullptr, _Out_ int* acceptPort = nullptr);

	int bindAddressInfo(const ULONG ipAddress, const USHORT bindPort);
	int bindAddressInfo(const std::string& ipAddress, const int bindPort);
	int connectSocket(const std::string& ipAddress, const int bindPort);
	int listenStart();

	DWORD bindReceive();
	DWORD sendMsg(const std::string& msgStirng);
	void close(bool isForce = false);

	void resetBufferAndSetOverlappedIOInfo(OverlappedIOBuffer& overlappedIOBuffer, OperationType type);

private:
	OverlappedIOBuffer	_overlappedAcceptBuffer;
	OverlappedIOBuffer	_overlappedSendBuffer;
	OverlappedIOBuffer	_overlappedReceiveBuffer;

	LPFN_CONNECTEX		_connectEx = nullptr;
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
	// 레거시 소켓 함수는 여기서 사용하지 않는다. IocpSocketHandler에서만 다룬다.

public:
	template<typename classType, typename funcType>
	explicit IocpCommunicationManager(classType* ownerInstance, funcType callBack_closeSocket, funcType callBack_accept, funcType callBack_connect, funcType callBack_send, funcType callBack_receive)
		: _callBack_closeSocket(std::bind(callBack_closeSocket, ownerInstance, std::placeholders::_1, std::placeholders::_2))
		, _callBack_accept(std::bind(callBack_accept, ownerInstance, std::placeholders::_1, std::placeholders::_2))
		, _callBack_connect(std::bind(callBack_connect, ownerInstance, std::placeholders::_1, std::placeholders::_2))
		, _callBack_send(std::bind(callBack_send, ownerInstance, std::placeholders::_1, std::placeholders::_2))
		, _callBack_receive(std::bind(callBack_receive, ownerInstance, std::placeholders::_1, std::placeholders::_2))
	{
		_wsaStartupResult = (WSAStartup(MAKEWORD(2, 2), &_wsaData) == 0);
	}
	~IocpCommunicationManager();

	IocpErrorCode	createIocp(const UINT32 maxIOThreadCount);
	IocpErrorCode	initializeAndConnectIocpSocketHandler(IocpSocketHandler& targetSocketHandler, const UINT32 index = 0);

	IocpErrorCode	bindAndListen(IocpSocketHandler& targetSocketHandler, const int bindPort);
	IocpErrorCode	acceptSocket(IocpSocketHandler& targetSocketHandler, IocpSocketHandler& listenSocketHandler);
	IocpErrorCode	connectSocket(IocpSocketHandler& targetSocketHandler, const std::string& ipAddress, const int bindPort);
	IocpErrorCode	connectSocketAsync(IocpSocketHandler& targetSocketHandler, const std::string& ipAddress, const int bindPort);
	void			acceptComplete(IocpSocketHandler& targetSocketHandler, _Out_ std::string* acceptIp = nullptr, _Out_ int* acceptPort = nullptr);
	void			connectComplete(IocpSocketHandler& targetSocketHandler);
	IocpErrorCode	receiveSocket(IocpSocketHandler& targetSocketHandler);
	IocpErrorCode	sendMsgSocket(IocpSocketHandler& targetSocketHandler, const std::string& msgStirng);
	IocpErrorCode	closeSocket(IocpSocketHandler& targetSocketHandler, bool isForce = false);

	IocpErrorCode	workIocpQueue(const DWORD timeoutMilliseconds);

	void			getReceiveMsg(IocpSocketHandler& targetSocketHandler, std::string& receiveMsg);

private:
	void			setSocketConnected(IocpSocketHandler& targetSocketHandler);

	WSADATA							_wsaData;

	const std::function<void(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo)>	_callBack_closeSocket;
	const std::function<void(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo)>	_callBack_accept;
	const std::function<void(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo)>	_callBack_connect;
	const std::function<void(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo)>	_callBack_send;
	const std::function<void(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo)>	_callBack_receive;

	HANDLE							_iocpHandle = nullptr;
	UINT32							_maxIOThreadCount = 0;

	bool							_wsaStartupResult = false;
	bool							_isCreateIOCP = false;
};