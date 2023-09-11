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


	//bool isConnect = false;
	//bool isCallBack = false;
	//if (chatClient.connectServer("127.0.0.1", SERVER_PORT, [&isConnect, &isCallBack](bool isResult) { isCallBack = true; isConnect = isResult; }) == false)
	//{
	//	printf_s("클라이언트 초기화에 실패했습니다 프로그램을 종료합니다.\n");
	//	return 0;
	//}
	//else
	//{
	//	while (isCallBack == false)
	//	{
	//		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	//	}

	//	chatClient.run();
	//}

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

// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.
