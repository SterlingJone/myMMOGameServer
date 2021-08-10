#ifndef  ____ITCP_H
#define  ____ITCP_H

#include "IData.h"

namespace tcp
{
	class IServer
	{
	public:
		virtual ~IServer() {}

		virtual int ConnnectCount() = 0;
		virtual int SecureCount() = 0;

		virtual void StartServer() = 0;
		virtual void StopServer() = 0;

		virtual bool IsCloseClient(const int index, int secure) = 0;
		virtual S_CONNECT_BASE* FindClient(const int socketfd, bool issecure) = 0;
		virtual S_CONNECT_BASE* FindClient(const int index) = 0;

		virtual void Update() = 0;

		virtual void CreatePackage(const int index, const uint16_t cmd, void* v, const int len) = 0;
		virtual void ReadPackage(const int index, void* v, const int len) = 0;

		virtual void SetNotify_Connect(ISERVER_NOTIFY e) = 0;
		virtual void SetNotify_Secure(ISERVER_NOTIFY e) = 0;
		virtual void SetNotify_DisConnect(ISERVER_NOTIFY e) = 0;
		virtual void SetNotify_Command(ISERVER_NOTIFY e) = 0;
	};
	class IClient
	{
	public:
		virtual ~IClient() {}
		virtual S_CLINET_BASE* GetDataPoint() = 0;
		virtual int GetSocket() = 0;
		virtual void StartClient(int sid, char* ip, int port) = 0;
		virtual bool ConnectServer() = 0;
		virtual void DisconnectServer(int errcode, const char* err) = 0;

		virtual void Update() = 0;
		virtual void CreatePackage(const uint16_t cmd, void* v, const int len) = 0;
		virtual void ReadPackage(void* v, const int len) = 0;

		virtual void SetNotify_Connect(ICLIENT_NOTIFY e) = 0;
		virtual void SetNotify_Secure(ICLIENT_NOTIFY e) = 0;
		virtual void SetNotify_DisConnect(ICLIENT_NOTIFY e) = 0;
		virtual void SetNotify_Command(ICLIENT_NOTIFY e) = 0;
	};
	extern IClient*   NewIClient();
	extern IServer*   NewIServer();
}


#endif