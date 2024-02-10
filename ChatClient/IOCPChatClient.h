#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <WinSock2.h>

class IocpCommunicationManager;
class IocpSocketHandler;
struct OverlappedIOInfo;

class IOCPChatClient
{
public:	
	IOCPChatClient();
	~IOCPChatClient();

	bool initialize(const UINT32 maxIOThreadCount);
	bool connectServer(const std::string& ipAddress, const int bindPort, std::function<void(bool)> callback);
	void run();

	void closeSocketComplete(IocpSocketHandler& socketIocpController, OverlappedIOInfo& overlappedIOInfo);
	void acceptComplete(IocpSocketHandler& socketIocpController, OverlappedIOInfo& overlappedIOInfo);
	void sendComplete(IocpSocketHandler& socketIocpController, OverlappedIOInfo& overlappedIOInfo);
	void receiveComplete(IocpSocketHandler& socketIocpController, OverlappedIOInfo& overlappedIOInfo);

private:
	void workThreadMain();

private:
	std::unique_ptr<IocpCommunicationManager>	_iocpCommunicationManager;
	std::unique_ptr<IocpSocketHandler>			_iocpSocketHandler;

	std::thread						_workThread;
	WSADATA							_wsaData;
	UINT32							_maxIOThreadCount = 0;

	bool							_wsaStartupResult = false;
	bool							_isWorkThreadRun = false;
};