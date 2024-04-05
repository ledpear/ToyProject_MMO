#pragma once
enum class IocpErrorCode
{
	//Code													//Cause
	NOT_IOCP_ERROR = 0,										//������ �߻����� ����

	IOCP_NO_TASK = 100,										//IOCP Queue�� ������� �۾��� ����.
	IOCP_DISCONNECT_REQUEST,								//���� ���� ��û

	IOCP_ERROR_FAIL_COMMUNICATION = 1000,					//��� ����
	IOCP_ERROR_FAIL_CREATE_IOCP,							//IOCP ���� ����
	IOCP_ERROR_FAIL_INITIALIZE_SOCKET,						//���� �ʱ�ȭ ����
	IOCP_ERROR_FAIL_CREATE_SOCKET,							//���� ���� ����
	IOCP_ERROR_FAIL_BIND_SOCKET,							//���� BIND ����
	IOCP_ERROR_FAIL_LISTEN_SOCKET,							//���� LISTEN ����
	IOCP_ERROR_FAIL_CONNECT_SOCKET,							//���� Connect ����
	IOCP_ERROR_FAIL_CONNECT_SOCKET_TO_IOCP,					//������ IOCP�� ���� ����
	IOCP_ERROR_FAIL_ASYNC_RECEIVE,							//�񵿱� Receive ����
	IOCP_ERROR_FAIL_ASYNC_CONNECT_SOCKET,					//�񵿱� Connect ����
	IOCP_ERROR_FAIL_ASYNC_ACCEPT,							//�񵿱� Accept ����
	IOCP_ERROR_FAIL_ASYNC_SEND,								//�񵿱� Send ����

	IOCP_ERROR_INVALID_TASK,								//IOCP ť���� ���������� �۾��� �߻�

	IOCP_ERROR_NOT_CREATE_IOCP,								//IOCP�� �������� �ʰ� ���ϰ��� �Լ��� ȣ��
	IOCP_ERROR_SOCKET_NOT_CONNECT_IOCP,						//IOCP�� �������� �ʰ� �񵿱� �Լ��� ȣ��
};