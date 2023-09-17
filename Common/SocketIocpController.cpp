#include <ws2tcpip.h>
#include <mswsock.h>

#include "PacketDefine.h"
#include "SocketIocpController.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")  // AcceptEx()

SocketIocpController::SocketIocpController()
{
	ZeroMemory(&_overlappedSendBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&_overlappedRecvBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&_overlappedAcceptBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
}

SocketIocpController::~SocketIocpController()
{
	close();
}

bool SocketIocpController::initialize(const UINT32 index, HANDLE iocpHandle)
{
	_index = index;
	_IOCPHandle = iocpHandle;
	_isConnected = false;

	_mySock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_mySock == INVALID_SOCKET)
	{
		printf_s("[acceptAsync] Client Socket WSASocket Error : %d\n", GetLastError());
		return false;
	}

	return true;
}

bool SocketIocpController::connectIOCP()
{
	HANDLE resultHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(getSocket()), _IOCPHandle, reinterpret_cast<ULONG_PTR>(this), 0);
	if (resultHandle == INVALID_HANDLE_VALUE)
	{
		printf_s("[connectIOCP] CreateIoCompletionPort fail : %d\n", getIndex());
		return false;
	}

	return true;
}

bool SocketIocpController::acceptAsync(SOCKET listenSock)
{
	OverlappedIOInfo& acceptIOInfo = _overlappedAcceptBuffer._overlappedIOInfo;
	ZeroMemory(&acceptIOInfo, sizeof(OverlappedIOInfo));

	DWORD bytes = 0;
	DWORD flags = 0;
	acceptIOInfo._wsaBuf.len = 0;
	acceptIOInfo._wsaBuf.buf = nullptr;
	acceptIOInfo._operationType = OperationType::ACCEPT;
	acceptIOInfo._index = _index;

	if (AcceptEx(listenSock, _mySock, _overlappedAcceptBuffer._buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, reinterpret_cast<LPWSAOVERLAPPED>(&acceptIOInfo)) == FALSE)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf_s("[acceptAsync] AcceptEx Error : %d\n", GetLastError());
			return false;
		}
	}

	return true;
}

bool SocketIocpController::acceptCompletion()
{
	printf_s("[acceptCompletion] : Index(%d)\n", _index);

	if (connectIOCP() == false)
		return false;

	_isConnected = true;

	SOCKADDR* localAddr = nullptr;
	SOCKADDR* remoteAddr = nullptr;
	int localSize, remoteSize = 0;
	char clientIP[32] = { 0, };
	GetAcceptExSockaddrs(_overlappedAcceptBuffer._buffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &localAddr, &localSize, &remoteAddr, &remoteSize);

	inet_ntop(AF_INET, &(reinterpret_cast<SOCKADDR_IN*>(remoteAddr)->sin_addr), clientIP, sizeof(clientIP));
	printf("Accept Completion Client : IP(%s) SOCKET(%d)\n", clientIP, reinterpret_cast<SOCKADDR_IN*>(remoteAddr)->sin_port);

	return true;
}

bool SocketIocpController::bindRecv()
{
	DWORD flag = 0;
	DWORD numBytes = 0;
	std::lock_guard<std::mutex> guard(_overlappedRecvBuffer._mutex);

	resetBufferAndSetOverlappedIOInfo(_overlappedRecvBuffer, OperationType::RECV);
	const int result = WSARecv(_mySock, &(_overlappedRecvBuffer._overlappedIOInfo._wsaBuf),
		1, &numBytes, &flag, reinterpret_cast<LPWSAOVERLAPPED>(&(_overlappedRecvBuffer._overlappedIOInfo)), NULL);

	if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[bindRecv:%d] WSARecv()함수 실패 : %d\n", _index, WSAGetLastError());
		return false;
	}

	return true;
}

bool SocketIocpController::sendMsg(const std::string& msgStirng)
{
	std::lock_guard<std::mutex> guard(_overlappedSendBuffer._mutex);

	resetBufferAndSetOverlappedIOInfo(_overlappedSendBuffer, OperationType::SEND);
	CopyMemory(_overlappedSendBuffer._overlappedIOInfo._wsaBuf.buf, msgStirng.c_str(), msgStirng.size());

	DWORD flag = 0;
	DWORD numBytes = 0;
	const int result = WSASend(_mySock, &(_overlappedSendBuffer._overlappedIOInfo._wsaBuf), 1, &numBytes, flag, 
		reinterpret_cast<LPWSAOVERLAPPED>(&(_overlappedSendBuffer._overlappedIOInfo)), NULL);
	if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[sendMsg:%d] WSASend()함수 실패 : %d\n", _index, WSAGetLastError());
		return false;
	}

	return true;
}

void SocketIocpController::close(bool isForce)
{
	linger closeLinger = { 0, 0 };	// SO_DONTLINGER로 설정
	if (isForce == false)
		closeLinger.l_onoff = 1;

	shutdown(_mySock, SD_BOTH);
	setsockopt(_mySock, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&closeLinger), sizeof(closeLinger));
	closesocket(_mySock);
	_mySock = INVALID_SOCKET;
	_isConnected = false;
}

void SocketIocpController::resetBufferAndSetOverlappedIOInfo(OverlappedIOBuffer& overlappedIOBuffer, OperationType type)
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