#include "IData.h"

#include <string>
#include <iostream>

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace common
{
	char FileExePath[MAX_FILE_LEN];
	ConfigXML *ServerXML = NULL;
    ConfigXML *ClientXML = NULL;
	std::vector<ServerListXML *> ServerXMLS;
	void(*MD5_FunPoint)(char *output, unsigned char *input, int len) = NULL;
	
	void InitPath()
	{
		memset(FileExePath, 0, MAX_FILE_LEN);

		int ret = readlink("/proc/self/exe", FileExePath, MAX_FILE_LEN);
		std::string str(FileExePath);
		size_t pos = str.find_last_of("/");
		str = str.substr(0, pos + 1);

		memcpy(FileExePath, str.c_str(), MAX_FILE_LEN);
		LOGINFO("path:%s \n", FileExePath);

	}
	uint8_t GetServerType(int32_t sid)
	{
		if (sid >= 10000 && sid < 20000)
		{
			return common::E_APP_DB;
		}
		else if (sid >= 20000 && sid < 30000)
		{
			return common::E_APP_Center;
		}
		else if (sid >= 30000 && sid < 40000) 
		{
			return common::E_APP_Game;
		}
		else if (sid >= 40000 && sid < 50000)
		{
			return common::E_APP_Gate;
		}
		else if (sid >= 50000 && sid < 60000)
		{
			return common::E_APP_Login;
		}
		return common::E_APP_Player;

	}
}

namespace tcp
{
	void S_CONNECT_BASE::Init()
	{
		recvs.buf = new char[common::ServerXML->recvBytesMax];
		sends.buf = new char[common::ServerXML->sendBytesMax];

		Reset();
	}
	void S_CONNECT_BASE::Reset()
	{
		socketfd = -1;
		port = 0;
		index = -1;
		xorCode = common::ServerXML->appXorCode;

		recvs.head = 0;
		recvs.tail = 0;
		recvs.isCompleted = false;
		sends.head = 0;
		sends.tail = 0;
		sends.isCompleted = true;

		packageLength = 0;

		temp_ConnectTime = (int)time(NULL);
		temp_HeartTime = (int)time(NULL);
		temp_CloseTime = (int)time(NULL);

		memset(recvs.buf, 0, common::ServerXML->recvBytesMax);
		memset(sends.buf, 0, common::ServerXML->recvBytesMax);
		memset(ip, 0, MAX_IP_LEN);

		closeState = 0;
		state = common::E_CSS_Free;
	}

	void S_CLINET_BASE::Init(int sid)
	{
		appID = sid;
		recvs.buf = new char[common::ClientXML->recvBytesMax];
		sends.buf = new char[common::ClientXML->sendBytesMax];
		temp_buf = new char[common::ClientXML->recvBytesOne];
		port = 0;
		memset(ip, 0, MAX_IP_LEN);
		Reset();
	}
	void S_CLINET_BASE::Reset()
	{
		state = 0;
		xorCode = common::ClientXML->appXorCode;
		recvs.head = 0;
		recvs.tail = 0;
		packageLength = 0;
		sends.head = 0;
		sends.tail = 0;
		temp_HeartTime = (int)time(NULL);
		temp_AutoConnectTime = (int)time(NULL);
		temp_testtime = 0;
		memset(recvs.buf, 0, common::ClientXML->recvBytesMax);
		memset(sends.buf, 0, common::ClientXML->sendBytesMax);
		memset(temp_buf, 0, common::ClientXML->recvBytesOne);
	}
}