#include "GameTest.h"
#include "PublicEntry.h"
#include "GameCommand.h"





namespace app
{	
	IGameBase* __Test = nullptr;
	GameTest::GameTest()
	{}

	GameTest::~GameTest()
	{}

	void GameTest::Init()
	{}

	void GameTest::Update()
	{}





	//定义一个测试数
	S_TEST_BASE   test;
	void Test_9000(tcp::IServer* ts, tcp::S_CONNECT_BASE* c)
	{
		ts->ReadPackage(c->index, &test, sizeof(S_TEST_BASE));

		ts->CreatePackage(c->index, 9000, &test, sizeof(S_TEST_BASE));
	}



	bool GameTest::ServerCommand(tcp::IServer* ts, tcp::S_CONNECT_BASE* c, const uint16_t cmd)
	{
		if (ts->IsCloseClient(c->index, common::E_SSS_Secure))
		{
			LOGINFO("GameTest err...line:%d \n", __LINE__);
			return false;
		}
	
		switch (cmd)
		{
		case 9000:
			Test_9000(ts, c); 
			break;
		}


		return true;
	}
}

