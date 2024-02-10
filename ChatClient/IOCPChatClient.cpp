#include <ws2tcpip.h>
#include <mswsock.h>
#include <WinSock2.h>
#include <memory>

#include "IOCPChatClient.h"

#include "PacketDefine.h"
#include "IocpCommunication.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")  // ConnectEx()

IOCPChatClient::IOCPChatClient()
	: _iocpCommunicationManager(std::make_unique<IocpCommunicationManager>(this, &IOCPChatClient::closeSocketComplete, &IOCPChatClient::acceptComplete, &IOCPChatClient::connectComplete, &IOCPChatClient::sendComplete, &IOCPChatClient::receiveComplete))
	, _iocpSocketHandler(std::make_unique<IocpSocketHandler>())
{
	_wsaStartupResult = (WSAStartup(MAKEWORD(2, 2), &_wsaData) == 0);
}

IOCPChatClient::~IOCPChatClient()
{
	_workThread.join();
	_iocpCommunicationManager->closeSocket(*_iocpSocketHandler.get());
	if (_wsaStartupResult)
		WSACleanup();
}

bool IOCPChatClient::initialize(const UINT32 maxIOThreadCount)
{
	//생성자에서 WSAStartup가 실패했으면 리턴
	if (_wsaStartupResult == false)
		return false;

	// IO Completion Port 생성
	if (_iocpCommunicationManager->createIocp(maxIOThreadCount) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	_maxIOThreadCount = maxIOThreadCount;
	if(_iocpCommunicationManager->initializeAndConnectIocpSocketHandler(*_iocpSocketHandler.get()) != IocpErrorCode::NOT_IOCP_ERROR)
	{
		printf_s("[initialize] Socket IOCP Controller Initialize Fail\n");
		return false;
	}

	return true;
}

bool IOCPChatClient::connectServer(const std::string& ipAddress, const int bindPort, std::function<void(bool)> callback)
{
	if (_iocpCommunicationManager->connectSocketAsync(*_iocpSocketHandler.get(), ipAddress, bindPort) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	return true;
}

void IOCPChatClient::run()
{
	{
		//쓰레드 실행
		if(_iocpCommunicationManager->receiveSocket(*_iocpSocketHandler.get()) != IocpErrorCode::NOT_IOCP_ERROR)
		{
			printf("[run] bindReceive Fail\n");
			_iocpCommunicationManager->closeSocket(*_iocpSocketHandler.get());
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
			_iocpCommunicationManager->closeSocket(*_iocpSocketHandler.get());
			break;
		}

		if(_iocpCommunicationManager->sendMsgSocket(*_iocpSocketHandler.get(), sendMsg) != IocpErrorCode::NOT_IOCP_ERROR)
		{
			printf("[sendMsg] Socket Iocp Controller sendMsg() Func Fail\n");
		}
	}
}

void IOCPChatClient::closeSocketComplete(IocpSocketHandler& socketIocpController, OverlappedIOInfo& overlappedIOInfo)
{
	_iocpCommunicationManager->closeSocket(*_iocpSocketHandler.get());
}

void IOCPChatClient::acceptComplete(IocpSocketHandler& socketIocpController, OverlappedIOInfo& overlappedIOInfo)
{
}

void IOCPChatClient::connectComplete(IocpSocketHandler& socketIocpController, OverlappedIOInfo& overlappedIOInfo)
{
	_iocpCommunicationManager->connectComplete(socketIocpController);
}

void IOCPChatClient::sendComplete(IocpSocketHandler& socketIocpController, OverlappedIOInfo& overlappedIOInfo)
{
}

void IOCPChatClient::receiveComplete(IocpSocketHandler& socketIocpController, OverlappedIOInfo& overlappedIOInfo)
{
	std::string msgString;
	_iocpCommunicationManager->getReceiveMsg(*_iocpSocketHandler.get(), msgString);
	printf("%s\n", msgString.c_str());

	if (_iocpCommunicationManager->receiveSocket(*_iocpSocketHandler.get()) != IocpErrorCode::NOT_IOCP_ERROR)
	{
		printf("[run] bindReceive Fail\n");
		_iocpCommunicationManager->closeSocket(*_iocpSocketHandler.get());
		return;
	}
}

void IOCPChatClient::workThreadMain()
{
	while (_isWorkThreadRun)
	{
		if (_iocpCommunicationManager->workIocpQueue(INFINITE) != IocpErrorCode::NOT_IOCP_ERROR)
			break;
	}
}