#include <WinSock2.h>
#include <Windows.h>

#include "ServerDefine.h"
#include "IOCPChatServer.h"
#include "ClientIOController.h"

#pragma comment(lib,"ws2_32.lib")

IOCPChatServer::IOCPChatServer()
{
	_wsaStartupResult = (WSAStartup(MAKEWORD(2, 2), &_wsaData) == 0);
}

IOCPChatServer::~IOCPChatServer()
{
	closesocket(_serverSock);
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
	_serverSock = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (_serverSock == INVALID_SOCKET)
		return false;

	return true;
}

bool IOCPChatServer::bindAndListen(const int bindPort)
{
	//소켓 주소 정보
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(bindPort);
	serverAddr.sin_addr.S_un.S_addr = htonl(ADDR_ANY);

	if (bind(_serverSock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		closesocket(_serverSock);
		return false;
	}

	if (listen(_serverSock, static_cast<int>(BACKLOG_SIZE)) == SOCKET_ERROR)
	{
		closesocket(_serverSock);
		return false;
	}

	return true;
}

void IOCPChatServer::run(const UINT32 maxClientCount)
{
	int			clientAddrSize = sizeof(SOCKADDR_IN);

	_maxClientCount = maxClientCount;

	//클라이언트 IO controller 생성
	_clientIOControllers.reserve(maxClientCount);
	for (UINT32 index = 0; index < maxClientCount; ++index)
		_clientIOControllers.emplace_back(new ClientIOController(index));

	// accept 쓰레드 생성
	createAcceptThread();

	// work 쓰레드 생성
	createWorkThread(maxClientCount);	
}

ClientIOController* IOCPChatServer::getAvailableClientIOController()
{
	const std::lock_guard<std::mutex> lock(_clientIOControllersLock);
	for (std::unique_ptr<ClientIOController>& clientIOController : _clientIOControllers)
	{
		if (clientIOController->isConnected() == false)
			return clientIOController.get();
	}

	return nullptr;
}

void IOCPChatServer::createAcceptThread()
{
	_isAcceptThreadRun = true;
	_acceptThread = std::thread([this](){ acceptThreadMain(); });
	printf_s("Accept Thread 생성\n");
}

void IOCPChatServer::createWorkThread(const UINT32 maxIOThreadCount)
{
	const int threadCount = (maxIOThreadCount * 2) + 1;

	_workThread.clear();
	_workThread.reserve(threadCount);

	_isWorkThreadRun = true;
	for (int i = 0; i < threadCount; ++i)
		_workThread.emplace_back([this]() { workThreadMain(); });
}

void IOCPChatServer::acceptThreadMain()
{
	while (_isAcceptThreadRun)
	{
		ClientIOController* clientIOController = getAvailableClientIOController();
		if (clientIOController == nullptr)
		{
			printf_s("클라이언트가 더 이상 접속할 수 없습니다.\n");
			continue;
		}
		else
			clientIOController->onAccept(_serverSock);

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

void IOCPChatServer::workThreadMain()
{
	bool				isSuccess = false;
	DWORD				ioSize = 0;
	LPOVERLAPPED		lpOverlapped = nullptr;
	ClientIOController*	clientIOController = nullptr;

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
		if ((isSuccess == false) || ((overlappedIOInfo->_operationType != OperationType::ACCEPT) && (ioSize == 0)))
		{
			//클라이언트 연결 해제 요청
			closeSocket(*clientIOController);
			continue;
		}

		//Accept
		switch (overlappedIOInfo->_operationType)
		{
			case OperationType::ACCEPT:
			{
				if (clientIOController->AcceptCompletion() == false)
					closeSocket(*clientIOController);
			}
			break;
			case OperationType::SEND:
			{
				//send  완료
				clientIOController->sendCompletion(overlappedIOInfo);
			}
			break;
			case OperationType::RECV:
			{
				//recv 요청
				std::string msgString(clientIOController->getSendBuffer()._buffer);
				sendMasAllClients(ioSize, msgString);
				printf_s("[index:%d] %s\n", clientIOController->getIndex(), msgString.c_str());
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

void IOCPChatServer::closeSocket(ClientIOController& clientIOController, bool isForce)
{

}

bool IOCPChatServer::sendMasAllClients(const UINT32 dataSize, const std::string& msgStirng)
{
	bool isResult = true;
	const std::lock_guard<std::mutex> lock(_clientIOControllersLock);
	for (std::unique_ptr<ClientIOController>& clientIOController : _clientIOControllers)
	{
		if (clientIOController->isConnected())
			isResult |= clientIOController->sendMsg(dataSize, msgStirng);
	}

	return isResult;
}
