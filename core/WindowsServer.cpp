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
	{	//1.���������û�
		m_Onlines = new common::HashContainer<S_CONNECT_BASE>(common::ServerXML->appMaxConnect);
		for (int i = 0; i < m_Onlines->Count(); ++i)
		{
			m_Onlines->Value(i)->Init();
		}
		//2.���������û�����
		m_OnlinesIndexs = new common::HashContainer<S_CONNECT_INDEX>(MAX_SOCKETFD_LEN);
		for (int i = 0; i < m_OnlinesIndexs->Count(); ++i)
		{
			m_OnlinesIndexs->Value(i)->Reset();
		}
		//3.��ʼ��socket
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
		//���÷�����ģʽ
		unsigned long ul;
		errcode = ioctlsocket(m_ListenSocket, FIONBIO, &ul);
		if (errcode == SOCKET_ERROR)
		{
			return -4;
		}

		int buffersize = 0;
		setsockopt(m_ListenSocket, SOL_SOCKET, SO_RCVBUF, (const char*)buffersize, sizeof(buffersize));
		setsockopt(m_ListenSocket, SOL_SOCKET, SO_SNDBUF, (const char*)buffersize, sizeof(buffersize));
		//����ɶ˿�
		HANDLE handle = CreateIoCompletionPort((HANDLE)m_ListenSocket, m_Completeport, 0, 0);
		if (handle == NULL)
		{
			return -5;
		}
		//��IP�Ͷ˿�
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

		//1����Ҫ���Ƶ��׽ӿڵľ����
		//2�����еĲ����Ŀ��ƴ��룬��������Ҫ�ٿص�����
		//3���뻺�����ĵ�ַ���������ָ��һ���������׽��ֽ��п��ƣ�������һ��guid��ָ��������ƺ�����guid����
		//4���뻺�����Ĵ�С������Ϊguid�Ĵ�С����sizeof(&guid)����
		//5����������ĵ�ַ�����Ｔ�Ǻ���ָ��ĵ�ַ����
		//6����������Ĵ�С������ָ��Ĵ�С����
		//7���ʵ���ֽ����ĵ�ַ��
		//8WSAOVERLAPPED�ṹ�ĵ�ַ��һ��ΪNULL����
		//9һ��ָ�������������õ�����ָ�루һ��ΪNULL����
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

