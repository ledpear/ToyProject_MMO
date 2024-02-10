// ChatClient.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>

#include "IOCPChatClient.h"
#include "PacketDefine.h"

constexpr static const UINT8	CLIENT_WORK_THREAD_COUNT = 1;

int main()
{
	IOCPChatClient chatClient;
	if( chatClient.initialize(CLIENT_WORK_THREAD_COUNT) == false)
	{
		printf_s("Client Initialize Fail. Turn off Program.\n");
		return 0;
	}
	else
		printf_s("IOCPChatClient Initialize Complete.\n");

	bool isConnectMyServer = false;
	std::string ipAddress("127.0.0.1");
	std::string connectMyServer("");
	//while (true)
	//{
	//	std::cout << "connect tjsrb7575.iptime.org? (yes/no) (if no, connect localhost)" << std::endl;
	//	std::cin >> connectMyServer;
	//	if (strcmp("yes", connectMyServer.c_str()) == 0)
	//	{
	//		isConnectMyServer = true;
	//		break;
	//	}
	//	else if (strcmp("no", connectMyServer.c_str()) == 0)
	//		break;
	//}

	if (isConnectMyServer)
	{
		//dns 사용
		hostent* host = gethostbyname("tjsrb75.synology.me");
		ipAddress = inet_ntoa(*reinterpret_cast<in_addr*>(host->h_addr_list[0]));
	}
	
	if (chatClient.connectServer(ipAddress, SERVER_PORT, [](bool isResult) {}) == false)
	{
		printf_s("Connect Server Fail. Turn off Program.\n");
		return 0;
	}
	else
	{
		printf_s("IOCPChatClient Connect Server Complete.\n");
		chatClient.run();
	}	

	return 0;
}