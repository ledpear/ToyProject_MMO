#include <ws2tcpip.h>
#include <mswsock.h>
#include <WinSock2.h>

#include "IOCPChatClient.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"mswsock.lib")  // ConnectEx()

IOCPChatClient::IOCPChatClient()
	: _iocpCommunicationManager(this, &IOCPChatClient::closeSocketComplete, &IOCPChatClient::acceptComplete, &IOCPChatClient::sendComplete, &IOCPChatClient::receiveComplete)
{
	_wsaStartupResult = (WSAStartup(MAKEWORD(2, 2), &_wsaData) == 0);
}

IOCPChatClient::~IOCPChatClient()
{
	_workThread.join();
	_iocpSocketHandler.close();
	if (_wsaStartupResult)
		WSACleanup();
}

bool IOCPChatClient::initialize(const UINT32 maxIOThreadCount)
{
	//생성자에서 WSAStartup가 실패했으면 리턴
	if (_wsaStartupResult == false)
		return false;

	// IO Completion Port 생성
	if (_iocpCommunicationManager.createIocp(maxIOThreadCount) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	_maxIOThreadCount = maxIOThreadCount;
	if (_iocpSocketHandler.initialize(0) == false)
	{
		printf_s("[initialize] Socket IOCP Controller Initialize Fail\n");
		return false;
	}

	return true;
}

bool IOCPChatClient::connectServer(const std::string& ipAddress, const int bindPort, std::function<void(bool)> callback)
{
	if (_iocpCommunicationManager.connectIocpSocketHandler(_iocpSocketHandler) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	if (_iocpCommunicationManager.connectSocket(_iocpSocketHandler, ipAddress, bindPort) != IocpErrorCode::NOT_IOCP_ERROR)
		return false;

	return true;
}

void IOCPChatClient::run()
{
	{
		//쓰레드 실행
		if (_iocpSocketHandler.bindRecv() == false)
		{
			printf("[run] bindRecv Fail\n");
			_iocpSocketHandler.close();
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
			_iocpSocketHandler.close();
			break;
		}

		if (_iocpSocketHandler.sendMsg(sendMsg) == false)
		{
			printf("[sendMsg] Socket Iocp Controller sendMsg() Func Fail\n");
		}
	}
}

void IOCPChatClient::closeSocketComplete(IocpSocketHandler& socketIocpController, bool isForce)
{
	_iocpSocketHandler.close();
}

void IOCPChatClient::acceptComplete(IocpSocketHandler& socketIocpController, bool isForce)
{
}

void IOCPChatClient::sendComplete(IocpSocketHandler& socketIocpController, bool isForce)
{
}

void IOCPChatClient::receiveComplete(IocpSocketHandler& socketIocpController, bool isForce)
{

	std::string msgString(socketIocpController.getRecvBuffer()._buffer);
	printf("%s\n", msgString.c_str());

	if (_iocpSocketHandler.bindRecv() == false)
	{
		printf("[run] bindRecv Fail\n");
		_iocpSocketHandler.close();
		return;
	}
}

void IOCPChatClient::workThreadMain()
{
	bool				isSuccess = false;
	DWORD				ioSize = 0;
	LPOVERLAPPED		lpOverlapped = nullptr;
	IocpSocketHandler*	socketIocpController = nullptr;	

	while (_isWorkThreadRun)
	{
		if (_iocpCommunicationManager.workIocpQueue(INFINITE) != IocpErrorCode::NOT_IOCP_ERROR)
			break;
	}
}