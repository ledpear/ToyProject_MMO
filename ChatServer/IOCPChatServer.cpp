#include <WinSock2.h>
#include <Windows.h>
#include <string>

#include "PacketDefine.h"
#include "IOCPChatServer.h"
#include "IocpCommunication.h"

#pragma comment(lib,"ws2_32.lib")

IOCPChatServer::IOCPChatServer()
	: _iocpCommunicationManager(std::make_unique<IocpCommunicationManager>
		(this, &IOCPChatServer::closeSocketComplete, &IOCPChatServer::acceptComplete, &IOCPChatServer::connectComplete, &IOCPChatServer::sendComplete, &IOCPChatServer::receiveComplete))
	, _listenSocketHandler(std::make_unique<IocpSocketHandler>())
{
	_wsaStartupResult = (WSAStartup(MAKEWORD(2, 2), &_wsaData) == 0);
}

IOCPChatServer::~IOCPChatServer()
{
	shutdown();
	if (_wsaStartupResult)
		WSACleanup();
}

bool IOCPChatServer::initialize(const UINT32 maxIOThreadCount)
{
	//�����ڿ��� WSAStartup�� ���������� ����
	if (_wsaStartupResult == false)
		return false;

	// IO Completion Port ����
	if (_iocpCommunicationManager->createIocp(maxIOThreadCount) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	//listen ���� �ʱ�ȭ
	if (_iocpCommunicationManager->initializeAndConnectIocpSocketHandler(*_listenSocketHandler.get()) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	_maxIOThreadCount = maxIOThreadCount;
	return true;
}

bool IOCPChatServer::bindAndListen(const int bindPort)
{

	if (_iocpCommunicationManager->bindAndListen(*_listenSocketHandler.get(), bindPort) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	return true;
}

bool IOCPChatServer::run(const UINT32 maxClientCount)
{
	int	clientAddrSize = sizeof(SOCKADDR_IN);

	_maxClientCount = maxClientCount;

	//Ŭ���̾�Ʈ IO controller ����
	_iocpSocketHandlers.reserve(maxClientCount);
	for (UINT32 index = 0; index < maxClientCount; ++index)
	{
		_iocpSocketHandlers.emplace_back(std::make_unique<IocpSocketHandler>());
		_iocpCommunicationManager->initializeAndConnectIocpSocketHandler(*_iocpSocketHandlers.back().get(), index);
	}

	// �� Ŭ���̾�Ʈ accept
	bool isAcceptResult = true;
	for (std::unique_ptr<IocpSocketHandler>& iocpSocketHandler : _iocpSocketHandlers)
	{
		isAcceptResult |= (_iocpCommunicationManager->acceptSocket(*iocpSocketHandler.get(), *_listenSocketHandler.get()) == IocpErrorCode::NOT_IOCP_ERROR);
		if (isAcceptResult == false)
		{
			printf_s("[Index:%d] acceptAsync Fail.\n", iocpSocketHandler->getIndex());
			return false;
		}
	}

	// work ������ ����
	createWorkThread();
	return true;
}

void IOCPChatServer::shutdown()
{
	//����� ���� ����
	for (std::unique_ptr<IocpSocketHandler>& iocpSocketHandler : _iocpSocketHandlers)
	{
		if (iocpSocketHandler->isSocketConnected() == false)
			continue;

		_iocpCommunicationManager->closeSocket(*iocpSocketHandler.get());
	}

	//������ ����
	_isWorkThreadRun = false;
	for (std::thread& workThread : _workThreads)
		workThread.join();

	//���� ���� ����
	_iocpCommunicationManager->closeSocket(*_listenSocketHandler.get());
}

IocpSocketHandler* IOCPChatServer::getAvailableIocpSocketHandler()
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
		switch (_iocpCommunicationManager->workIocpQueue(INFINITE))
		{
		case IocpErrorCode::IOCP_ERROR_FAIL_COMMUNICATION:
			_isWorkThreadRun = false;
			break;
		default:
			break;
		}
	}
}

void IOCPChatServer::closeSocketIocpControllerAndStartAccept(IocpSocketHandler& iocpSocketHandler, bool isForce)
{
	if (iocpSocketHandler.isSocketConnected() == false)
		return;

	const UINT32 clientIndex = iocpSocketHandler.getIndex();
	_iocpCommunicationManager->closeSocket(iocpSocketHandler, isForce);

	//������ �ִ� Ŭ���̾�Ʈ���� ���� �˸� ������
	const std::string message(std::to_string(clientIndex) + " Index Client Exit");
	printf_s("[Index:%d] Client Exit.\n", clientIndex);
	sendMsgAllClients(message.c_str());

	//������ ���� �ٽ� �񵿱� Accept
	if (_iocpCommunicationManager->initializeAndConnectIocpSocketHandler(iocpSocketHandler, clientIndex) != IocpErrorCode::NOT_IOCP_ERROR)
	{
		printf_s("[Index:%d] initializeAndConnectIocpSocketHandler Fail.\n", clientIndex);
		return;
	}

	if (_iocpCommunicationManager->acceptSocket(iocpSocketHandler, *_listenSocketHandler.get()) != IocpErrorCode::NOT_IOCP_ERROR)
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
		IocpErrorCode result = _iocpCommunicationManager->sendMsgSocket(*iocpSocketHandler.get(), msgStirng);
		if (result == IocpErrorCode::IOCP_ERROR_FAIL_ASYNC_SEND)
		{
			closeSocketIocpControllerAndStartAccept(*iocpSocketHandler.get());
			continue;
		}
	}
}

void IOCPChatServer::closeSocketComplete(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo)
{
	closeSocketIocpControllerAndStartAccept(iocpSocketHandler);
}

void IOCPChatServer::acceptComplete(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo)
{
	const int clientIndex = overlappedIOInfo._index;
	std::string clientIP;
	int port = 0;

	std::unique_ptr<IocpSocketHandler>& acceptIocpSocketHandler = _iocpSocketHandlers[clientIndex];
	_iocpCommunicationManager->acceptComplete(*acceptIocpSocketHandler.get(), &clientIP, &port);
	printf("Accept Completion Client : IP(%s) SOCKET(%d)\n", clientIP.c_str(), port);

	if (_iocpCommunicationManager->receiveSocket(*acceptIocpSocketHandler) != IocpErrorCode::NOT_IOCP_ERROR)
	{
		printf_s("[Index:%d] bindReceive Fail.\n", clientIndex);
		closeSocketIocpControllerAndStartAccept(*acceptIocpSocketHandler);
		return;
	}

	//������ �ִ� Ŭ���̾�Ʈ���� ���� �˸� ������
	const std::string message(std::to_string(clientIndex) + " Index Client Access");
	sendMsgAllClients(message.c_str());
}

void IOCPChatServer::connectComplete(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo)
{
}

void IOCPChatServer::sendComplete(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo)
{
}

void IOCPChatServer::receiveComplete(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo)
{
	const UINT32 clientIndex = iocpSocketHandler.getIndex();
	std::string msgString;
	_iocpCommunicationManager->getReceiveMsg(iocpSocketHandler, msgString);
	msgString = "[index:" + std::to_string(clientIndex) + "] " + msgString;

	printf_s("%s\n", msgString.c_str());
	sendMsgAllClients(msgString);
	if (_iocpCommunicationManager->receiveSocket(iocpSocketHandler) != IocpErrorCode::NOT_IOCP_ERROR)
	{
		//Ŭ���̾�Ʈ ���� ���� ��û
		printf_s("[Index:%d] bindReceive Fail.\n", clientIndex);
		closeSocketIocpControllerAndStartAccept(iocpSocketHandler);
		return;
	}
}
