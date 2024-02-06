// IocpSocketHandler.cpp : 정적 라이브러리를 위한 함수를 정의합니다.
//

#include "pch.h"
#include "framework.h"

#include <ws2tcpip.h>
#include <mswsock.h>

#include "PacketDefine.h"
#include "IocpCommunication.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")  // AcceptEx()

IocpSocketHandler::IocpSocketHandler()
{
	ZeroMemory(&_overlappedSendBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&_overlappedReceiveBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&_overlappedAcceptBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
}

IocpSocketHandler::~IocpSocketHandler()
{
	close();
}

DWORD IocpSocketHandler::initialize(const UINT32 index)
{
	_index = index;
	_isSocketConnected = false;

	_iocpSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_iocpSocket == INVALID_SOCKET)
		return GetLastError();

	return ERROR_SUCCESS;
}

DWORD IocpSocketHandler::acceptAsync(SOCKET listenSocket)
{
	//if (_isIocpConnected == false)
	//	return WSAENOTCONN;

	OverlappedIOInfo& acceptIOInfo = _overlappedAcceptBuffer._overlappedIOInfo;
	ZeroMemory(&acceptIOInfo, sizeof(OverlappedIOInfo));

	DWORD bytes = 0;
	DWORD flags = 0;
	acceptIOInfo._wsaBuf.len = 0;
	acceptIOInfo._wsaBuf.buf = nullptr;
	acceptIOInfo._operationType = OperationType::ACCEPT;
	acceptIOInfo._index = _index;

	if (AcceptEx(listenSocket, _iocpSocket, _overlappedAcceptBuffer._buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, reinterpret_cast<LPWSAOVERLAPPED>(&acceptIOInfo)) == FALSE)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return GetLastError();
	}

	return ERROR_SUCCESS;
}

void IocpSocketHandler::getAcceptAddressInfo(_Out_ std::string& acceptIp, _Out_ int& acceptPort)
{
	SOCKADDR* localAddr = nullptr;
	SOCKADDR* remoteAddr = nullptr;
	int localSize, remoteSize = 0;
	char clientIP[46] = { 0, };
	GetAcceptExSockaddrs(_overlappedAcceptBuffer._buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &localAddr, &localSize, &remoteAddr, &remoteSize);

	inet_ntop(AF_INET, &(reinterpret_cast<SOCKADDR_IN*>(remoteAddr)->sin_addr), clientIP, sizeof(clientIP));
	acceptIp = clientIP;
	acceptPort = reinterpret_cast<SOCKADDR_IN*>(remoteAddr)->sin_port;
}

int IocpSocketHandler::bindAddressInfo(const ULONG ipAddress, const USHORT bindPort)
{
	//소켓 주소 정보
	SOCKADDR_IN bindAddressInfo;
	ZeroMemory(&bindAddressInfo, sizeof(bindAddressInfo));
	bindAddressInfo.sin_family = AF_INET;
	bindAddressInfo.sin_addr.s_addr = ipAddress;
	bindAddressInfo.sin_port = htons(bindPort);;

	//bind
	int bindResult = bind(_iocpSocket, reinterpret_cast<sockaddr*>(&bindAddressInfo), sizeof(SOCKADDR_IN));
	if (bindResult == SOCKET_ERROR)
		closesocket(_iocpSocket);

	return bindResult;
}

int IocpSocketHandler::bindAddressInfo(const std::string& ipAddress, const int bindPort)
{
	//소켓 주소 정보
	SOCKADDR_IN bindAddressInfo;
	ZeroMemory(&bindAddressInfo, sizeof(bindAddressInfo));
	bindAddressInfo.sin_family = AF_INET;
	bindAddressInfo.sin_addr.s_addr = inet_addr(ipAddress.c_str());
	bindAddressInfo.sin_port = htons(bindPort);

	//bind
	int bindResult = bind(_iocpSocket, reinterpret_cast<sockaddr*>(&bindAddressInfo), sizeof(SOCKADDR_IN));
	if (bindResult == SOCKET_ERROR)
		closesocket(_iocpSocket);

	return bindResult;
}

int IocpSocketHandler::connectSocket(const std::string& ipAddress, const int bindPort)
{
	//소켓 주소 정보
	SOCKADDR_IN connectAddressInfo;
	connectAddressInfo.sin_family = AF_INET;
	connectAddressInfo.sin_port = htons(bindPort);
	connectAddressInfo.sin_addr.s_addr = inet_addr(ipAddress.c_str());

	return connect(_iocpSocket, (sockaddr*)&connectAddressInfo, sizeof(sockaddr));
}

int IocpSocketHandler::listenStart()
{
	int bindResult = listen(_iocpSocket, static_cast<int>(BACKLOG_SIZE));
	if (bindResult == SOCKET_ERROR)
		closesocket(_iocpSocket);

	return bindResult;
}

DWORD IocpSocketHandler::bindReceive()
{
	DWORD flag = 0;
	DWORD numBytes = 0;
	std::lock_guard<std::mutex> guard(_overlappedReceiveBuffer._mutex);

	resetBufferAndSetOverlappedIOInfo(_overlappedReceiveBuffer, OperationType::RECV);
	const int result = WSARecv(_iocpSocket, &(_overlappedReceiveBuffer._overlappedIOInfo._wsaBuf),
		1, &numBytes, &flag, reinterpret_cast<LPWSAOVERLAPPED>(&(_overlappedReceiveBuffer._overlappedIOInfo)), NULL);

	const DWORD lastError = WSAGetLastError();
	if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
		return lastError;

	return ERROR_SUCCESS;
}

DWORD IocpSocketHandler::sendMsg(const std::string& msgStirng)
{
	std::lock_guard<std::mutex> guard(_overlappedSendBuffer._mutex);

	resetBufferAndSetOverlappedIOInfo(_overlappedSendBuffer, OperationType::SEND);
	CopyMemory(_overlappedSendBuffer._overlappedIOInfo._wsaBuf.buf, msgStirng.c_str(), msgStirng.size());

	DWORD flag = 0;
	DWORD numBytes = 0;
	const int result = WSASend(_iocpSocket, &(_overlappedSendBuffer._overlappedIOInfo._wsaBuf), 1, &numBytes, flag,
		reinterpret_cast<LPWSAOVERLAPPED>(&(_overlappedSendBuffer._overlappedIOInfo)), NULL);

	const DWORD lastError = WSAGetLastError();
	if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
		return lastError;

	return ERROR_SUCCESS;
}

void IocpSocketHandler::close(bool isForce)
{
	linger closeLinger = { 0, 0 };	// SO_DONTLINGER로 설정
	if (isForce == false)
		closeLinger.l_onoff = 1;

	shutdown(_iocpSocket, SD_BOTH);
	setsockopt(_iocpSocket, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&closeLinger), sizeof(closeLinger));
	closesocket(_iocpSocket);
	_iocpSocket = INVALID_SOCKET;
	_isSocketConnected = false;
}

void IocpSocketHandler::resetBufferAndSetOverlappedIOInfo(OverlappedIOBuffer& overlappedIOBuffer, OperationType type)
{
	//Buffer 초기화
	ZeroMemory(&overlappedIOBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&overlappedIOBuffer._buffer, CHAT_BUF_SIZE);

	//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
	overlappedIOBuffer._overlappedIOInfo._wsaBuf.len = CHAT_BUF_SIZE;
	overlappedIOBuffer._overlappedIOInfo._wsaBuf.buf = overlappedIOBuffer._buffer;
	overlappedIOBuffer._overlappedIOInfo._operationType = type;
	overlappedIOBuffer._overlappedIOInfo._index = _index;
}

IocpCommunicationManager::~IocpCommunicationManager()
{
	if (_wsaStartupResult)
		WSACleanup();
}

IocpErrorCode IocpCommunicationManager::createIocp(const UINT32 maxIOThreadCount)
{
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxIOThreadCount);
	if (_iocpHandle == nullptr)
		return IocpErrorCode::IOCP_ERROR_FAIL_CREATE_IOCP;

	_maxIOThreadCount = maxIOThreadCount;
	_isCreateIOCP = true;
	return IocpErrorCode::NOT_IOCP_ERROR;
}

IocpErrorCode IocpCommunicationManager::initializeAndConnectIocpSocketHandler(IocpSocketHandler& targetSocketHandler, const UINT32 index)
{
	if(targetSocketHandler.initialize(index) != ERROR_SUCCESS)
		return IocpErrorCode::IOCP_ERROR_FAIL_INITIALIZE_SOCKET;

	HANDLE resultHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(targetSocketHandler.getSocket()), _iocpHandle, reinterpret_cast<ULONG_PTR>(&targetSocketHandler), 0);
	if (resultHandle == INVALID_HANDLE_VALUE)
		return IocpErrorCode::IOCP_ERROR_FAIL_CONNECT_SOCKET_TO_IOCP;

	targetSocketHandler._isIocpConnected = true;
	return IocpErrorCode::NOT_IOCP_ERROR;
}

IocpErrorCode IocpCommunicationManager::bindAndListen(IocpSocketHandler& targetSocketHandler, const int bindPort)
{
	if (_isCreateIOCP == false)
		return IocpErrorCode::IOCP_ERROR_NOT_CREATE_IOCP;

	//if (targetSocketHandler.isIocpConnected() == false)
	//	return IocpErrorCode::IOCP_ERROR_SOCKET_NOT_CONNECT_IOCP;

	//소켓 주소 정보 bind
	if (targetSocketHandler.bindAddressInfo(ADDR_ANY, bindPort) == SOCKET_ERROR)
		return IocpErrorCode::IOCP_ERROR_FAIL_BIND_SOCKET;

	//listen
	if (targetSocketHandler.listenStart() == SOCKET_ERROR)
		return IocpErrorCode::IOCP_ERROR_FAIL_LISTEN_SOCKET;

	return IocpErrorCode::NOT_IOCP_ERROR;
}

IocpErrorCode IocpCommunicationManager::acceptSocket(IocpSocketHandler& targetSocketHandler, IocpSocketHandler& listenSocketHandler)
{
	if (targetSocketHandler.acceptAsync(listenSocketHandler.getSocket()) != ERROR_SUCCESS)
		return IocpErrorCode::IOCP_ERROR_FAIL_ASYNC_ACCEPT;
		
	return IocpErrorCode::NOT_IOCP_ERROR;
}

IocpErrorCode IocpCommunicationManager::connectSocket(IocpSocketHandler& targetSocketHandler, const std::string& ipAddress, const int bindPort)
{
	if (targetSocketHandler.isIocpConnected() == false)
		return IocpErrorCode::IOCP_ERROR_SOCKET_NOT_CONNECT_IOCP;

	if (targetSocketHandler.connectSocket(ipAddress, bindPort) == SOCKET_ERROR)
		return IocpErrorCode::IOCP_ERROR_FAIL_CONNECT_SOCKET;

	return IocpErrorCode::NOT_IOCP_ERROR;
}

void IocpCommunicationManager::connectSocketComplete(IocpSocketHandler& targetSocketHandler, _Out_ std::string& acceptIp, _Out_ int& acceptPort)
{
	targetSocketHandler._isSocketConnected = true;
	targetSocketHandler.getAcceptAddressInfo(acceptIp, acceptPort);
}

IocpErrorCode IocpCommunicationManager::receiveSocket(IocpSocketHandler& targetSocketHandler)
{
	if (targetSocketHandler.bindReceive() != ERROR_SUCCESS)
		return IocpErrorCode::IOCP_ERROR_FAIL_ASYNC_RECEIVE;

	return IocpErrorCode::NOT_IOCP_ERROR;
}

IocpErrorCode IocpCommunicationManager::sendMsgSocket(IocpSocketHandler& targetSocketHandler, const std::string& msgStirng)
{
	if (targetSocketHandler.isSocketConnected() == false)
		return IocpErrorCode::IOCP_ERROR_SOCKET_NOT_CONNECT_IOCP;

	if (targetSocketHandler.sendMsg(msgStirng) != ERROR_SUCCESS)
		return IocpErrorCode::IOCP_ERROR_FAIL_ASYNC_SEND;

	return IocpErrorCode::NOT_IOCP_ERROR;
}

IocpErrorCode IocpCommunicationManager::closeSocket(IocpSocketHandler& targetSocketHandler, bool isForce)
{
	targetSocketHandler.close();
	return IocpErrorCode::NOT_IOCP_ERROR;
}

IocpErrorCode IocpCommunicationManager::workIocpQueue(const DWORD timeoutMilliseconds)
{
	LPOVERLAPPED			lpOverlapped = nullptr;
	IocpSocketHandler*		iocpSocketHandler = nullptr;
	DWORD					ioSize = 0;
	bool					isSuccess = false;

	isSuccess = GetQueuedCompletionStatus(_iocpHandle, &ioSize, reinterpret_cast<PULONG_PTR>(&iocpSocketHandler), &lpOverlapped, timeoutMilliseconds);
	if ((isSuccess == true) && (ioSize == 0) && (lpOverlapped == nullptr))
		return IocpErrorCode::IOCP_ERROR_FAIL_COMMUNICATION;

	if (lpOverlapped == nullptr)
		return IocpErrorCode::IOCP_ERROR_INVALID_TASK;

	OverlappedIOInfo* overlappedIOInfo = reinterpret_cast<OverlappedIOInfo*>(lpOverlapped);
	if ((isSuccess == false) || ((overlappedIOInfo->_operationType != OperationType::ACCEPT) && (ioSize == 0)))
	{
		if (iocpSocketHandler != nullptr)
			_callBack_closeSocket(*iocpSocketHandler, *overlappedIOInfo);

		return IocpErrorCode::IOCP_DISCONNECT_REQUEST;
	}

	switch (overlappedIOInfo->_operationType)
	{
	case OperationType::ACCEPT:
	{		
		if (iocpSocketHandler != nullptr)
		{
			iocpSocketHandler->connectComplete();
			_callBack_accept(*iocpSocketHandler, *overlappedIOInfo);
		}
	}
	break;
	case OperationType::SEND:
	{
		if (iocpSocketHandler != nullptr)
			_callBack_send(*iocpSocketHandler, *overlappedIOInfo);
	}
	break;
	case OperationType::RECV:
	{
		if (iocpSocketHandler != nullptr)
			_callBack_receive(*iocpSocketHandler, *overlappedIOInfo);
	}
	break;
	default:
	break;
	}

	return IocpErrorCode::NOT_IOCP_ERROR;
}

void IocpCommunicationManager::getReceiveMsg(IocpSocketHandler& targetSocketHandler, std::string& receiveMsg)
{
	receiveMsg = targetSocketHandler.getReceiveBuffer()._buffer;
}
