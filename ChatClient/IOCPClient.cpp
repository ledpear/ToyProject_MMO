#include <ws2tcpip.h>
#include <mswsock.h>
#include <WinSock2.h>

#include "IOCPClient.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")  // ConnectEx()

IOCPClient::IOCPClient()
{
	_wsaStartupResult = (WSAStartup(MAKEWORD(2, 2), &_wsaData) == 0);
}

IOCPClient::~IOCPClient()
{
	closesocket(_clientSock);
	if (_wsaStartupResult)
		WSACleanup();
}

bool IOCPClient::initialize(const UINT32 maxIOThreadCount)
{
	//생성자에서 WSAStartup가 실패했으면 리턴
	if (_wsaStartupResult == false)
		return false;

	// IO Completion Port 생성
	_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, maxIOThreadCount);
	if (_iocpHandle == nullptr)
		return false;

	_maxIOThreadCount = maxIOThreadCount;

	//소켓 생성
	_clientSock = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (_clientSock == INVALID_SOCKET)
		return false;

	return true;
}

bool IOCPClient::connect(const std::string& ipAddress, const int bindPort, std::function<void(bool)> callback)
{
	//소켓 주소 정보
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(bindPort);
	unsigned long inAddr = 0;
	InetPton(AF_INET, reinterpret_cast<PCWSTR>(ipAddress.c_str()), &inAddr);
	serverAddr.sin_addr.S_un.S_addr = htonl(inAddr);

	ZeroMemory(&_connectIOInfo, sizeof(OverlappedIOInfo));
	_connectIOInfo._operationType = OperationType::CONNECT;
	HANDLE resultHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(_clientSock), _iocpHandle, reinterpret_cast<ULONG_PTR>(this), 0);

	GUID guid = WSAID_CONNECTEX;
	LPFN_CONNECTEX connectExFuntionPointer = nullptr;
	unsigned long bytes;
	WSAIoctl(_clientSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &connectExFuntionPointer, sizeof(connectExFuntionPointer), &bytes, nullptr, nullptr);

	int connectExResult = connectExFuntionPointer(_clientSock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr), nullptr, NULL, nullptr, reinterpret_cast<LPOVERLAPPED>(&_connectIOInfo));
	if (connectExResult == SOCKET_ERROR )
	{
		const int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			printf("[sendMsg] ConnectEx 실패 : %d\n", error);
			return false;
		}
	}

	return true;
}

void IOCPClient::run()
{
	//채팅
	//char	sendMsg[CHAT_BUF_SIZE];
	std::string sendMsg;
	{
		//쓰레드 실행 및 recv 시작
		bindRecv();
		_isWorkThreadRun = true;
		_runThread = std::thread([this]() { workThreadMain(); });
	}

	while (true)
	{
		std::cin >> sendMsg;
		if ((_strcmpi(sendMsg.c_str(), "q") == 0) || (_strcmpi(sendMsg.c_str(), "quit") == 0))
			break;

		const ULONG msgLength = static_cast<ULONG>(sendMsg.length());
		OverlappedIOInfo* sendOverlappedIOInfo = new OverlappedIOInfo();

		ZeroMemory(sendOverlappedIOInfo, sizeof(OverlappedIOInfo));
		sendOverlappedIOInfo->_wsaBuf.len = msgLength;
		sendOverlappedIOInfo->_wsaBuf.buf = new char[msgLength];
		CopyMemory(sendOverlappedIOInfo->_wsaBuf.buf, sendMsg.c_str(), msgLength);
		sendOverlappedIOInfo->_operationType = OperationType::SEND;

		DWORD numBytes = 0;
		const int result = WSASend(_clientSock, &(sendOverlappedIOInfo->_wsaBuf), 1, &numBytes, 0, reinterpret_cast<LPWSAOVERLAPPED>(&sendOverlappedIOInfo), NULL);
		if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[sendMsg] WSASend()함수 실패 : %d\n", WSAGetLastError());
		}
	}
}

void IOCPClient::workThreadMain()
{
	bool				isSuccess = false;
	DWORD				ioSize = 0;
	LPOVERLAPPED		lpOverlapped = nullptr;
	ClientIOController* clientIOController = nullptr;

	while (_isWorkThreadRun)
	{
		isSuccess = GetQueuedCompletionStatus(_iocpHandle, &ioSize, reinterpret_cast<PULONG_PTR>(clientIOController), &lpOverlapped, INFINITE);

		if ((isSuccess == true) && (ioSize == 0) && (lpOverlapped == nullptr))
		{
			_isWorkThreadRun = false;
			continue;
		}

		if ((lpOverlapped == nullptr) || clientIOController == nullptr)
			continue;

		OverlappedIOInfo* overlappedIOInfo = reinterpret_cast<OverlappedIOInfo*>(lpOverlapped);
		if ((isSuccess == false) || ((overlappedIOInfo->_operationType != OperationType::CONNECT) && (ioSize == 0)))
		{
			//클라이언트 연결 해제 요청
			continue;
		}

		switch (overlappedIOInfo->_operationType)
		{
		case OperationType::CONNECT:
		{
			_connectCompleteCallBack(true);
		}
		break;
		case OperationType::SEND:
		{
			//send  완료
			printf("[sendMsg] send complate\n");
			delete[] overlappedIOInfo->_wsaBuf.buf;
			delete overlappedIOInfo;
		}
		break;
		case OperationType::RECV:
		{
			std::string msgString(clientIOController->getSendBuffer()._buffer);
			printf("%s\n", msgString.c_str());

			bindRecv();
		}
		break;
		default:
		{
			printf_s("비정상적인 OperationType입니다 [client index : %d]\n", clientIOController->getIndex());
		}
		break;
		}

	}
}

bool IOCPClient::bindRecv()
{
	DWORD flag = 0;
	DWORD numBytes = 0;

	//Overlapped I/O을 위해 각 정보를 셋팅해 준다.
	std::lock_guard<std::mutex> lockGuard(_recvBuffer._mutex);
	ZeroMemory(&_recvBuffer._overlappedIOInfo, sizeof(OverlappedIOInfo));
	ZeroMemory(&_recvBuffer._buffer, sizeof(_recvBuffer._buffer));
	_recvBuffer._overlappedIOInfo._wsaBuf.len = CHAT_BUF_SIZE;
	_recvBuffer._overlappedIOInfo._wsaBuf.buf = _recvBuffer._buffer;
	_recvBuffer._overlappedIOInfo._operationType = OperationType::RECV;

	const int result = WSARecv(_clientSock, &(_recvBuffer._overlappedIOInfo._wsaBuf), 1, &numBytes, &flag, reinterpret_cast<LPWSAOVERLAPPED>(&(_recvBuffer._overlappedIOInfo)), NULL);
	if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
	{
		printf("[bindRecv] WSARecv()함수 실패 : %d\n", WSAGetLastError());
		return false;
	}

	return true;
}