#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>

#include "PacketDefine.h"
#include "IocpCommunication.h"

class IOCPChatClient
{
public:	
	IOCPChatClient();
	~IOCPChatClient();

	bool initialize(const UINT32 maxIOThreadCount);
	bool connectServer(const std::string& ipAddress, const int bindPort, std::function<void(bool)> callback);
	void run();

private:
	void workThreadMain();

private:
	IocpSocketHandler				_socketIocpController;
	std::thread						_workThread;
	WSADATA							_wsaData;
	HANDLE							_iocpHandle = nullptr;
	UINT32							_maxIOThreadCount = 0;

	bool							_wsaStartupResult = false;
	bool							_isWorkThreadRun = false;

	//리시브는 쓰레드로 별도로 돌고 샌드는 메인스레드로 돌면서 사용자의 입력을 받는다
};