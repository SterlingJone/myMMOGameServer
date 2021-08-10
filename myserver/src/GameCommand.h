#ifndef  __GAMEMCOMMAND_H__
#define  __GAMEMCOMMAND_H__

#include "ITcp.h"
namespace app
{
	extern tcp::IServer* __IServer;
	extern void Event_Server_Accept(tcp::IServer* tcp, tcp::S_CONNECT_BASE* c, const int32_t code);
	extern void Event_Server_Secure(tcp::IServer* tcp, tcp::S_CONNECT_BASE* c, const int32_t code);
	extern void Event_Server_Disconnect(tcp::IServer* tcp, tcp::S_CONNECT_BASE* c, const int32_t code);
	extern void Event_Server_Command(tcp::IServer* tcp, tcp::S_CONNECT_BASE* c, const int32_t cmd);
}

#endif