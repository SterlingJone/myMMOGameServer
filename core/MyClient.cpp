#include "MyClient.h"

#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
using namespace tcp;


IClient* tcp::NewIClient()
{
	return new MyClient();
}


tcp::MyClient::MyClient()
{
	socketfd = -1;


	m_Notify_Accept = nullptr;
	m_Notify_Secure = nullptr;
	m_Notify_Disconnect = nullptr;
	m_Notify_Command = nullptr;
}

tcp::MyClient::~MyClient()
{
	if (socketfd > 0)
	{
		close(socketfd);
	}
}



void tcp::MyClient::StartClient(int sid, char* ip, int port)
{
	m_data.Init(sid);
	m_data.temp_AutoConnectTime = 0;
	if (ip != NULL) strcpy(m_data.ip, ip);
	if (port > 0)  m_data.port = port;

	//��ʼ��socket
	int err = InitSocket();
	if (err < 0)
	{
		LOGINFO("InitSocket err...%d \n", err);
		return;
	}
	//��ʼ���߳�
	InitThread();
}

bool tcp::MyClient::setNonblockingSocket()
{
	int flags = fcntl(socketfd, F_GETFL);
	if (flags < 0)
	{
		LOGINFO("setNonblockingSocket err...\n");
		return false;
	}
	flags |= O_NONBLOCK;
	if (fcntl(socketfd, F_SETFL, flags) < 0)
	{
		LOGINFO("setNonblockingSocket err...\n");
		return false;
     }
	return true;
}



int tcp::MyClient::InitSocket()
{
	//1�������׽���
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0) return -2;
	//2�����÷�����
	setNonblockingSocket();
	//3�����÷��� ���ջ�����
	int receone = common::ClientXML->recvBytesOne;
	int sendone = common::ClientXML->sendBytesOne;
	setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char*)& receone, sizeof(int));
	setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, (char*)& sendone, sizeof(int));

	return 0;
}
void tcp::MyClient::Connect_Select()
{
	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	fd_set wset;
	FD_ZERO(&wset);
	FD_SET(socketfd, &wset);

	int error = select(socketfd + 1, NULL, &wset, NULL, &tv);
	switch (error)
	{
	case 0:
	case -1:
		this->DisconnectServer(2001, "select timeout");
		break;
	default:
		if (FD_ISSET(socketfd, &wset)) ConnectServer();
		break;
	}



}


bool tcp::MyClient::ConnectServer()
{
	if (m_data.port < 1000) return false;

	m_data.state = common::E_CSS_TryConnect;

	struct sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr(m_data.ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_data.port);

	int value = connect(socketfd, (struct sockaddr*) & addr, sizeof(addr));
	if (value == 0)//���ӳɹ�
	{
		m_data.state = common::E_CSS_Connect;
		if (m_Notify_Accept != NULL) m_Notify_Accept(this, 1);
		return true;
	}
	if (value < 0)
	{
		if (errno == EINTR || errno == EAGAIN)
		{
			return false;
		}
		if (errno == EINPROGRESS)
		{
			Connect_Select();
			return false;
		}
		if (errno == EISCONN)
		{
			m_data.state = common::E_CSS_Connect;
			if (m_Notify_Accept != NULL)
			{
				m_Notify_Accept(this, 2);
				return true;
			}
		}
	}

	return false;
}

void tcp::MyClient::DisconnectServer(int errcode,const char* err)
{
	if (m_data.state == common::E_CSS_Free)
	{
		return;
	}
	if (socketfd == -1)
	{
		return;
	}

	close(socketfd);

	m_data.Reset();
	socketfd = -1;

	InitSocket();
	if (m_Notify_Disconnect != NULL)
	{
		this->m_Notify_Disconnect(this, errcode);
	}
}





void tcp::MyClient::AutoConnect()
{
	if (m_data.port < 1000) return;
	auto c = GetDataPoint();
	int temp = (int)time(NULL) - m_data.temp_AutoConnectTime;
	if (temp >= common::ClientXML->autoConnectTime)
	{
		c->Reset();
		LOGINFO("client--AutoConnect:%d \n",this->socketfd);
		ConnectServer();
		m_data.temp_AutoConnectTime = (int)time(NULL);
		
	}
}
//��������
int tcp::MyClient::Event_Recv()
{
	auto c = this->GetDataPoint();
	memset(c->temp_buf, 0, common::ClientXML->recvBytesOne);
	int recvBytes = recv(socketfd, c->temp_buf, common::ClientXML->recvBytesOne, 0);
	if (recvBytes > 0)
	{
		if (c->recvs.tail == c->recvs.head)
		{
			c->recvs.tail = 0;
			c->recvs.head = 0;
		}
		if (c->recvs.tail + recvBytes >= common::ClientXML->recvBytesMax)
		{
			this->DisconnectServer(3001, "recv err");
			return -1;
		}

		memcpy(&c->recvs.buf[c->recvs.tail], c->temp_buf, recvBytes);
		c->recvs.tail += recvBytes;
		return 0;
	}
	if (recvBytes == 0)
	{
		this->DisconnectServer(3002, "recvBytes=0");
		return -1;
	}
	switch (errno)
	{
	case EINTR:
	case EAGAIN:
		break;
	default:
		this->DisconnectServer(3003, "recv err");
		return -1;
	}
}

int tcp::MyClient::Event_Send()
{
	auto c = this->GetDataPoint();
	if (c->sends.tail <= c->sends.head) return 0;
	if (c->state < common::E_CSS_Connect) return -1;

	int sendlen = c->sends.tail - c->sends.head;

	int sendBytes = send(socketfd, &c->sends.buf[c->sends.head], sendlen, 0);
	if (sendBytes > 0)
	{
		c->sends.head += sendBytes;
		return 0;
	}
	switch (errno)
	{
	case EINTR:
	case EAGAIN:
		break;
	default:
		this->DisconnectServer(4001, "send err");
		return -1;
	}
	return 0;
}



//********************************************************************
//********************************************************************
//********************************************************************
//�߳�
void tcp::MyClient::InitThread()
{
	m_WorkThread.reset(new std::thread(MyClient::RunThread, this));
	m_WorkThread->detach();
}

void tcp::MyClient::RunThread(MyClient* tcp)
{
	LOGINFO("RunThread...\n");
	auto c = tcp->GetDataPoint();
	auto socketfd = tcp->GetSocket();

	while (true)
	{
		//1����������
		if (c->state == common::E_CSS_Free)
		{
			//�Զ����ӷ�����
			tcp->AutoConnect();
			socketfd = tcp->GetSocket();
		}
		else if (c->state == common::E_CSS_TryConnect)
		{
			//���ӷ�����
			tcp->ConnectServer();
			socketfd = tcp->GetSocket();
		}
		if (c->state < common::E_CSS_Free)
		{
			sleep(20);
			continue;
		}

		//2�����ӳɹ� 
		struct timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 1000;

		fd_set  frecv;
		FD_ZERO(&frecv);
		FD_SET(socketfd, &frecv);

		int errcode = select(socketfd + 1, &frecv, NULL, NULL, &tv);
		switch (errcode)
		{
			case -1:
				 {
				    switch (errno)
					{
						case EINTR:
							break;
						default:
							tcp->DisconnectServer(1001, "select = -1");
							break;
					}
				}
				break;
			case 0:
				break;
			default:
				//�����ݵ���
				if (FD_ISSET(socketfd, &frecv))
				{
					//���� ��������
					tcp->Event_Recv();
				}
				break;
		}

		
	}

	LOGINFO("exit RunThread...\n");
}

