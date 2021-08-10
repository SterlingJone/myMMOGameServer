#include "GameManager.h"
#include "GameCommand.h"
#include "PublicEntry.h"
#include "GameTest.h"

#include <time.h>
#include <string.h>
#include <unistd.h>

namespace app
{
	
	void Init()
	{
		__Test = new GameTest();

		auto xml = common::ServerXMLS[0];
		__IClientArr.reserve(TESTCONNECT);

		for (int i = 0; i < TESTCONNECT; i++)
		{
			auto client = tcp::NewIClient();
			client->SetNotify_Connect(Event_Client_Accpet);
			client->SetNotify_Secure(Event_Client_Secure);
			client->SetNotify_DisConnect(Event_Client_Disconnect);
			client->SetNotify_Command(Event_Client_Command);
			__IClientArr.emplace_back(client);

			client->StartClient(xml->ID, xml->IP, xml->Port);
			client->GetDataPoint()->ID = i;

		}
	}

	void Update()
	{
		for (int i = 0; i < TESTCONNECT; i++)
		{
			__IClientArr[i]->Update();
		}
		__Test->Update();
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

