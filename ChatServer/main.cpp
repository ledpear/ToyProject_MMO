// ChatServer.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#include <string>
#include <iostream>
#include "ServerDefine.h"
#include "IOCPChatServer.h"

int main()
{
    IOCPChatServer chatServer;

	if (chatServer.initialize(9) == false)
	{
		printf_s("IOCP Server Initialize Fail. Turn off Program.\n");
		return 0;
	}
	printf_s("IOCP Server Initialize Complete.\n");


	if (chatServer.bindAndListen(SERVER_PORT) == false)
	{
		printf_s("IOCP Server Bind And Listen Fail. Turn off Program.\n");
		return 0;
	}
	printf_s("IOCP Server Bind And Listen Complete.\n");

	if (chatServer.run(4) == false)
	{
		printf_s("IOCP Server Run Fail. Turn off Program.\n");
		return 0;
	}
	printf_s("IOCP Server Run Start.\n");

	while (true)
	{
		std::string inputCmd;
		std::getline(std::cin, inputCmd);

		if (inputCmd == "quit")
			break;
	}
}