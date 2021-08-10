#ifndef  ____GAMEMCOMMAND_H
#define  ____GAMEMCOMMAND_H

#include "ITcp.h"

#define TESTCONNECT 1000

namespace app
{
	extern std::vector<tcp::IClient*>   __IClientArr;

	extern void Event_Client_Accpet(tcp::IClient* tcp, const int code);
	extern void Event_Client_Secure(tcp::IClient* tcp, const int code);
	extern void Event_Client_Disconnect(tcp::IClient* tcp, const int code);
	extern void Event_Client_Command(tcp::IClient* tcp, const int cmd);

}

#endif