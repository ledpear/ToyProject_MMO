#include <ws2tcpip.h>
#include <mswsock.h>

#include "ServerDefine.h"
#include "../Define/PacketDefine.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")  // AcceptEx()

ClientIOController::ClientIOController()
{
	ZeroMemory(&_sendBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&_recvBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&_acceptIOInfo, sizeof(OverlappedIOInfo));
}

ClientIOController::~ClientIOController()
{
	close();
}

void ClientIOController::initialize(const UINT32 index, HANDLE iocpHandle)
{
	_index = index;
	_IOCPHandle = iocpHandle;
	_isConnected = false;
}

bool ClientIOController::connectIOCP()
{
	HANDLE resultHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(getSocket()), _IOCPHandle, reinterpret_cast<ULONG_PTR>(this), 0);
	if (resultHandle == INVALID_HANDLE_VALUE)
	{
		printf_s("[connectIOCP] CreateIoCompletionPort fail : %d\n", getIndex());
		return false;
	}

	return true;
}

bool ClientIOController::onAccept(SOCKET listenSock)
{
	_clientSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_clientSock == INVALID_SOCKET)
	{
		printf_s("[onAccept] Client Socket WSASocket Error : %d\n", GetLastError());
		return false;
	}

	ZeroMemory(&_acceptIOInfo, sizeof(OverlappedIOInfo));

	DWORD bytes = 0;
	DWORD flags = 0;
	_acceptIOInfo._wsaBuf.len = 0;
	_acceptIOInfo._wsaBuf.buf = nullptr;
	_acceptIOInfo._operationType = OperationType::ACCEPT;
	_acceptIOInfo._index = _index;

	if (AcceptEx(listenSock, _clientSock, _acceptBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, reinterpret_cast<LPWSAOVERLAPPED>(&_acceptIOInfo)) == FALSE)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf_s("[onAccept] AcceptEx Error : %d\n", GetLastError());
			return false;
		}
	}
	else
	{
		printf_s("[onAccept] client Index: %d\n", getIndex());
	}

	return true;
}

bool ClientIOController::AcceptCompletion()
{
	printf_s("[AcceptCompletion] : Index(%d)\n", _index);

	if (connectIOCP() == false)
		return false;

	_isConnected = true;

	SOCKADDR* localAddr = nullptr;
	SOCKADDR* remoteAddr = nullptr;
	int localSize, remoteSize = 0;
	char clientIP[32] = { 0, };
	GetAcceptExSockaddrs(_acceptBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &localAddr, &localSize, &remoteAddr, &remoteSize);

	inet_ntop(AF_INET, &(reinterpret_cast<SOCKADDR_IN*>(remoteAddr)->sin_addr), clientIP, sizeof(clientIP));
	printf("Accept Completion Client : IP(%s) SOCKET(%d)\n", clientIP, reinterpret_cast<SOCKADDR_IN*>(remoteAddr)->sin_port);
	if (bindRecv() == false)
		return false;

	return true;
}

bool ClientIOController::bindRecv()
{
	DWORD flag = 0;
	DWORD numBytes = 0;
	std::lock_guard<std::mutex> guard(_recvBuffer._mutex);

	resetBufferAndSetOverlappedIOInfo(_recvBuffer, OperationType::RECV);
	const int result = WSARecv(_clientSock, &(_recvBuffer._overlappedIOInfo._wsaBuf), 1, &numBytes, &flag, reinterpret_cast<LPWSAOVERLAPPED>(&(_recvBuffer._overlappedIOInfo)), NULL);
	if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[bindRecv:%d] WSARecv()함수 실패 : %d\n", _index, WSAGetLastError());
		return false;
	}

	return true;
}

bool ClientIOController::sendMsg(const UINT32 dataSize, const std::string& msgStirng)
{
	OverlappedIOInfo* sendOverlappedIOInfo = new OverlappedIOInfo();
	std::lock_guard<std::mutex> guard(_sendBuffer._mutex);

	resetBufferAndSetOverlappedIOInfo(_sendBuffer, OperationType::SEND);
	CopyMemory(_sendBuffer._overlappedIOInfo._wsaBuf.buf, msgStirng.c_str(), dataSize);

	DWORD flag = 0;
	DWORD numBytes = 0;
	const int result = WSASend(_clientSock, &(_sendBuffer._overlappedIOInfo._wsaBuf), 1, &numBytes, flag, reinterpret_cast<LPWSAOVERLAPPED>(&(_sendBuffer._overlappedIOInfo)), NULL);
	if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[sendMsg:%d] WSASend()함수 실패 : %d\n", _index, WSAGetLastError());
		return false;
	}

	return true;
}

void ClientIOController::close(bool isForce)
{
	linger closeLinger = { 0, 0 };	// SO_DONTLINGER로 설정
	if (isForce == false)
		closeLinger.l_onoff = 1;

	shutdown(_clientSock, SD_BOTH);
	setsockopt(_clientSock, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&closeLinger) , sizeof(closeLinger));
	closesocket(_clientSock);
	_clientSock = INVALID_SOCKET;
	_isConnected = false;
}

void ClientIOController::resetBufferAndSetOverlappedIOInfo(ThreadSafeBuffer& threadSafeBuffer, OperationType type)
{
	//Buffer 초기화
	ZeroMemory(&threadSafeBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&threadSafeBuffer._buffer, CHAT_BUF_SIZE);

	//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
	threadSafeBuffer._overlappedIOInfo._wsaBuf.len = CHAT_BUF_SIZE;
	threadSafeBuffer._overlappedIOInfo._wsaBuf.buf = threadSafeBuffer._buffer;
	threadSafeBuffer._overlappedIOInfo._operationType = type;
	threadSafeBuffer._overlappedIOInfo._index = _index;
}
