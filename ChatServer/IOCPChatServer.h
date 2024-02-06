#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>

#include <WinSock2.h>

class IocpSocketHandler;
class IocpCommunicationManager;
struct OverlappedIOInfo;

class IOCPChatServer
{
public:
	IOCPChatServer();
	~IOCPChatServer();

	bool initialize(const UINT32 maxIOThreadCount);
	bool bindAndListen(const int bindPort);
	bool run(const UINT32 maxClientCount);
	void shutdown();

private:
	IocpSocketHandler* getAvailableIocpSocketHandler();

	void createWorkThread();
	void workThreadMain();

	void closeSocketIocpControllerAndStartAccept(IocpSocketHandler& iocpSocketHandler, bool isForce = false);
	void sendMsgAllClients(const std::string& msgStirng);

	void closeSocketComplete(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo);
	void acceptComplete(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo);
	void sendComplete(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo);
	void receiveComplete(IocpSocketHandler& iocpSocketHandler, OverlappedIOInfo& overlappedIOInfo);

private:
	std::vector<std::thread>		_workThreads;
	//채팅을 보낼 때 반복해서 순회해야하기 때문에 vector로 결정
	std::unique_ptr<IocpCommunicationManager>		_iocpCommunicationManager;
	std::vector<std::unique_ptr<IocpSocketHandler>>	_iocpSocketHandlers;
	std::unique_ptr<IocpSocketHandler>				_listenSocketHandler;
	std::mutex										_iocpSocketHandlersLock;
	WSADATA							_wsaData;
	UINT32							_maxIOThreadCount	= 0;
	UINT32							_maxClientCount		= 0;
	bool							_wsaStartupResult	= false;
	bool							_isWorkThreadRun	= false;
};