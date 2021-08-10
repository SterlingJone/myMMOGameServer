#ifndef  __ILINUXSERVER_H__
#define  __ILINUXSERVER_H__

#include "ITcp.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <list>
#include <netinet/in.h>

#define FDSIZE       9999 
#define EPOLLEVENTS  1024


namespace tcp
{

	class LinuxServer :public IServer
	{
	private:
		int   InitSocket();
		void  InitThread();

		//解包相关
		void  ReadPackage_Head(S_CONNECT_BASE* c);
		void  ReadPackage_Command(S_CONNECT_BASE* c, uint16_t cmd);
		void  UpdateDisconnect(S_CONNECT_BASE* c);

		int32_t ReleaseSocket(int socketfd, S_CONNECT_BASE* c, int kind);
		void ShutDownSocket(int s, const int32_t mode, S_CONNECT_BASE* c, int kind);

		bool SetNonblock(int fd);
		void Event_Delete(int epollfd, int sockfd, int state);
		int  Event_Add(int epollfd, int sockfd, int state);

		void Event_Accept();
		void Event_Recv(int socketfd);
		int  Event_Send(S_CONNECT_BASE* c);

		S_CONNECT_BASE* FindNoStateData();
		S_CONNECT_INDEX* FindOnlinesIndex(const int socketfd);

		void ComputeSecureNum(bool isadd);
		void ComputeConnectNum(bool isadd);

		static void Thread_Manager(LinuxServer* epoll);
		static void Thread_Accept(LinuxServer* epoll);
		static void Thread_Recv(LinuxServer* epoll);
	public:
		LinuxServer();
		virtual ~LinuxServer();

		virtual inline int ConnnectCount() { return m_ConnectCount; }
		virtual inline int SecureCount() { return m_SecurityCount; }

		virtual void StartServer();
		virtual void StopServer();

		virtual bool IsCloseClient(const int index, int secure);
		virtual S_CONNECT_BASE* FindClient(const int socketfd, bool issecure);
		virtual S_CONNECT_BASE* FindClient(const int index);

		virtual void Update();

		virtual void CreatePackage(const int index, const uint16_t cmd, void* v, const int len);
		virtual void ReadPackage(const int index, void* v, const int len) ;

		virtual void SetNotify_Connect(ISERVER_NOTIFY e);
		virtual void SetNotify_Secure(ISERVER_NOTIFY e);
		virtual void SetNotify_DisConnect(ISERVER_NOTIFY e);
		virtual void SetNotify_Command(ISERVER_NOTIFY e);


	private:
		//玩家数据
		common::HashContainer<S_CONNECT_BASE>* m_Onlines;//在线玩家数据
		common::HashContainer<S_CONNECT_INDEX>* m_OnlinesIndexs;//连接玩家索引数组
		//通知事件
		ISERVER_NOTIFY      m_Notify_Accept;
		ISERVER_NOTIFY      m_Notify_Secure;
		ISERVER_NOTIFY      m_Notify_Disconnect;
		ISERVER_NOTIFY      m_Notify_Command;
		//互斥锁
		std::mutex					 m_Mutex_ConnectCount;//连接数量互拆锁
		std::mutex					 m_Mutex_SecureCount;//连接数量互拆锁
		std::mutex					 m_Mutex_Accept;//监听互拆锁
		std::mutex					 m_Mutex_Recv;  //读取互拆锁
		std::mutex					 m_Mutex_NoState;//寻找空位互拆锁

		//私有数据
		int32_t             m_ConnectCount;//当前连接数
		int32_t             m_SecurityCount;//安全连接数

		bool                m_IsRunning;
		std::condition_variable		 m_Condition_Accept; //监听条件变量
		std::condition_variable		 m_Condition_Recv;   //读取条件变量

		std::shared_ptr<std::thread> m_Thread_Accept;
		std::shared_ptr<std::thread> m_Thread_Recv;
		std::shared_ptr<std::thread> m_Thread_Manager;

		std::list<int>				 m_SocketfdArr;
		int listenfd;//监听fd
		int epollfd; //epoll fd
		char* temp_Buf;
	};





}




#endif