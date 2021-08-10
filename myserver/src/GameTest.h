#ifndef  ____APPTEST_H
#define  ____APPTEST_H

#include "GameCommand.h"
namespace app
{
	class GameTest:public IGameBase
	{
	public:
		GameTest();
		~GameTest();
		virtual void  Init();
		virtual void  Update();
		virtual bool  ServerCommand(tcp::IServer* ts, tcp::S_CONNECT_BASE* c, const uint16_t cmd);
	};

	extern IGameBase* __Test;
	
}
#endif