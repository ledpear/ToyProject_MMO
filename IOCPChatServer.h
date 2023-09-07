#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

class ClientIOController;

class IOCPChatServer
{
public:
	IOCPChatServer();
	~IOCPChatServer();

	//실패 원인 세분화 필요
	bool initialize(const UINT32 maxIOThreadCount);
	bool bindAndListen(const int bindPort);
	void run(const UINT32 maxClientCount);

private:
	ClientIOController* getAvailableClientIOController();

	void createAcceptThread();
	void createWorkThread(const UINT32 maxIOThreadCount);

	void acceptThreadMain();
	void workThreadMain();

	void closeSocket(ClientIOController& clientIOController, bool isForce = false);
	bool sendMasAllClients(const UINT32 dataSize, const std::string& msgStirng);

private:
	std::thread						_acceptThread;
	std::vector<std::thread>		_workThread;
	//채팅을 보낼 때 반복해서 순회해야하기 때문에 vector로 결정
	std::vector<std::unique_ptr<ClientIOController>>	_clientIOControllers;
	std::mutex						_clientIOControllersLock;
	WSADATA							_wsaData;
	SOCKET							_serverSock;
	HANDLE							_iocpHandle			= nullptr;
	UINT32							_maxIOThreadCount	= 0;
	UINT32							_maxClientCount		= 0;
	bool							_wsaStartupResult	= false;
	bool							_isAcceptThreadRun	= false;
	bool							_isWorkThreadRun	= false;
};