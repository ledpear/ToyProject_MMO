#pragma once
enum class IocpErrorCode
{
	//Code													//Cause
	NOT_IOCP_ERROR = 0,										//에러가 발생하지 않음

	IOCP_NO_TASK = 100,										//IOCP Queue에 대기중인 작업이 없다.

	IOCP_ERROR_FAIL_CREATE_IOCP = 1000,						//IOCP 생성 실패
	IOCP_ERROR_FAIL_CREATE_SOCKET,							//소켓 생성 실패
	IOCP_ERROR_FAIL_BIND_SOCKET,							//소켓 BIND 실패
	IOCP_ERROR_FAIL_LISTEN_SOCKET,							//소켓 LISTEN 실패
	IOCP_ERROR_FAIL_CONNECT_SOCKET,							//소켓 LISTEN 실패
	IOCP_ERROR_FAIL_CONNECT_SOCKET_TO_IOCP,					//소켓을 IOCP에 연결 실패
	IOCP_ERROR_FAIL_ASYNC_RECEIVE,							//비동기 Receive 실패
	IOCP_ERROR_FAIL_ASYNC_ACCEPT,							//비동기 Accept 실패

	IOCP_ERROR_INVALID_TASK,								//IOCP 큐에서 비정상적인 작업이 발생

	IOCP_ERROR_NOT_CREATE_IOCP,								//IOCP를 생성하지 않고 소켓관련 함수를 호출
	IOCP_ERROR_SOCKET_NOT_CONNECT_IOCP,						//IOCP와 연결하지 않고 비동기 함수를 호출
};