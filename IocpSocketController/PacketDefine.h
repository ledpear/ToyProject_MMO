#pragma once
#include <mutex>
#include <stdint.h>
#include <WinSock2.h>

constexpr static const UINT8	BACKLOG_SIZE = 5;
constexpr static const UINT8	CHAT_BUF_SIZE = 255;
constexpr static const UINT32	SERVER_PORT = 12000;

constexpr enum class OperationType : unsigned __int8
{
	ACCEPT,
	CONNECT,
	SEND,
	RECV,
	COUNT
};

struct OverlappedIOInfo
{
	WSAOVERLAPPED	_overlapped;
	WSABUF			_wsaBuf;
	OperationType	_operationType = OperationType::COUNT;
	UINT32			_index = UINT32_MAX;
};

struct OverlappedIOBuffer
{
	char _buffer[CHAT_BUF_SIZE];
	std::mutex	_mutex;
	OverlappedIOInfo _overlappedIOInfo;
};