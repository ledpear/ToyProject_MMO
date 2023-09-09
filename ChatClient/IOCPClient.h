#pragma once
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>

#include "../Define/PacketDefine.h"

class IOCPClient
{
public:	
	IOCPClient();
	~IOCPClient();

	bool initialize(const UINT32 maxIOThreadCount);
	bool connectServer(const std::string& ipAddress, const int bindPort, std::function<void(bool)> callback);
	void run();

private:
	void workThreadMain();
	bool bindRecv();

private:
	std::vector<std::thread>		_workThread;
	std::function<void(bool)>		_connectCompleteCallBack;
	std::mutex						_clientIOLock;
	std::thread						_runThread;
	WSADATA							_wsaData;
	SOCKET							_serverSock;
	SOCKET							_clientSock;
	HANDLE							_iocpHandle = nullptr;
	UINT32							_maxIOThreadCount = 0;

	OverlappedIOInfo				_connectIOInfo;
	threadSafeBuffer				_recvBuffer;

	bool							_wsaStartupResult = false;
	bool							_isWorkThreadRun = false;

	//리시브는 쓰레드로 별도로 돌고 샌드는 메인스레드로 돌면서 사용자의 입력을 받는다
};