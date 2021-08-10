#include "WindowsServer.h"
#include <concurrent_queue.h>

namespace recycle
{
	Concurrency::concurrent_queue<PostAcceptRecycle*> __accepts;
	Concurrency::concurrent_queue<PostRecvRecycle*> __recvs;
	Concurrency::concurrent_queue<PostSendRecycle *> __sends;

	RecycleBase::RecycleBase()
	{
		m_PostSocket = INVALID_SOCKET;
		m_PostType = 0;
		memset(&m_OverLapped, 0, sizeof(m_OverLapped));
	}

	RecycleBase::~RecycleBase() {}

	PostAcceptRecycle::PostAcceptRecycle(int type)
	{
		m_PostType = type;
		m_PostSocket = INVALID_SOCKET;
		memset(&m_OverLapped, 0, sizeof(m_OverLapped));
	}

	PostAcceptRecycle::~PostAcceptRecycle()
	{
		m_PostType = common::E_CT_Accept;
		m_PostSocket = INVALID_SOCKET;
	}
	void PostAcceptRecycle::Clear()
	{
		m_PostSocket = INVALID_SOCKET;
		memset(&m_OverLapped, 0, sizeof(m_OverLapped));
	}

	PostAcceptRecycle* PostAcceptRecycle::Pop()
	{
		PostAcceptRecycle* buff = nullptr;
		if (__accepts.empty() == true)
		{
			buff = new PostAcceptRecycle(common::E_CT_Accept);
		}
		else
		{
			__accepts.try_pop(buff);
			if (buff == nullptr)
			{
				buff = new PostAcceptRecycle(common::E_CT_Accept);
			}
		}
		return buff;
	}

	void PostAcceptRecycle::Push(PostAcceptRecycle* acc)
	{
		if (acc == nullptr)
		{
			return;
		}
		if (__accepts.unsafe_size() > 1000)
		{
			delete acc;
			return;
		}
		acc->Clear();
		__accepts.push(acc);
	}

	PostRecvRecycle::PostRecvRecycle(int type)
	{
		m_PostType = type;
		m_PostSocket = INVALID_SOCKET;
		m_Buffs = new char[common::ServerXML->recvBytesOne];

		memset(&m_OverLapped, 0, sizeof(m_OverLapped));
		memset(m_Buffs, 0, common::ServerXML->recvBytesOne);
		m_wsaBuf.buf = m_Buffs;
		m_wsaBuf.len = common::ServerXML->recvBytesOne;

	}

	PostRecvRecycle::~PostRecvRecycle()
	{
		m_PostSocket = INVALID_SOCKET;
		m_PostSocket = -1;
	}

	void PostRecvRecycle::Clear()
	{
		m_PostSocket = INVALID_SOCKET;
		memset(m_Buffs, 0, common::ServerXML->recvBytesOne);
		m_wsaBuf.buf = m_Buffs;
		m_wsaBuf.len = common::ServerXML->recvBytesOne;
	}
	PostRecvRecycle* PostRecvRecycle::Pop()
	{
		PostRecvRecycle* buff = nullptr;
		if (__recvs.empty() == true)
		{
			buff = new PostRecvRecycle(common::E_CT_Accept);
		}
		else
		{
			__recvs.try_pop(buff);
			if (buff == nullptr)
			{
				buff = new PostRecvRecycle(common::E_CT_Accept);
			}
		}
		return buff;
	}
	void PostRecvRecycle::Push(PostRecvRecycle* acc)
	{
		if (acc == nullptr)
		{
			return;
		}
		if (__recvs.unsafe_size() > 1000)
		{
			delete acc;
			return;
		}
		acc->Clear();
		__recvs.push(acc);
	}

	PostSendRecycle::PostSendRecycle(int type)
	{
		m_PostType = type;
		m_PostSocket = INVALID_SOCKET;
		m_Buffs = new char[common::ServerXML->sendBytesOne];

		memset(&m_OverLapped, 0, sizeof(m_OverLapped));
		memset(m_Buffs, 0, common::ServerXML->sendBytesOne);
		m_wsaBuf.buf = m_Buffs;
		m_wsaBuf.len = common::ServerXML->sendBytesOne;
	}
	PostSendRecycle::~PostSendRecycle()
	{
		m_PostSocket = INVALID_SOCKET;
		m_PostSocket = common::E_CT_Send;
	}
	void PostSendRecycle::Clear()
	{
		m_PostSocket = INVALID_SOCKET;
		memset(m_Buffs, 0, common::ServerXML->recvBytesOne);
		m_wsaBuf.buf = m_Buffs;
		m_wsaBuf.len = common::ServerXML->recvBytesOne;
	}
	int PostSendRecycle::SetPostSend(SOCKET s, char* data, const int sendByte)
	{
		m_PostSocket = s;
		if (&m_wsaBuf)
		{
			if (sendByte != 0 && data)
			{
				memcpy(m_Buffs, data, sendByte);
				m_wsaBuf.buf = m_Buffs;
				m_wsaBuf.len = sendByte;
			}
		}
		return sendByte;
	}
	PostSendRecycle* PostSendRecycle::Pop()
	{
		PostSendRecycle* buff = nullptr;
		if (__sends.empty() == true)
		{
			buff = new PostSendRecycle(common::E_CT_Send);
		}
		else
		{
			__sends.try_pop(buff);
			if (buff == nullptr)
			{
				buff = new PostSendRecycle(common::E_CT_Send);
			}
		}
		return buff;
	}
	void PostSendRecycle::Push(PostSendRecycle* acc)
	{
		if (acc == nullptr)
		{
			return;
		}
		if (__sends.unsafe_size() > 1000)
		{
			delete acc;
			return;
		}
		acc->Clear();
		__sends.push(acc);
	}
}


