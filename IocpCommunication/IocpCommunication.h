#pragma once
#include <WinSock2.h>
#include <functional>
#include "PacketDefine.h"
#include "IocpErrorCode.h"

#include <mswsock.h>

class IocpSocketHandler
{
	//Ŭ���� ��
	//Iocp ����� �ϱ� ���� ���� �ϴ��� ��ü
	//Iocp ���� �� ��ü
	//IocpCommunicationManager�� ���ؼ� ���

	friend class IocpCommunicationManager;

public:
	IocpSocketHandler();
	~IocpSocketHandler();
	inline const bool			isSocketConnected()	const { return _isSocketConnected; }
	inline const bool			isIocpConnected()	const { return _isIocpConnected; }
	inline const UINT32			getIndex()			const { return _index; }

private:
	inline void					connectComplete() { _isSocketConnected = true; }
	inline SOCKET& getSocket() { return _iocpSocket; }
	inline OverlappedIOBuffer& getAcceptBuffer() { return _overlappedAcceptBuffer; }
	inline OverlappedIOBuffer& getSendBuffer() { return _overlappedSendBuffer; }
	inline OverlappedIOBuffer& getReceiveBuffer() { return _overlappedReceiveBuffer; }

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
	//Ŭ���� ��
	// Iocp ����� �����ִ� �Ŵ���
	// �������� ������ �����ϰ� �����Ѵ�.
	// -> ������ �����ϴ°� �ܺο��� �ϰ� ���⼱ ��Ʈ�Ѹ��Ѵ�
	// ��� ����� ���� �ݹ� �Լ��� ����ȴ�.
	// ���Ž� ���� �Լ��� ���⼭ ������� �ʴ´�. IocpSocketHandler������ �ٷ��.

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
	IocpErrorCode	receiveSocket(IocpSocketHandler& targetSocketHandler);
	IocpErrorCode	sendMsgSocket(IocpSocketHandler& targetSocketHandler, const std::string& msgStirng);
	IocpErrorCode	closeSocket(IocpSocketHandler& targetSocketHandler, bool isForce = false);

	IocpErrorCode	workIocpQueue(const DWORD timeoutMilliseconds);

	void			getReceiveMsg(IocpSocketHandler& targetSocketHandler, std::string& receiveMsg);

private:
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