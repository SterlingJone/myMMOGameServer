#include "GameTest.h"

#include "PublicEntry.h"
#include <time.h>
#include <cstring>


namespace app
{
	IGameBase* __Test = nullptr;
	GameTest::GameTest()
	{
	}

	GameTest::~GameTest()
	{
	}

	void GameTest::Init()
	{
	}

	//定义一个测试数
	S_TEST_BASE   test;

	//测试客户端发送消息给服务器
	void GameTest::Update()
	{
		for (int i = 0; i < TESTCONNECT; i++)
		{
			auto client = __IClientArr[i];
			auto c = client->GetDataPoint();
			if (c->state < common::E_CSS_Secure) continue;

			int tempTime = (int)time(NULL) - c->temp_testtime;
			if (tempTime > 2)
			{
				memset(&test, 0, sizeof(S_TEST_BASE));
				test.curtime = clock();
				test.id = i;

				test.aa[0] = 1;
				test.aa[11] = 2;
				test.aa[22] = 3;

				c->temp_testtime = (int)time(NULL);
				client->CreatePackage(9000, &test, sizeof(S_TEST_BASE));

			}
		}
		
	}


	//收到数据 解析
	void Test_9000(tcp::IClient* tc)
	{
	
		tc->ReadPackage(&test, sizeof(S_TEST_BASE));
	
		if (test.id == 0)
		{
			int ftime = clock() - test.curtime;
			LOGINFO("GameTest id:%d  arr:%d/%d/%d  time:%d 毫秒/ \n",
				test.id, test.aa[0], test.aa[11], test.aa[22], ftime);
		}


	}
	bool GameTest::ClientCommand(tcp::IClient* tc, const uint16_t cmd)
	{
		auto c = tc->GetDataPoint();
		if(c->state < common::E_CSS_Secure) return false;

		switch (cmd)
		{
		     case 9000:Test_9000(tc); break;
		}
		return true;
	}
}

