#include "GameManager.h"
#include "GamePlayer.h"
#include "GameCommand.h"
#include "PublicEntry.h"
#include "GameTest.h"

#include <string.h>
#include <unistd.h>


namespace app
{
	int temp_time = 0;
	char printfstr[1000];
	//´òÓ¡ÐÅÏ¢
	void printInfo()
	{
	}

	void Init()
	{
		__IServer = tcp::NewIServer();
		__IServer->SetNotify_Connect(Event_Server_Accept);
		__IServer->SetNotify_Secure(Event_Server_Secure);
		__IServer->SetNotify_DisConnect(Event_Server_Disconnect);
		__IServer->SetNotify_Command(Event_Server_Command);

		__IServer->StartServer();

		__Player = new GamePlayer();
		__Test = new GameTest();
	}

	void Update()
	{
		__IServer->Update();
		__Player->Update();
		printInfo();

	}
	int StartApp()
	{
		if (!entry::Init()) return -1;
		Init();

		while (true)
		{
			Update();
			sleep(2);
		}

		return 0;
	}

}

