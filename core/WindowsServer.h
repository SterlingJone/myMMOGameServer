#ifndef __WINDOWSSERVER_H__
#define __WINDOWSSERVER_H__
#include "ITcp.h"

#include <mutex>
#include <thread>
#include <MSWSock.h>

#define ACCEPT_BUF_LENGTH  (sizeof(struct sockaddr_in) + 16) * 2

namespace recycle
{
	class RecycleBase
	{
	public:
		RecycleBase();
		~RecycleBase();
	public:
		WSAOVERLAPPED m_OverLapped;
		SOCKET m_PostSocket;
		int m_PostType;
	};

	class PostAcceptRecycle : public RecycleBase
	{
	public:
		PostAcceptRecycle(int type);
		~PostAcceptRecycle();
	public:
		unsigned char m_buf[ACCEPT_BUF_LENGTH];
		void Clear();
		static PostAcceptRecycle* Pop();
		static void Push(PostAcceptRecycle* acc);
	};

	class PostRecvRecycle : public RecycleBase
	{
	public:
		PostRecvRecycle(int type);
		~PostRecvRecycle();
	private:
		char* m_Buffs;
	public:
		WSABUF m_wsaBuf;
		void Clear();
		static PostRecvRecycle* Pop();
		static void Push(PostRecvRecycle* acc);
	};

	class PostSendRecycle : public RecycleBase
	{
	public:
		PostSendRecycle(int type);
		~PostSendRecycle();
	private:
		char* m_Buffs;
	public:
		WSABUF m_wsaBuf;
		void Clear();
		int SetPostSend(SOCKET s, char* data, const int sendByte);
		static PostSendRecycle* Pop();
		static void Push(PostSendRecycle* acc);
	};
}

namespace tcp
{
	class WindowsServer : public IServer
	{
	public:
		WindowsServer();
		virtual ~WindowsServer();

		virtual inline int ConnectCount() { return m_ConnectCount; };
		virtual inline int SecureCount() { return m_SecurityCount; }

		virtual void StartServer();
		virtual void StopServer();

		virtual bool IsCloseClient(const int index, int secure);
		virtual S_CONNECT_BASE* FindClient(const int socketfd, bool issecure);
		virtual S_CONNECT_BASE* FindClient(const int index);

		virtual void Update();

		virtual void CreatePackage(const int index, const uint16_t cmd, void* v, const int len);
		virtual void ReadPackage(const int index, void* v, const int len);

		virtual void SetNotify_Connect(ISERVER_NOTIFY e);
		virtual void SetNotify_Secure(ISERVER_NOTIFY e);
		virtual void SetNotify_DisConnect(ISERVER_NOTIFY e);
		virtual void SetNotify_Command(ISERVER_NOTIFY e);
	private:
		int InitSocket();
		void UpdateDisconnect(S_CONNECT_BASE* c);
		void ShutDownSocket(SOCKET s, const int32_t mode, S_CONNECT_BASE* c, int kind);
		int32_t ReleaseSocket(SOCKET soketfd, S_CONNECT_BASE* c, int kind);
		S_CONNECT_BASE* FindNoStateData();
		S_CONNECT_INDEX* FindOnlinesIndex(const int socketfd);
		void ComputeSecureNum(bool isadd);
		void ComputeConnectNum(bool isadd);
	private:
		
		common::HashContainer<S_CONNECT_BASE>* m_Onlines;
		common::HashContainer<S_CONNECT_INDEX>* m_OnlinesIndexs;
		
		ISERVER_NOTIFY      m_Notify_Accept;
		ISERVER_NOTIFY      m_Notify_Secure;
		ISERVER_NOTIFY      m_Notify_Disconnect;
		ISERVER_NOTIFY      m_Notify_Command;
		
		std::mutex          m_Mutex_NoState;
		std::mutex          m_Mutex_ConnectCount;
		std::mutex          m_Mutex_SecureCount;
		
		int32_t             m_ConnectCount;
		int32_t             m_SecurityCount;

		bool                         m_IsRunning;
		SOCKET                       m_ListenSocket;   
		HANDLE                       m_Completeport;
		LPFN_ACCEPTEX                m_AcceptEx;
		LPFN_GETACCEPTEXSOCKADDRS    m_GetAcceptEx;
		std::shared_ptr<std::thread> m_WorkThread[MAX_THREAD_LEN];
	};
}
#endif