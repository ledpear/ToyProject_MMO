#include <WinSock2.h>
#include <Windows.h>
#include <string>

#include "IOCPChatServer.h"
#include "IocpCommunication.h"

#pragma comment(lib,"ws2_32.lib")

IOCPChatServer::IOCPChatServer()
	: _iocpCommunicationManager(std::make_unique<IocpCommunicationManager>(this, &IOCPChatServer::closeSocketComplete, &IOCPChatServer::acceptComplete, &IOCPChatServer::sendComplete, &IOCPChatServer::receiveComplete) )
	, _listenSocketHandler(std::make_unique<IocpSocketHandler>())
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
	if (_iocpCommunicationManager->createIocp(maxIOThreadCount) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	_maxIOThreadCount = maxIOThreadCount;

	//accept 소켓 초기화
	if (_listenSocketHandler->initialize() != ERROR_SUCCESS)
		return false;

	return true;
}

bool IOCPChatServer::bindAndListen(const int bindPort)
{
	if (_iocpCommunicationManager->connectIocpSocketHandler(*_listenSocketHandler.get()) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	if (_iocpCommunicationManager->bindAndListen(*_listenSocketHandler.get(), bindPort) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	return true;
}

bool IOCPChatServer::run(const UINT32 maxClientCount)
{
	int	clientAddrSize = sizeof(SOCKADDR_IN);

	_maxClientCount = maxClientCount;

	//클라이언트 IO controller 생성
	_iocpSocketHandlers.reserve(maxClientCount);
	for (UINT32 index = 0; index < maxClientCount; ++index)
	{
		_iocpSocketHandlers.emplace_back(std::make_unique<IocpSocketHandler>());
		_iocpSocketHandlers.back()->initialize(index);
	}

	// 각 클라이언트 accept
	bool isAcceptResult = true;
	for (std::unique_ptr<IocpSocketHandler>& iocpSocketHandler : _iocpSocketHandlers)
	{
		isAcceptResult = iocpSocketHandler->acceptAsync(_listenSocketHandler->getSocket());
		if (isAcceptResult == false)
		{
			printf_s("[Index:%d] acceptAsync Fail.\n", iocpSocketHandler->getIndex());
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
	for (std::unique_ptr<IocpSocketHandler>& iocpSocketHandler : _iocpSocketHandlers)
	{
		if (iocpSocketHandler->isSocketConnected() == false)
			continue;

		closeSocketIocpControllerAndStartAccept(iocpSocketHandler.get());
	}

	//스레드 종료
	_isWorkThreadRun = false;
	for (std::thread& workThread : _workThreads)
		workThread.join();

	//리슨 소켓 종료
	closesocket(_listenSocketHandler->getSocket());
}

IocpSocketHandler* IOCPChatServer::getAvailableSocketIocpController()
{
	const std::lock_guard<std::mutex> lock(_iocpSocketHandlersLock);
	for (std::unique_ptr<IocpSocketHandler>& iocpSocketHandler : _iocpSocketHandlers)
	{
		if (iocpSocketHandler->isSocketConnected() == false)
			return iocpSocketHandler.get();
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
	while (_isWorkThreadRun)
	{
		if (_iocpCommunicationManager->workIocpQueue(INFINITE) != IocpErrorCode::NOT_IOCP_ERROR)
			break;
	}
}

void IOCPChatServer::closeSocketIocpControllerAndStartAccept(IocpSocketHandler* iocpSocketHandler, bool isForce)
{
	if (iocpSocketHandler->isSocketConnected() == false)
		return;

	const UINT32 clientIndex = iocpSocketHandler->getIndex();
	iocpSocketHandler->close(isForce);

	//접속해 있는 클라이언트에게 종료 알림 보내기
	const std::string message(std::to_string(clientIndex) + " Index Client Exit");
	printf_s("[Index:%d] Client Exit.\n", clientIndex);
	sendMsgAllClients(message.c_str());

	//종료한 소켓 다시 비동기 Accept
	if(iocpSocketHandler->initialize(clientIndex) == false)
	{
		printf_s("[Index:%d] initialize Fail.\n", clientIndex);
		return;
	}

	if (_iocpCommunicationManager->acceptSocket(*iocpSocketHandler, *_listenSocketHandler.get()) != IocpErrorCode::NOT_IOCP_ERROR)
	{
		printf_s("[Index:%d] acceptAsync Fail.\n", clientIndex);
		return;
	}
}

void IOCPChatServer::sendMsgAllClients(const std::string& msgStirng)
{
	const std::lock_guard<std::mutex> lock(_iocpSocketHandlersLock);
	for (std::unique_ptr<IocpSocketHandler>& iocpSocketHandler : _iocpSocketHandlers)
	{
		if (iocpSocketHandler->isSocketConnected() == false)
			continue;

		if (iocpSocketHandler->sendMsg(msgStirng) == false)
		{
			closeSocketIocpControllerAndStartAccept(iocpSocketHandler.get());
			continue;
		}
	}
}

void IOCPChatServer::closeSocketComplete(IocpSocketHandler* iocpSocketHandler, bool isForce)
{
}

void IOCPChatServer::acceptComplete(IocpSocketHandler* iocpSocketHandler, bool isForce)
{
	const int clientIndex = iocpSocketHandler->getIndex();
	std::string clientIP;
	int port = 0;

	iocpSocketHandler->getAcceptAddressInfo(clientIP, port);
	printf("Accept Completion Client : IP(%s) SOCKET(%d)\n", clientIP.c_str(), port);
	
	if (_iocpCommunicationManager->receiveSocket(*iocpSocketHandler) != IocpErrorCode::NOT_IOCP_ERROR)
	{
		printf_s("[Index:%d] bindReceive Fail.\n", clientIndex);
		closeSocketIocpControllerAndStartAccept(iocpSocketHandler);
		return;
	}

	//접속해 있는 클라이언트에게 접속 알림 보내기
	const std::string message(std::to_string(clientIndex) + " Index Client Access");
	sendMsgAllClients(message.c_str());
}

void IOCPChatServer::sendComplete(IocpSocketHandler* iocpSocketHandler, bool isForce)
{
}

void IOCPChatServer::receiveComplete(IocpSocketHandler* iocpSocketHandler, bool isForce)
{
	const UINT32 clientIndex = iocpSocketHandler->getIndex();
	std::string msgString(iocpSocketHandler->getReceiveBuffer()._buffer);
	msgString = "[index:" + std::to_string(clientIndex) + "] " + msgString;

	printf_s("%s\n", msgString.c_str());
	sendMsgAllClients(msgString);
	if(_iocpCommunicationManager->receiveSocket(*iocpSocketHandler) != IocpErrorCode::NOT_IOCP_ERROR)
	{
		//클라이언트 연결 해제 요청
		printf_s("[Index:%d] bindReceive Fail.\n", clientIndex);
		closeSocketIocpControllerAndStartAccept(iocpSocketHandler);
		return;
	}
}
