#include "MyClient.h"


#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>


using namespace tcp;



void tcp::MyClient::Update()
{
	if (socketfd < 0) return;
	auto c = GetDataPoint();
	if (c->state < common::E_CSS_Connect) return;

	//发送心跳包
	SendHeart();
	//解析指令
	ReadPackage_Head();
	//发送指令
	Event_Send();
}

void tcp::MyClient::SendHeart()
{
	auto c = GetDataPoint();
	if (c->state < common::E_CSS_Secure) return;
	int temp = (int)time(NULL) - c->temp_HeartTime;
	if (temp >= common::ClientXML->maxHeartTime)
	{
		c->temp_HeartTime = (int)time(NULL);
		CreatePackage(CMD_HEART, NULL, 0);
	}
}


void tcp::MyClient::ReadPackage_Head()
{
	auto c = GetDataPoint();

	for (int i = 0; i < 1000; i++)
	{
		int len = c->recvs.tail - c->recvs.head;
		if (len < 8) break;
		char head[2];
		head[0] = c->recvs.buf[c->recvs.head] ^ c->xorCode;
		head[1] = c->recvs.buf[c->recvs.head + 1] ^ c->xorCode;
		if (head[0] != common::ClientXML->Head[0] || head[1] != common::ClientXML->Head[1])
		{
			DisconnectServer(6001, "head");
			return;
		}

		uint32_t  length = (*(uint32_t*)(c->recvs.buf + c->recvs.head + 2)) ^ c->xorCode;
		uint16_t  cmd = (*(uint16_t*)(c->recvs.buf + c->recvs.head + 6)) ^ c->xorCode;

		if (c->recvs.tail < c->recvs.head + length) break;
		c->packageLength = length;

		ReadPackage_Command(cmd);

		if (c->state < common::E_CSS_Connect)
		{
			return;
		}

		//偏移length
		c->recvs.head += length;
	}
}

void tcp::MyClient::ReadPackage_Command(uint16_t cmd)
{
	if (cmd < 65000)
	{
		if (m_Notify_Command != NULL) this->m_Notify_Command(this, cmd);
		return;
	}

	switch (cmd)
	{
	case CMD_XOR:
		{
			auto c = GetDataPoint();
			S_CMD_XOR code;
			ReadPackage(&code, sizeof(S_CMD_XOR));
			code.xorCode = code.xorCode ^ c->xorCode;
			c->xorCode = code.xorCode;

			S_CMD_SECURE secure;
			secure.appID      = common::ServerXML->appID;
			secure.appVersion = common::ServerXML->appVersion;

			char a[20];
			sprintf(a, "%s_%d", common::ClientXML->SecureCode, c->xorCode);
			memset(secure.appMD5, 0, MAX_MD5_LEN);
			if (common::MD5_FunPoint != NULL)
			{
				common::MD5_FunPoint(secure.appMD5, (unsigned char*)a, strlen(a));
			}
			//发送MD5码安全验证
			CreatePackage(CMD_SECURITY, &secure, sizeof(S_CMD_SECURE));
		}
		break;
	case CMD_SECURITY:
		{
			auto c = GetDataPoint();
			S_CMD_RESULT kind;
			ReadPackage(&kind, sizeof(S_CMD_RESULT));

			//1 版本错误 2 MD5错误
			if (kind.type > 0)
			{
				LOGINFO("secure is error...%d \n", kind.type);
				break;
			}

			c->state = common::E_CSS_Secure;
			if (m_Notify_Secure != NULL) this->m_Notify_Secure(this, 0);
		}
		break;
	}
}


void tcp::MyClient::CreatePackage(const uint16_t cmd, void* v, const int len)
{
	auto c = GetDataPoint();
	if (c->state < common::E_CSS_Connect ||
		socketfd <= 0 ||
		c->sends.tail + 8 + len >= common::ClientXML->recvBytesMax)
	{
		DisconnectServer(5001, "createpackage");
		return;
	}
	if (c->sends.tail == c->sends.head)
	{
		c->sends.tail = 0;
		c->sends.head = 0;
	}
	int tail = c->sends.tail;
	//1、设置头
	c->sends.buf[tail + 0] = common::ClientXML->Head[0] ^ c->xorCode;
	c->sends.buf[tail + 1] = common::ClientXML->Head[1] ^ c->xorCode;
	//2、设置数据包长度
	uint32_t length = (8 + len) ^ c->xorCode;
	char* p = (char*)& length;
	for (int i = 0; i < 4; i++)  c->sends.buf[tail + 2 + i] = p[i];
	p = NULL;
	//3、设置头指令
	uint16_t newcmd = cmd ^ c->xorCode;
	p = (char*)& newcmd;
	for (int i = 0; i < 2; i++)  c->sends.buf[tail + 6 + i] = p[i];
	tail += 8;

	if (len > 0 && v != NULL)
	{
		memcpy(&c->sends.buf[tail], v, len);
		tail += len;
	}


	c->sends.tail = tail;

}

void tcp::MyClient::ReadPackage(void* v, const int len)
{
	auto c = GetDataPoint();
	if (c == nullptr) return;

	uint32_t temp_head = c->recvs.head + 8;
	uint32_t temp_tail = c->recvs.head + c->packageLength;

	if (c->state <= 0 ||
		c->recvs.buf == nullptr ||
		temp_head + len >= common::ClientXML->recvBytesMax ||
		temp_head + len > temp_tail)
	{
		return;
	}

	memcpy(v, &c->recvs.buf[temp_head], len);
}





void tcp::MyClient::SetNotify_Connect(ICLIENT_NOTIFY e)
{
	m_Notify_Accept = e;
}

void tcp::MyClient::SetNotify_Secure(ICLIENT_NOTIFY e)
{
	m_Notify_Secure = e;
}

void tcp::MyClient::SetNotify_DisConnect(ICLIENT_NOTIFY e)
{
	m_Notify_Disconnect = e;
}

void tcp::MyClient::SetNotify_Command(ICLIENT_NOTIFY e)
{
	m_Notify_Command = e;
}


