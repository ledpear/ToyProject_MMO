#pragma once
#include <WinSock2.h>
#include <mutex>


constexpr static const UINT8	BACKLOG_SIZE	= 5;
constexpr static const UINT8	CHAT_BUF_SIZE	= 255;
constexpr static const UINT32	SERVER_PORT		= 12000;

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

struct ThreadSafeBuffer
{
	char _buffer[CHAT_BUF_SIZE];
	std::mutex	_mutex;
	OverlappedIOInfo _overlappedIOInfo;
};

class ClientIOController
{
public:
	ClientIOController();
	~ClientIOController();

	inline const bool			isConnected()	const { return _isConnected; }
	inline const UINT32			getIndex()		const { return _index; }
	inline SOCKET				getSocket() { return _clientSock; }
	inline ThreadSafeBuffer&	getRecvBuffer() { return _recvBuffer; }
	inline ThreadSafeBuffer&	getSendBuffer() { return _sendBuffer; }

	void initialize(const UINT32 index, HANDLE iocpHandle);
	bool connectIOCP();

	bool onAccept(SOCKET listenSock);
	bool AcceptCompletion();

	bool bindRecv();
	bool sendMsg(const UINT32 dataSize, const std::string& msgStirng);
	void close(bool isForce = false);

private:
	void resetBufferAndSetOverlappedIOInfo(ThreadSafeBuffer& threadSafeBuffer, OperationType type);


private:
	ThreadSafeBuffer	_sendBuffer;
	ThreadSafeBuffer	_recvBuffer;

	char				_acceptBuffer[CHAT_BUF_SIZE];
	OverlappedIOInfo	_acceptIOInfo;

	HANDLE				_IOCPHandle = INVALID_HANDLE_VALUE;
	SOCKET				_clientSock = INVALID_SOCKET;
	SOCKADDR_IN			_clientAddr;

	INT32				_index = -1;
	bool				_isConnected = false;
};