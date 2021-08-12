#ifndef  __IDATA_H__
#define  __IDATA_H__

#include  "malloc.h"

#include <vector>
#include <cstdint>




#define LOGINFO             printf
#define MAX_SOCKETFD_LEN    1000000
#define MAX_FILE_LEN        250
#define MAX_MD5_LEN         35
#define MAX_THREAD_LEN      5
#define MAX_IP_LEN          20 

#define CMD_HEART     65000
#define CMD_XOR       65531
#define CMD_SECURITY  65532

namespace common
{
	enum E_CONTEXT_TYPE
	{
		E_CT_Accept = 0,
		E_CT_Recv = 1,
		E_CT_Send = 2
	};

	//服务器
	enum E_SERVER_SOCKET_STATE
	{
		E_SSS_Free = 0,
		E_SSS_Connect = 1,
		E_SSS_Secure = 2,
		E_SSS_Login = 3,
		E_SSS_NeedSave = 4,
		E_SSS_Saving = 5
	};
	//客户端SOCKET枚举状态
	enum E_CLIENT_SOCKET_STATE
	{
		E_CSS_Free = 0,
		E_CSS_TryConnect = 1,
		E_CSS_Connect = 2,
		E_CSS_Secure = 3,
		E_CSS_Login = 4
	};
	//关闭状态
	enum E_SERVER_SOCKET_CLOSESTATE
	{
		E_SSC_Free = 0,
		E_SSC_Accept = 1,   //连接出错关闭
		E_SSC_ShutDown = 2, //关闭连接
		E_SSC_Close = 3     //正式关闭
	};
	//0 玩家 1-DB 2-中心服务器 3-游戏服务器 4-网关服务器 5-登录服务器
	enum E_APP_TYPE
	{
		E_APP_Player = 0,
		E_APP_DB = 1,
		E_APP_Center = 2,
		E_APP_Game = 3,
		E_APP_Gate = 4,
		E_APP_Login = 5
	};
	struct ConfigXML
	{
		uint16_t  appPort;   //服务器端口号
		int32_t   appID;     //服务器ID
		int32_t   appMaxPlayer;//最大玩家数量
		int32_t   appMaxConnect;//最大客户端连接数量
		uint8_t   appXorCode; //异或码 主要用于数据加密
		int32_t   appVersion; //当前应用程序版本号
		int32_t   recvBytesOne;//当前一次接收数据字节长度
		int32_t   recvBytesMax;//最大接收缓冲区
		int32_t   sendBytesOne;//当前一次发送数据字节长度
		int32_t   sendBytesMax;//最大发送缓冲区
		int32_t   maxHeartTime;//心跳时间
		int32_t   autoConnectTime;//自动重连时间
		int32_t   maxPostAccept; //最大投递连接数量
		int32_t   maxPostRecv;   //最大收到消息数量
		int32_t   maxPostSend;   //最大发送信息数量
		int32_t   maxThread;     //设置当前最大线程
		char  SecureCode[20];
		char  Head[3];
	};
	struct ServerListXML
	{
		int32_t    ID;
		uint16_t   Port;
		char       IP[16];
	};

	template<class T>
	class HashContainer
	{
	private:
		int     m_Count;
		void*   m_Pointer;
	public:
		HashContainer()
		{
			m_Count = 0;
			m_Pointer = nullptr;
		}
		HashContainer(int counter)
		{
			m_Count = 0;
			if (counter < 1) return;
			m_Count = counter;
			m_Pointer = malloc(m_Count * sizeof(T));
		}
		virtual ~HashContainer()
		{
			if (m_Pointer != nullptr)
			{
				free(m_Pointer);
				m_Pointer = nullptr;
			}
		}
		T* Value(const int index)
		{
			T* v = (T*)m_Pointer;
			return &v[index];
		}
		int Count()
		{
			return m_Count;
		}


	};


	extern char FileExePath[MAX_FILE_LEN];
	extern ConfigXML* ServerXML;
	extern ConfigXML* ClientXML;   
	extern std::vector<ServerListXML*> ServerXMLS;

	extern uint8_t GetServerType(int32_t sid);
	extern void(*MD5_FunPoint)(char* output, unsigned char* input, int len);
	extern void InitPath();
}


namespace tcp
{


#pragma pack(push,packing)
#pragma pack(1)

	struct S_CONNECT_INDEX
	{
		int index;
		inline void Reset() { index = -1; }
	};
	struct S_BUFFS_BASE
	{
		char*   buf; //缓冲区
		int     head;//消费者偏移 头
		int     tail;//生产者偏移 尾
		bool    isCompleted;
	};
	//服务器连接用户数据结构
	struct S_CONNECT_BASE
	{
		int    index;
		int    socketfd;

		int8_t    state;
		int8_t    closeState;
		char      ip[MAX_IP_LEN];
		uint16_t  port;
		uint8_t   xorCode;
		int32_t   appID;//连接程序的ID

		S_BUFFS_BASE   recvs;
		S_BUFFS_BASE   sends;
		int      packageLength;//数据包长度


		int32_t  temp_ConnectTime;
		int32_t  temp_HeartTime;
		int32_t  temp_CloseTime;
		int32_t  temp_ShutDown;

		void Init();
		void Reset();

		inline bool IsEqual(int sid)
		{
			if (socketfd == sid) return true;
			return false;
		}
	};
	//客户端结构体
	struct S_CLINET_BASE
	{
		int              ID;
		char             ip[MAX_IP_LEN];
		uint16_t         port;
		int32_t          appID;
		uint8_t          state;
		uint8_t          xorCode;

		S_BUFFS_BASE     recvs;
		S_BUFFS_BASE     sends;
		int32_t          packageLength;//消息长度
		char* temp_buf;

		int32_t          temp_HeartTime;
		int32_t          temp_AutoConnectTime;
		int32_t          temp_testtime;
		void Init(int sid);
		void Reset();
	};



	//CMD_XOR
	struct S_CMD_XOR
	{
		uint8_t  xorCode;
	};
	//CMD_SECURE
	struct S_CMD_SECURE
	{
		uint32_t  appID;
		uint32_t  appVersion;
		char      appMD5[MAX_MD5_LEN];
	};
	//通用
	struct S_CMD_RESULT
	{
		int32_t    type;
	};

#pragma pack(pop, packing)


	class IServer;
	class IClient;
	typedef void(*ISERVER_NOTIFY)(IServer* tcp, S_CONNECT_BASE* c, const int code);
	typedef void(*ICLIENT_NOTIFY)(IClient* tcp, const int code);
}

namespace app
{
	class IGameBase
	{
	public:
		IGameBase() {}
		virtual ~IGameBase() {}
		virtual void  Init() {}
		virtual void  Update() {}
		virtual bool  ServerCommand(tcp::IServer* ts, tcp::S_CONNECT_BASE* c, const uint16_t cmd) { return false; }
		virtual bool  ClientCommand(tcp::IClient* tc, const uint16_t cmd) { return false; }
	};
}
//github test
//github test
#endif // __IDATA_H__
