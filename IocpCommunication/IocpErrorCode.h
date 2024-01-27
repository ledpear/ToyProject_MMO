#pragma once
enum class IocpErrorCode
{
	//Code													//Cause
	NOT_IOCP_ERROR = 0,										//에러가 발생하지 않음

	IOCP_ERROR_FAIL_CREATE_IOCP,							//IOCP 생성 실패
	IOCP_ERROR_FAIL_CREATE_SOCKET,							//소켓 생성 실패
	IOCP_ERROR_FAIL_BIND_SOCKET,							//소켓 BIND 실패
	IOCP_ERROR_FAIL_LISTEN_SOCKET,							//소켓 LISTEN 실패
	IOCP_ERROR_FAIL_CONNECT_SOCKET_TO_IOCP,					//소켓을 IOCP에 연결 실패

	IOCP_ERROR_NOT_CREATE_IOCP,								//IOCP를 생성하지 않고 소켓관련 함수를 호출
};