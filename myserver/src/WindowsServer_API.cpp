#include "WindowsServer.h"

namespace tcp
{
	void WindowsServer::Update()
	{
		for (int i = 0; i < m_Onlines->Count(); ++i)
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
		}
	}
	void WindowsServer::UpdateDisconnect(S_CONNECT_BASE* c)
	{
		//1.检查安全关闭
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

		//2.检查安全连接
		temp = (int)time(NULL) - c->temp_ConnectTime;
		if (c->state == common::E_SSS_Connect)
		{
			if (temp > 10)
			{
				ShutDownSocket(c->socketfd, 0, c, 1001);
				return;
			}
		}
		//3.检查心跳连接
		temp = (int)time(NULL) - c->temp_HeartTime;
		if (temp > common::ServerXML->maxHeartTime)
		{
			ShutDownSocket(c->socketfd, 0, c, 1002);
			return;
		}
	}
	S_CONNECT_BASE* WindowsServer::FindNoStateData()
	{
		std::lock_guard<std::mutex> guard(this->m_Mutex_NoState);
		for (int i = 0; i < m_Onlines->Count(); ++i)
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
	S_CONNECT_INDEX* WindowsServer::FindOnlinesIndex(const int socketfd)
	{
		if (socketfd < 0 || socketfd >= MAX_SOCKETFD_LEN)
		{
			return nullptr;
		}
		auto index = m_OnlinesIndexs->Value(socketfd);
		return index;
	}
	S_CONNECT_BASE* WindowsServer::FindClient(const int socketfd, bool issecure)
	{	
		auto cindex = FindOnlinesIndex(socketfd);
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
		return nullptr;
	}
	S_CONNECT_BASE* WindowsServer::FindClient(const int index)
	{	
		if (index < 0 || index >= m_Onlines->Count())
		{
			return nullptr;
		}
		auto c = m_Onlines->Value(index);
		return c;
	}
	bool tcp::WindowsServer::IsCloseClient(const int index, int secure)
	{
		if (index < 0 || index >= m_Onlines->Count())
		{
			return false;
		}
		auto c = m_Onlines->Value(index);
		if (c == nullptr)
		{
			return false;
		}
		if (c->state >= secure)
		{
			return false;
		}
		//shutdown
		return true;
	}
	void WindowsServer::ComputeSecureNum(bool isadd)
	{
		std::lock_guard<std::mutex> guard(this->m_Mutex_NoState);
		if (isadd)
		{
			++m_SecurityCount;
		}
		else 
		{
			--m_SecurityCount;
		}
	}
	void WindowsServer::ComputeConnectNum(bool isadd)
	{
		std::lock_guard<std::mutex> guard(this->m_Mutex_NoState);
		if (isadd)
		{
			++m_ConnectCount;
		}
		else
		{
			--m_ConnectCount;
		}
	}
	void WindowsServer::SetNotify_Connect(ISERVER_NOTIFY e)
	{
		m_Notify_Accept = e;
	}
	void WindowsServer::SetNotify_Secure(ISERVER_NOTIFY e)
	{
		m_Notify_Secure = e;
	}
	void WindowsServer::SetNotify_DisConnect(ISERVER_NOTIFY e)
	{
		m_Notify_Disconnect = e;
	}
	void WindowsServer::SetNotify_Command(ISERVER_NOTIFY e)
	{
		m_Notify_Command = e;
	}
}