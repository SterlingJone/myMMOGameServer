#include "GameCommand.h"

#include "GameTest.h"

namespace app
{
	std::vector<tcp::IClient*>   __IClientArr;




	void Event_Client_Accpet(tcp::IClient* tcp, const int code)
	{
		LOGINFO("client---connect...%d\n",tcp->GetDataPoint()->ID);
	}

	void Event_Client_Secure(tcp::IClient* tcp, const int code)
	{
		LOGINFO("client---secure...%d\n", tcp->GetDataPoint()->ID);
	}

	void Event_Client_Disconnect(tcp::IClient* tcp, const int code)
	{
		LOGINFO("client---disconnect...%d err:%d\n", tcp->GetDataPoint()->ID,code);
	}

	void Event_Client_Command(tcp::IClient* tcp, const int cmd)
	{
		auto c = tcp->GetDataPoint();
		if (c->state < common::E_CSS_Secure) return;
		switch (cmd)
		{
		case 9000:
			__Test->ClientCommand(tcp, cmd);
			break;
		}
	}

}