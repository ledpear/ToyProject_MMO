#include <WinSock2.h>
#include <Windows.h>
#include <string>

#include "IOCPChatServer.h"
#include "../Common/PacketDefine.h"
#include "../Common/SocketIocpController.h"

#pragma comment(lib,"ws2_32.lib")

IOCPChatServer::IOCPChatServer()
{
	_wsaStartupResult = (WSAStartup(MAKEWORD(2, 2), &_wsaData) == 0);
}

IOCPChatServer::~IOCPChatServer()
{
	shutdown();
	if(_wsaStartupResult)
		WSACleanup();
}

bool IOCPChatServer::initialize(const UINT32 maxIOThreadCount)
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
	_serverSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, NULL, WSA_FLAG_OVERLAPPED);
	if (_serverSock == INVALID_SOCKET)
		return false;

	return true;
}

bool IOCPChatServer::bindAndListen(const int bindPort)
{
	//소켓 주소 정보
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(bindPort);
	serverAddr.sin_addr.s_addr = htonl(ADDR_ANY);

	//bind
	if (bind(_serverSock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		closesocket(_serverSock);
		return false;
	}

	//listen
	if (listen(_serverSock, static_cast<int>(BACKLOG_SIZE)) == SOCKET_ERROR)
	{
		closesocket(_serverSock);
		return false;
	}

	//Server Socket IOCP Connect
	SocketIocpController* socketIocpController = nullptr;
	HANDLE resultHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(_serverSock), _iocpHandle, (UINT32)0, 0);
	if (resultHandle == INVALID_HANDLE_VALUE)
	{
		closesocket(_serverSock);
		return false;
	}

	return true;
}

bool IOCPChatServer::run(const UINT32 maxClientCount)
{
	int	clientAddrSize = sizeof(SOCKADDR_IN);

	_maxClientCount = maxClientCount;

	//클라이언트 IO controller 생성
	_socketIocpControllers.reserve(maxClientCount);
	for (UINT32 index = 0; index < maxClientCount; ++index)
	{
		_socketIocpControllers.emplace_back(new SocketIocpController);
		_socketIocpControllers.back()->initialize(index, _iocpHandle);
	}

	// 각 클라이언트 accept
	bool isAcceptResult = true;
	for (std::unique_ptr<SocketIocpController>& socketIocpController : _socketIocpControllers)
	{
		isAcceptResult = socketIocpController->acceptAsync(_serverSock);
		if (isAcceptResult == false)
		{
			printf_s("[Index:%d] acceptAsync Fail.\n", socketIocpController->getIndex());
			return false;
		}
	}

	// work 쓰레드 생성
	createWorkThread();	
	return true;
}

void IOCPChatServer::shutdown()
{
	//연결된 소켓 종료
	for (std::unique_ptr<SocketIocpController>& socketIocpController : _socketIocpControllers)
	{
		if (socketIocpController->isConnected() == false)
			continue;

		closeSocketIocpControllerAndStartAccept(*socketIocpController.get());
	}

	//스레드 종료
	_isWorkThreadRun = false;
	for (std::thread& workThread : _workThreads)
		workThread.join();

	closesocket(_serverSock);
}

SocketIocpController* IOCPChatServer::getAvailableSocketIocpController()
{
	const std::lock_guard<std::mutex> lock(_socketIocpControllersLock);
	for (std::unique_ptr<SocketIocpController>& socketIocpController : _socketIocpControllers)
	{
		if (socketIocpController->isConnected() == false)
			return socketIocpController.get();
	}

	return nullptr;
}

void IOCPChatServer::createWorkThread()
{
	_workThreads.clear();
	_workThreads.reserve(_maxIOThreadCount);

	_isWorkThreadRun = true;
	for (UINT32 i = 0; i < _maxIOThreadCount; ++i)
		_workThreads.emplace_back([this]() { workThreadMain(); });
}

void IOCPChatServer::workThreadMain()
{
	bool					isSuccess = false;
	DWORD					ioSize = 0;
	LPOVERLAPPED			lpOverlapped = nullptr;
	SocketIocpController*	socketIocpController = nullptr;

	while (_isWorkThreadRun)
	{
		isSuccess = GetQueuedCompletionStatus(_iocpHandle, &ioSize, reinterpret_cast<PULONG_PTR>(&socketIocpController), &lpOverlapped, INFINITE);

		if ((isSuccess == true) && (ioSize == 0) && (lpOverlapped == nullptr))
		{
			_isWorkThreadRun = false;
			continue;
		}

		if (lpOverlapped == nullptr)
			continue;

		OverlappedIOInfo* overlappedIOInfo = reinterpret_cast<OverlappedIOInfo*>(lpOverlapped);
		if ((isSuccess == false) || ((overlappedIOInfo->_operationType != OperationType::ACCEPT) && (ioSize == 0)))
		{
			//클라이언트 연결 해제 요청
			if(socketIocpController != nullptr)
				closeSocketIocpControllerAndStartAccept(*socketIocpController);

			continue;
		}

		switch (overlappedIOInfo->_operationType)
		{
			case OperationType::ACCEPT:
			{
				const UINT32 clientIndex = overlappedIOInfo->_index;
				if (clientIndex == UINT32_MAX || clientIndex >= _socketIocpControllers.size())
				{
					printf_s("클라이언트 IO 컨트롤러의 인덱스가 비정상입니다[%d]\n", clientIndex);
					continue;
				}

				SocketIocpController* accpetSocketIocpController = _socketIocpControllers[clientIndex].get();
				if ( accpetSocketIocpController->acceptCompletion() == false)
				{
					printf_s("[Index:%d] acceptCompletion Fail.\n", clientIndex);
					closeSocketIocpControllerAndStartAccept(*accpetSocketIocpController);
					continue;
				}

				if (accpetSocketIocpController->bindRecv() == false)
				{
					printf_s("[Index:%d] bindRecv Fail.\n", clientIndex);
					closeSocketIocpControllerAndStartAccept(*accpetSocketIocpController);
					continue;
				}

				//접속해 있는 클라이언트에게 접속 알림 보내기
				const std::string message(std::to_string(clientIndex) + " Index Client Access");
				sendMsgAllClients(message.c_str());
			}
			break;
			case OperationType::SEND:
			{
				//send  완료
			}
			break;
			case OperationType::RECV:
			{
				//recv 요청
				const UINT32 clientIndex = overlappedIOInfo->_index;
				std::string msgString(socketIocpController->getRecvBuffer()._buffer);
				msgString = "[index:" + std::to_string(clientIndex) + "] " + msgString;

				printf_s("%s\n", msgString.c_str());
				sendMsgAllClients(msgString);
				if (socketIocpController->bindRecv() == false)
				{
					//클라이언트 연결 해제 요청
					printf_s("[Index:%d] bindRecv Fail.\n", clientIndex);
					closeSocketIocpControllerAndStartAccept(*socketIocpController);
					continue;
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

void IOCPChatServer::closeSocketIocpControllerAndStartAccept(SocketIocpController& socketIocpController, bool isForce)
{
	if (socketIocpController.isConnected() == false)
		return;

	const UINT32 clientIndex = socketIocpController.getIndex();
	socketIocpController.close(isForce);

	//접속해 있는 클라이언트에게 종료 알림 보내기
	const std::string message(std::to_string(clientIndex) + " Index Client Exit");
	printf_s("[Index:%d] Client Exit.\n", clientIndex);
	sendMsgAllClients(message.c_str());

	//종료한 소켓 다시 비동기 Accept
	if(socketIocpController.initialize(clientIndex, _iocpHandle) == false)
	{
		printf_s("[Index:%d] initialize Fail.\n", clientIndex);
		return;
	}

	if (socketIocpController.acceptAsync(_serverSock) == false)
	{
		printf_s("[Index:%d] acceptAsync Fail.\n", clientIndex);
		return;
	}
}

void IOCPChatServer::sendMsgAllClients(const std::string& msgStirng)
{
	const std::lock_guard<std::mutex> lock(_socketIocpControllersLock);
	for (std::unique_ptr<SocketIocpController>& socketIocpController : _socketIocpControllers)
	{
		if (socketIocpController->isConnected() == false)
			continue;

		if (socketIocpController->sendMsg(msgStirng) == false)
		{
			closeSocketIocpControllerAndStartAccept(*socketIocpController.get());
			continue;
		}
	}
}
