#ifndef  ____MYCLIENT_H
#define  ____MYCLIENT_H
#include "ITcp.h"
#include <mutex>
#include <thread>


namespace tcp
{
	class MyClient:public IClient
	{
	public:
		MyClient();
		virtual ~MyClient();
		virtual inline S_CLINET_BASE* GetDataPoint() { return &m_data; }
		virtual inline int GetSocket() { return socketfd; }
		virtual void StartClient(int sid, char* ip, int port) ;
		virtual bool ConnectServer();
		virtual void DisconnectServer(int errcode,const char* err);

		virtual void Update();
		virtual void CreatePackage(const uint16_t cmd, void* v, const int len);
		virtual void ReadPackage(void* v, const int len);

		virtual void SetNotify_Connect(ICLIENT_NOTIFY e);
		virtual void SetNotify_Secure(ICLIENT_NOTIFY e);
		virtual void SetNotify_DisConnect(ICLIENT_NOTIFY e);
		virtual void SetNotify_Command(ICLIENT_NOTIFY e);
	private:
		int      socketfd;
		S_CLINET_BASE  m_data;
		std::shared_ptr<std::thread> m_WorkThread;

		ICLIENT_NOTIFY      m_Notify_Accept;
		ICLIENT_NOTIFY      m_Notify_Secure;
		ICLIENT_NOTIFY      m_Notify_Disconnect;
		ICLIENT_NOTIFY      m_Notify_Command;

		int InitSocket();
		bool setNonblockingSocket();

		void Connect_Select();
		void AutoConnect();
	
		int Event_Recv();
		int Event_Send();

		void  ReadPackage_Head();
		void  ReadPackage_Command(uint16_t cmd);

		void  SendHeart();
		void InitThread();
		static void RunThread(MyClient* tcp);

	};




}


#endif