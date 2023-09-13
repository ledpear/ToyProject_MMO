// ChatClient.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include "IOCPClient.h"

int main()
{
	IOCPClient chatClient;
	if( chatClient.initialize(9) == false)
	{
		printf_s("Client Initialize Fail. Turn off Program.\n");
		return 0;
	}
	else
		printf_s("IOCPClient Initialize Complete.\n");

	//IP / Domain인지 선택

	if (chatClient.connectServer("127.0.0.1", SERVER_PORT, [](bool isResult) {}) == false)
	{
		printf_s("Connect Server Fail. Turn off Program.\n");
		return 0;
	}
	else
	{
		printf_s("IOCPClient Connect Server Complete.\n");
		chatClient.run();
	}	
}