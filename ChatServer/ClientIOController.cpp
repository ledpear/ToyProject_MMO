#include <ws2tcpip.h>
#include <mswsock.h>

#include "ServerDefine.h"
#include "../Define/PacketDefine.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")  // AcceptEx()

ClientIOController::ClientIOController(const UINT32 index)
	: _index(index)
{
	ZeroMemory(&_sendBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&_recvBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&_acceptIOInfo, sizeof(OverlappedIOInfo));
}

ClientIOController::~ClientIOController()
{
	__noop;
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

	_isConnected = true;
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

	if (AcceptEx(listenSock, _clientSock, _acceptBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, (LPWSAOVERLAPPED) &_acceptIOInfo) == FALSE )
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

	SOCKADDR_IN	clientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	char clientIP[32] = { 0, };
	inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, 32 - 1);
	printf("Accept Completion Client : IP(%s) SOCKET(%d)\n", clientIP, static_cast<int>(_clientSock));

	return true;
}

bool ClientIOController::bindRecv()
{
	DWORD flag = 0;
	DWORD numBytes = 0;

	//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
	_recvBuffer._overlappedIOInfo._wsaBuf.len = CHAT_BUF_SIZE;
	_recvBuffer._overlappedIOInfo._wsaBuf.buf = _recvBuffer._buffer;
	_recvBuffer._overlappedIOInfo._operationType = OperationType::RECV;

	std::lock_guard<std::mutex> lockGuard(_recvBuffer._mutex);
	const int result = WSARecv(_clientSock, &(_recvBuffer._overlappedIOInfo._wsaBuf), 1, &numBytes, &flag, reinterpret_cast<LPWSAOVERLAPPED>(&(_recvBuffer._overlappedIOInfo)), NULL);
	if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[bindRecv] WSARecv()함수 실패 : %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

bool ClientIOController::sendMsg(const UINT32 dataSize, const std::string& msgStirng)
{
	OverlappedIOInfo* sendOverlappedIOInfo = new OverlappedIOInfo();
	std::lock_guard<std::mutex> guard(_sendBuffer._mutex);

	ZeroMemory(sendOverlappedIOInfo, sizeof(OverlappedIOInfo));
	sendOverlappedIOInfo->_wsaBuf.len = dataSize;
	sendOverlappedIOInfo->_wsaBuf.buf = new char[dataSize];
	CopyMemory(sendOverlappedIOInfo->_wsaBuf.buf, msgStirng.c_str(), dataSize);
	sendOverlappedIOInfo->_operationType = OperationType::SEND;

	DWORD numBytes = 0;
	const int result = WSASend(_clientSock, &(sendOverlappedIOInfo->_wsaBuf), 1, &numBytes, 0, reinterpret_cast<LPWSAOVERLAPPED>(&sendOverlappedIOInfo), NULL);
	if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[sendMsg] WSASend()함수 실패 : %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

bool ClientIOController::sendCompletion(OverlappedIOInfo* sendOverlappedIOInfo)
{
	printf_s("[SendCompletion] : Index(%d)\n", _index);
	std::lock_guard<std::mutex> guard(_sendBuffer._mutex);

	//스마트포인터로 할 수 있으면 하자
	//sendOverlappedIOInfo가 보낸 객채와 동일한지 확인
	delete[] sendOverlappedIOInfo->_wsaBuf.buf;
	delete sendOverlappedIOInfo;

	return true;
}

void ClientIOController::close(bool isForce)
{
	linger closeLinger = { 0, 0 };	// SO_DONTLINGER로 설정

	if (isForce == false)
	{
		closeLinger.l_onoff = 1;
	}

	shutdown(_clientSock, SD_BOTH);
	setsockopt(_clientSock, SOL_SOCKET, SO_LINGER, (char*)&closeLinger, sizeof(closeLinger));
	closesocket(_clientSock);
	_clientSock = INVALID_SOCKET;
	_isConnected = false;
}
