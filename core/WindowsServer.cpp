#include "WindowsServer.h"

namespace tcp
{
	IServer* NewIServer()
	{
		return new WindowsServer();
	}
	WindowsServer::WindowsServer()
	{
		m_AcceptEx = NULL;
		m_GetAcceptEx = NULL;
		m_ListenSocket = INVALID_SOCKET;
		m_Completeport = 0;
		m_SecurityCount = 0;
		m_IsRunning = false;
		m_Onlines = NULL;
		m_OnlinesIndexs = NULL;

		m_Notify_Accept = NULL;
		m_Notify_Secure = NULL;
		m_Notify_Disconnect = NULL;
		m_Notify_Command = NULL;
	}
	WindowsServer::~WindowsServer() {}

	void WindowsServer::StartServer()
	{	//1.创建连接用户
		m_Onlines = new common::HashContainer<S_CONNECT_BASE>(common::ServerXML->appMaxConnect);
		for (int i = 0; i < m_Onlines->Count(); ++i)
		{
			m_Onlines->Value(i)->Init();
		}
		//2.创建连接用户索引
		m_OnlinesIndexs = new common::HashContainer<S_CONNECT_INDEX>(MAX_SOCKETFD_LEN);
		for (int i = 0; i < m_OnlinesIndexs->Count(); ++i)
		{
			m_OnlinesIndexs->Value(i)->Reset();
		}
		//3.初始化socket
		int ret = InitSocket();
		if (ret < 0)
		{
			LOGINFO("InitSocket error : %d \n", ret);
			if (m_Completeport != NULL && m_Completeport != INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_Completeport);
				m_Completeport = INVALID_HANDLE_VALUE;
			}
			if (m_ListenSocket != INVALID_SOCKET)
			{
				closesocket(m_ListenSocket);
				m_ListenSocket = INVALID_SOCKET;
			}
			if (ret != -2)
			{
				WSACleanup();
			}
			return;
		}
	}
	int WindowsServer::InitSocket()
	{
		m_Completeport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (m_Completeport == NULL)
		{
			return -1;
		}
		WSADATA wsData;
		int errcode = WSAStartup(MAKEWORD(2, 2), &wsData);
		if (errcode != 0)
		{
			return -2;
		}
		m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (m_ListenSocket == INVALID_SOCKET)
		{
			return -3;
		}
		//设置非阻塞模式
		unsigned long ul;
		errcode = ioctlsocket(m_ListenSocket, FIONBIO, &ul);
		if (errcode == SOCKET_ERROR)
		{
			return -4;
		}

		int buffersize = 0;
		setsockopt(m_ListenSocket, SOL_SOCKET, SO_RCVBUF, (const char*)buffersize, sizeof(buffersize));
		setsockopt(m_ListenSocket, SOL_SOCKET, SO_SNDBUF, (const char*)buffersize, sizeof(buffersize));
		//绑定完成端口
		HANDLE handle = CreateIoCompletionPort((HANDLE)m_ListenSocket, m_Completeport, 0, 0);
		if (handle == NULL)
		{
			return -5;
		}
		//绑定IP和端口
		sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(common::ServerXML->appPort);
		serAddr.sin_addr.S_un.S_addr = INADDR_ANY;

		errcode = ::bind(m_ListenSocket, (struct sockaddr*)&serAddr, sizeof(sockaddr));
		if (errcode == SOCKET_ERROR)
		{
			return -6;
		}
		errcode = ::listen(m_ListenSocket, SOMAXCONN);
		if (errcode == SOCKET_ERROR)
		{
			return -7;
		}

		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		GUID GuidGetAcceptEx = WSAID_GETACCEPTEXSOCKADDRS;
		DWORD dwBytes = 0;

		//1你需要控制的套接口的句柄。
		//2将进行的操作的控制代码，即你所需要操控的类型
		//3输入缓冲区的地址（如果你想指定一个函数对套接字进行控制，可以是一个guid，指定这个控制函数的guid）。
		//4输入缓冲区的大小（这里为guid的大小，即sizeof(&guid)）。
		//5输出缓冲区的地址（这里即是函数指针的地址）。
		//6输出缓冲区的大小（函数指针的大小）。
		//7输出实际字节数的地址。
		//8WSAOVERLAPPED结构的地址（一般为NULL）。
		//9一个指向操作结束后调用的例程指针（一般为NULL）。
		if (m_AcceptEx == NULL)
		{
			errcode = WSAIoctl(m_ListenSocket,
							SIO_GET_EXTENSION_FUNCTION_POINTER,
							&GuidAcceptEx,
							sizeof(GuidAcceptEx),
							&m_AcceptEx,
							sizeof(m_AcceptEx),
							&dwBytes,
							NULL,
							NULL);
		}

		if (m_AcceptEx == NULL || errcode == SOCKET_ERROR)
		{
			return -8;
		}

		if (m_GetAcceptEx == NULL)
		{
			errcode = WSAIoctl(m_ListenSocket,
								SIO_GET_EXTENSION_FUNCTION_POINTER,
								&GuidGetAcceptEx,
								sizeof(GuidGetAcceptEx),
								&m_GetAcceptEx,
								sizeof(m_GetAcceptEx),
								&dwBytes,
								NULL,
								NULL);
		}

		if (m_GetAcceptEx == NULL || errcode == SOCKET_ERROR)
		{
			return -9;
		}
		return 0;
	}
	void WindowsServer::ShutDownSocket(SOCKET s, const int32_t mode, S_CONNECT_BASE* c, int kind)
	{
		if (c != nullptr)
		{
			if (c->state == common::E_SSS_Free)
			{
				return;
			}
			if (c->closeState == common::E_SSC_ShutDown)
			{
				return;
			}
			c->temp_ShutDown = kind;
			c->temp_CloseTime = (int)time(NULL);
			c->closeState = common::E_SSC_ShutDown;
			shutdown(s, SD_BOTH);

			CancelIoEx((HANDLE)s, nullptr);
			return;
		}

		auto c2 = FindClient(s, true);
		if (c2 == nullptr)
		{
			return;
		}
		if (c2->state == common::E_SSS_Free)
		{
			return;
		}
		if (c2->closeState == common::E_SSC_ShutDown)
		{
			return;
		}
		switch (mode)
		{
		case common::E_CT_Recv:
			c2->recvs.isCompleted = true;
			break;
		case common::E_CT_Send:
			c2->sends.isCompleted = true;
			break;
		}

		c2->temp_ShutDown = kind;
		c2->temp_CloseTime = (int)time(NULL);
		c2->closeState = common::E_SSC_ShutDown;
		shutdown(s, SD_BOTH);
	}
	int32_t WindowsServer::ReleaseSocket(SOCKET socketfd, S_CONNECT_BASE* c, int kind)
	{
		if (socketfd == SOCKET_ERROR || socketfd == INVALID_SOCKET)
		{
			return -1;
		}
		if (c != nullptr)
		{
			if (c->state == common::E_SSS_Free)
			{
				return 0;
			}
			if (c->closeState >= common::E_SSS_Secure)
			{
				this->ComputeSecureNum(false);
			}
		}
		this->ComputeConnectNum(false);
		shutdown(socketfd, SD_BOTH);
		if (socketfd != INVALID_SOCKET)
		{
			closesocket(socketfd);
			socketfd = INVALID_SOCKET;
		}
		return 0;
	}
}

