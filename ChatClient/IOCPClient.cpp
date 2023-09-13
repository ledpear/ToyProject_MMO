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

	if (_socketIocpController.initialize(0, _iocpHandle) == false)
	{
		printf_s("[initialize] Socket IOCP Controller Initialize Fail\n");
		return false;
	}

	return true;
}

bool IOCPClient::connectServer(const std::string& ipAddress, const int bindPort, std::function<void(bool)> callback)
{
	//소켓 주소 정보
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(bindPort);

	//dns 사용
	std::string myServer = "tjsrb7575.iptime.org";
	hostent* host = gethostbyname(myServer.c_str());
	serverAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*reinterpret_cast<in_addr*>(host->h_addr_list[0])));

	if (_socketIocpController.connectAsync(serverAddr) == false)
	{
		printf("[connectServer] connectAsync Fail\n");
		return false;
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
		if (_socketIocpController.bindRecv() == false)
		{
			printf("[run] bindRecv Fail\n");
			_socketIocpController.close();
			return;
		}
		
		_isWorkThreadRun = true;
		_runThread = std::thread([this]() { workThreadMain(); });
		printf_s("IOCPClient Start.\n");
	}

	while (true)
	{
		std::cin >> sendMsg;
		if ((_strcmpi(sendMsg.c_str(), "q") == 0) || (_strcmpi(sendMsg.c_str(), "quit") == 0))
		{
			//연결 종료
			linger closeLinger = { 1, 0 };
			shutdown(_clientSock, SD_BOTH);
			setsockopt(_clientSock, SOL_SOCKET, SO_LINGER, reinterpret_cast<char*>(&closeLinger), sizeof(closeLinger));
			closesocket(_clientSock);
			_clientSock = INVALID_SOCKET;
			_runThread.join();
			printf_s("IOCPClient End\n");
			break;
		}

		const ULONG msgLength = static_cast<ULONG>(sendMsg.length());
		OverlappedIOInfo* sendOverlappedIOInfo = new OverlappedIOInfo();

		ZeroMemory(sendOverlappedIOInfo, sizeof(OverlappedIOInfo));
		sendOverlappedIOInfo->_wsaBuf.len = msgLength;
		sendOverlappedIOInfo->_wsaBuf.buf = new char[msgLength];
		CopyMemory(sendOverlappedIOInfo->_wsaBuf.buf, sendMsg.c_str(), msgLength);
		sendOverlappedIOInfo->_operationType = OperationType::SEND;

		DWORD numBytes = 0;
		DWORD flag = 0;
		const int result = WSASend(_clientSock, &(sendOverlappedIOInfo->_wsaBuf), 1, &numBytes, flag, NULL, NULL);

		if ((result == SOCKET_ERROR) && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[sendMsg] WSASend() Func Fail : %d\n", WSAGetLastError());
		}
	}
}

void IOCPClient::workThreadMain()
{
	bool				isSuccess = false;
	DWORD				ioSize = 0;
	LPOVERLAPPED		lpOverlapped = nullptr;
	SocketIocpController* socketIocpController = nullptr;

	while (_isWorkThreadRun)
	{
		isSuccess = GetQueuedCompletionStatus(_iocpHandle, &ioSize, reinterpret_cast<PULONG_PTR>(&socketIocpController), &lpOverlapped, INFINITE);

		if ((isSuccess == true) && (ioSize == 0) && (lpOverlapped == nullptr))
		{
			_isWorkThreadRun = false;
			continue;
		}

		if ((lpOverlapped == nullptr) || socketIocpController == nullptr)
			continue;

		OverlappedIOInfo* overlappedIOInfo = reinterpret_cast<OverlappedIOInfo*>(lpOverlapped);
		if ((isSuccess == false) || ((overlappedIOInfo->_operationType != OperationType::CONNECT) && (ioSize == 0)))
		{
			//클라이언트 연결 해제 요청
			continue;
		}

		switch (overlappedIOInfo->_operationType)
		{
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
			std::string msgString(overlappedIOInfo->_wsaBuf.buf);
			printf("%s\n", msgString.c_str());

			if (_socketIocpController.bindRecv() == false)
			{
				printf("[run] bindRecv Fail\n");
				_socketIocpController.close();
				return;
			}
		}
		break;
		default:
		{
			printf_s("비정상적인 OperationType입니다 [client index : %d]\n", socketIocpController->getIndex());
		}
		break;
		}

	}
}