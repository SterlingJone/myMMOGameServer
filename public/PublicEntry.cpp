#include "PublicEntry.h"
#include "IData.h"
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"
#include "tinyxml/md5.h"


using namespace common;

namespace entry
{
	int LoadAppConfig(const char* filename)
	{
		char fpath[MAX_FILE_LEN];
		memset(fpath, 0, MAX_FILE_LEN);
		sprintf(fpath, "%s%s", common::FileExePath, filename);

		if (common::ServerXML == nullptr)
		{
			common::ServerXML = new common::ConfigXML();
			memset(common::ServerXML, 0, sizeof(common::ConfigXML));
		}
		TiXmlDocument xml;
		if (!xml.LoadFile(fpath))
		{
			return -1;
		}

		TiXmlElement* xmlRoot = xml.RootElement();
		if (xmlRoot == NULL)
		{
			return -2;
		}

		//获取子节点信息1  
		TiXmlElement* xmlNode = xmlRoot->FirstChildElement("app");
		memcpy(ServerXML->SecureCode, xmlNode->Attribute("SecureCode"), 20);
		memcpy(ServerXML->Head, xmlNode->Attribute("Head"), 2);
		ServerXML->appPort = atoi(xmlNode->Attribute("appPort"));
		ServerXML->appID = atoi(xmlNode->Attribute("appID"));
		ServerXML->appMaxPlayer = atoi(xmlNode->Attribute("appMaxPlayer"));
		ServerXML->appMaxConnect = atoi(xmlNode->Attribute("appMaxConnect"));
		ServerXML->maxPostAccept = atoi(xmlNode->Attribute("maxPostAccept"));
		ServerXML->maxPostRecv = atoi(xmlNode->Attribute("maxPostRecv"));
		ServerXML->maxPostSend = atoi(xmlNode->Attribute("maxPostSend"));
		ServerXML->appXorCode = atoi(xmlNode->Attribute("appXorCode"));
		ServerXML->appVersion = atoi(xmlNode->Attribute("appVersion"));
		ServerXML->recvBytesOne = atoi(xmlNode->Attribute("recvBytesOne")) * 1024;
		ServerXML->recvBytesMax = atoi(xmlNode->Attribute("recvBytesMax")) * 1024;
		ServerXML->sendBytesOne = atoi(xmlNode->Attribute("sendBytesOne")) * 1024;
		ServerXML->sendBytesMax = atoi(xmlNode->Attribute("sendBytesMax")) * 1024;
		ServerXML->maxHeartTime = atoi(xmlNode->Attribute("maxHeartTime"));
		ServerXML->maxThread = atoi(xmlNode->Attribute("maxThread"));
		return 0;

	}
	int LoadRemoteConfig(const char* filename)
	{

		char fpath[MAX_FILE_LEN];
		memset(fpath, 0, MAX_FILE_LEN);
		sprintf(fpath, "%s/%s", FileExePath, filename);

		if (ClientXML == nullptr)
		{
			ClientXML = new ConfigXML();
			memset(ClientXML, 0, sizeof(ConfigXML));
		}
		TiXmlDocument xml;
		if (!xml.LoadFile(fpath))
		{
			return -1;
		}
		TiXmlElement* xmlRoot = xml.RootElement();
		if (xmlRoot == NULL)
		{
			return -2;
		}

		//获取子节点信息1  
		TiXmlElement* xmlNode = xmlRoot->FirstChildElement("remote");
		memcpy(ClientXML->SecureCode, xmlNode->Attribute("SecureCode"), 20);
		memcpy(ClientXML->Head, xmlNode->Attribute("Head"), 2);
		ClientXML->appMaxPlayer = 1;
		ClientXML->appMaxConnect = 1;
		ClientXML->appXorCode = atoi(xmlNode->Attribute("appXorCode"));
		ClientXML->appVersion = atoi(xmlNode->Attribute("appVersion"));
		ClientXML->recvBytesOne = atoi(xmlNode->Attribute("recvBytesOne")) * 1024;
		ClientXML->recvBytesMax = atoi(xmlNode->Attribute("recvBytesMax")) * 1024;
		ClientXML->sendBytesOne = atoi(xmlNode->Attribute("sendBytesOne")) * 1024;
		ClientXML->sendBytesMax = atoi(xmlNode->Attribute("sendBytesMax")) * 1024;
		ClientXML->maxHeartTime = atoi(xmlNode->Attribute("maxHeartTime"));
		ClientXML->autoConnectTime = atoi(xmlNode->Attribute("autoConnectTime"));

		//获取子节点信息1  
		xmlNode = xmlRoot->FirstChildElement("remote_count");
		int num = atoi(xmlNode->Attribute("count"));
		char str[10];
		for (int i = 1; i <= num; i++)
		{
			memset(&str, 0, 10);
			sprintf(str, "remote_%d", i);
			xmlNode = xmlRoot->FirstChildElement(str);

			ServerListXML* serverlist = new ServerListXML();
			memcpy(serverlist->IP, xmlNode->Attribute("IP"), 16);
			serverlist->Port = atoi(xmlNode->Attribute("Port"));
			serverlist->ID = atoi(xmlNode->Attribute("ID"));

			ServerXMLS.push_back(serverlist);
		}

		return 0;

	}
	bool Init()
	{
		//设置函数指针
		common::MD5_FunPoint = &utils::EncryptMD5str;
		//初始化数据
		common::InitPath();
		int errs = LoadAppConfig("config_app.xml");
		if (errs < 0)
		{
			return false;
		}

		errs = LoadRemoteConfig("config_remote.xml");
		if (errs < 0)
		{
			return false;
		}

		if (common::ServerXMLS.size() < 1)
		{
			return false;
		}

		LOGINFO("xml:%d-%d \n", ServerXML->appXorCode, ServerXML->appVersion);
		return true;

	}

}