#include <ws2tcpip.h>
#include <mswsock.h>
#include <WinSock2.h>

#include "IOCPChatClient.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")  // ConnectEx()

IOCPChatClient::IOCPChatClient()
{
	_wsaStartupResult = (WSAStartup(MAKEWORD(2, 2), &_wsaData) == 0);
}

IOCPChatClient::~IOCPChatClient()
{
	_workThread.join();
	_socketIocpController.close();
	if (_wsaStartupResult)
		WSACleanup();
}

bool IOCPChatClient::initialize(const UINT32 maxIOThreadCount)
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

bool IOCPChatClient::connectServer(const std::string& ipAddress, const int bindPort, std::function<void(bool)> callback)
{
	//소켓 주소 정보
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(bindPort);
	serverAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());

	const int connectResult = connect(_socketIocpController.getSocket(), (sockaddr*)&serverAddr, sizeof(sockaddr));
	if (connectResult == SOCKET_ERROR) 
	{
		printf_s("[connectServer] Connect Socket Fail\n");
		return false;
	}

	if (_socketIocpController.connectIOCP() == false)
	{
		printf_s("[connectServer] Connect IOCP Fail\n");
		return false;
	}

	if (_socketIocpController.bindRecv() == false)
	{
		printf("[workThreadMain] bindRecv Fail\n");
		_socketIocpController.close();
		return false;
	}
	;
	return true;
}

void IOCPChatClient::run()
{
	{
		//쓰레드 실행
		if (_socketIocpController.bindRecv() == false)
		{
			printf("[run] bindRecv Fail\n");
			_socketIocpController.close();
			return;
		}
		
		_isWorkThreadRun = true;
		_workThread = std::thread([this]() { workThreadMain(); });
		printf_s("IOCPChatClient Start.\n");
	}

	std::string sendMsg;
	while (true)
	{
		std::cin >> sendMsg;
		if ((_strcmpi(sendMsg.c_str(), "q") == 0) || (_strcmpi(sendMsg.c_str(), "quit") == 0))
		{
			//연결 종료
			_socketIocpController.close();
			break;
		}

		if (_socketIocpController.sendMsg(sendMsg) == false)
		{
			printf("[sendMsg] Socket Iocp Controller sendMsg() Func Fail\n");
		}
	}
}

void IOCPChatClient::workThreadMain()
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
			_socketIocpController.close();
			break;
		}

		switch (overlappedIOInfo->_operationType)
		{
			case OperationType::SEND:
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