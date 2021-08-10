#include "LinuxServer.h"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <map>
#include <cstring>

using namespace tcp;
using namespace common;

namespace tcp
{
	void LinuxServer::Update()
	{
		for (int i = 0; i < m_Onlines->Count(); i++)
		{
			auto c = m_Onlines->Value(i);
			if (c->index == -1)
			{
				continue;
			}
			if (c->state == common::E_SSS_Free)
			{
				continue;
			}
			if (c->state == common::E_SSS_NeedSave)
			{
				continue;
			}

			UpdateDisconnect(c);
			if (c->closeState == common::E_SSC_ShutDown)
			{
				continue;
			}
			//解析指令
			ReadPackage_Head(c);
			//发送数据
			this->Event_Send(c);
			
		}
	}

	void LinuxServer::ReadPackage_Head(S_CONNECT_BASE* c)
	{
		if (!c->recvs.isCompleted)
		{
			return;
		}

		for (int i = 0; i < 1000; i++)
		{
			int len = c->recvs.tail - c->recvs.head;
			if (len < 8)
			{
				break;
			}
			//1、解析头
			char head[2];
			head[0] = c->recvs.buf[c->recvs.head] ^ c->xorCode;
			head[1] = c->recvs.buf[c->recvs.head + 1] ^ c->xorCode;

			if (head[0] != ServerXML->Head[0] || head[1] != ServerXML->Head[1])
			{
				ShutDownSocket(c->socketfd, 0, c, 4001);
				return;
			}

			int32_t length = (*(uint32_t*)(c->recvs.buf + c->recvs.head + 2)) ^ c->xorCode;
			uint16_t cmd = (*(uint16_t*)(c->recvs.buf + c->recvs.head + 6)) ^ c->xorCode;

			//2、长度不够 需要继续等待
			if (c->recvs.tail < c->recvs.head + length) break;

			c->packageLength = length;

			ReadPackage_Command(c, cmd);

			if (c->state < common::E_SSS_Connect)
			{
				LOGINFO("clinet已经reset....\n");
				return;
			}
			//4、增加读取长度
			c->recvs.head += length;

		}

		c->recvs.isCompleted = false;
	}

	void LinuxServer::ReadPackage_Command(S_CONNECT_BASE* c, uint16_t cmd)
	{
		c->temp_HeartTime = (int)time(NULL);

		if (cmd < 65000)
		{
			if (this->m_Notify_Command != NULL) this->m_Notify_Command(this, c, cmd);
			return;
		}
		switch (cmd)
		{
		case CMD_HEART:
			CreatePackage(c->index, CMD_HEART, NULL, 0);
			break;
		case CMD_SECURITY://安全连接
		{
			char  md5[MAX_MD5_LEN];
			char  a[20];
			sprintf(a, "%s_%d", ServerXML->SecureCode, c->xorCode);
			memset(md5, 0, MAX_MD5_LEN);

			if (common::MD5_FunPoint != NULL) common::MD5_FunPoint(md5, (unsigned char*)a, strlen(a));


			S_CMD_SECURE secure;
			memset(&secure, 0, sizeof(S_CMD_SECURE));
			ReadPackage(c->index, &secure, sizeof(S_CMD_SECURE));


			if (secure.appVersion != ServerXML->appVersion)
			{
				S_CMD_RESULT  kind;
				kind.type = 1;
				CreatePackage(c->index, CMD_SECURITY, &kind, sizeof(S_CMD_RESULT));
				return;
			}
			int error = strcasecmp(md5, secure.appMD5);
			if (error != 0)
			{
				S_CMD_RESULT  kind;
				kind.type = 2;
				CreatePackage(c->index, CMD_SECURITY, &kind, sizeof(S_CMD_RESULT));
				return;
			}

			//安全连接
			c->appID = secure.appID;
			c->state = E_SERVER_SOCKET_STATE::E_SSS_Secure;
			S_CMD_RESULT  kind;
			kind.type = 0;
			CreatePackage(c->index, CMD_SECURITY, &kind, sizeof(S_CMD_RESULT));

			this->ComputeSecureNum(true);
			if (m_Notify_Secure != nullptr) this->m_Notify_Secure(this, c, 0);
		}

		break;
		}
	}

	void LinuxServer::UpdateDisconnect(S_CONNECT_BASE* c)
	{
		//1、检查安全关闭
		int temp = 0;
		if (c->closeState == common::E_SSC_ShutDown)
		{
			temp = (int)time(NULL) - c->temp_CloseTime;
			if (c->recvs.isCompleted && c->sends.isCompleted)
			{
				ReleaseSocket(c->socketfd, c, 1001);
			}
			else if (temp > 1)
			{
				ReleaseSocket(c->socketfd, c, 1002);
			}
			return;
		}
		//2、检查安全连接
		temp = (int)time(NULL) - c->temp_ConnectTime;
		if (c->state == common::E_SSS_Connect)
		{
			if (temp > 10)
			{
				ShutDownSocket(c->socketfd, 0, c, 1001);
				return;
			}
		}
		//3、检查心跳连接
		temp = (int)time(NULL) - c->temp_HeartTime;
		if (temp > common::ServerXML->maxHeartTime)
		{
			ShutDownSocket(c->socketfd, 0, c, 1002);
			return;
		}

	}


	bool LinuxServer::IsCloseClient(const int index, int secure)
	{
		if (index < 0 || index >= m_Onlines->Count())
		{
			return false;
		}

		auto c = m_Onlines->Value(index);
		if (c == NULL)
		{
			return false;
		}
		if (c->state >= secure)
		{
			return false;
		}

		ShutDownSocket(c->socketfd, 0, c, 7001);
		return true;
	}

	S_CONNECT_BASE* LinuxServer::FindClient(const int socketfd, bool issecure)
	{
		if (socketfd < 0 || socketfd >= MAX_SOCKETFD_LEN)
		{
			return nullptr;
		}
		auto cindex = m_OnlinesIndexs->Value(socketfd);
		if (cindex == nullptr)
		{
			return nullptr;
		}
		if (cindex->index < 0)
		{
			return nullptr;
		}

		auto c = FindClient(cindex->index);
		if (c == nullptr)
		{
			return nullptr;
		}
		if (issecure)
		{
			if (!c->IsEqual(socketfd))
			{
				return nullptr;
			}
		}

		return c;
	}

	S_CONNECT_BASE* LinuxServer::FindClient(const int index)
	{
		if (index < 0 || index >= m_Onlines->Count())
		{
			return nullptr;
		}
		auto c = m_Onlines->Value(index);
		return c;
	}

	void LinuxServer::CreatePackage(const int index, const uint16_t cmd, void* v, const int len)
	{
		auto c = FindClient(index);
		if (c == nullptr) return;

		if (c->state <= 0 ||
			c->socketfd <= 0 &&
			c->sends.tail + 8 + len > ServerXML->sendBytesMax)
		{
			ShutDownSocket(c->socketfd, 0, c, 2004);
			return;
		}
		//头尾相等
		if (c->sends.head == c->sends.tail)
		{
			c->sends.tail = 0;
			c->sends.head = 0;
		}
		int tail = c->sends.tail;

		//1、设置头
		c->sends.buf[tail + 0] = ServerXML->Head[0] ^ c->xorCode;
		c->sends.buf[tail + 1] = ServerXML->Head[1] ^ c->xorCode;
		//2、设置包长度
		uint32_t lenth = (8 + len) ^ c->xorCode;
		char* p = (char*)& lenth;
		for (int i = 0; i < 4; i++)  c->sends.buf[tail + 2 + i] = p[i];
		p = NULL;

		//3、设置头指令
		uint16_t newcmd = cmd ^ c->xorCode;
		p = (char*)& newcmd;
		for (int i = 0; i < 2; i++) c->sends.buf[tail + 6 + i] = p[i];


		//偏移
		tail += 8;
		//复制消息体
		if (len > 0 && v != nullptr)
		{
			memcpy(&c->sends.buf[tail], v, len);
			tail += len;
		}
		c->sends.tail = tail;
	}

	void LinuxServer::ReadPackage(const int index, void* v, const int len)
	{
		auto c = FindClient(index);
		if (c == nullptr) return;

		uint32_t  temp_head = c->recvs.head + 8;
		uint32_t  temp_tail = c->recvs.head + c->packageLength;

		if (c->index == -1 ||
			c->state == common::E_SSS_Free ||
			c->recvs.buf == nullptr ||
			temp_head + len > common::ServerXML->recvBytesMax ||
			temp_head + len > temp_tail)
		{
			return;
		}


		memcpy(v, &c->recvs.buf[temp_head], len);
	}

//***********************************************************************
//***********************************************************************
//***********************************************************************
//***********************************************************************

	S_CONNECT_BASE* LinuxServer::FindNoStateData()
	{
		std::lock_guard<std::mutex> guard(this->m_Mutex_NoState);

		for (int i = 0; i < m_Onlines->Count(); i++)
		{
			auto c = m_Onlines->Value(i);
			if (c->state == common::E_SSS_Free)
			{
				c->Reset();
				c->index = i;
				c->state = common::E_SSS_Connect;
				return c;
			}

		}
		return nullptr;
	}

	S_CONNECT_INDEX* LinuxServer::FindOnlinesIndex(const int socketfd)
	{
		if (socketfd < 0 || socketfd >= MAX_SOCKETFD_LEN) return nullptr;
		auto index = m_OnlinesIndexs->Value(socketfd);
		return index;
	}
	void LinuxServer::ComputeSecureNum(bool isadd)
	{
		std::lock_guard<std::mutex> guard(this->m_Mutex_SecureCount);

		if (isadd)
			m_SecurityCount++;
		else
			m_SecurityCount--;
	}

	void LinuxServer::ComputeConnectNum(bool isadd)
	{
		std::lock_guard<std::mutex> guard(this->m_Mutex_ConnectCount);

		if (isadd)
			m_ConnectCount++;
		else
			m_ConnectCount--;
	}

	void LinuxServer::SetNotify_Connect(ISERVER_NOTIFY e)
	{
		m_Notify_Accept = e;
	}

	void LinuxServer::SetNotify_Secure(ISERVER_NOTIFY e)
	{
		m_Notify_Secure = e;
	}

	void LinuxServer::SetNotify_DisConnect(ISERVER_NOTIFY e)
	{
		m_Notify_Disconnect= e;
	}

	void LinuxServer::SetNotify_Command(ISERVER_NOTIFY e)
	{
		m_Notify_Command = e;
	}
}