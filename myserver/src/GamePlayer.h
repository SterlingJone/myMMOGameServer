#ifndef  __GAMELAYER_H__
#define  __GAMELAYER_H__


#include "GameData.h"
#include <map>

namespace app
{

	class GamePlayer :public IGameBase
	{
	public:
		GamePlayer();
		~GamePlayer();
		virtual void  Init();
		virtual void  Update();
		virtual bool  ServerCommand(tcp::IServer* ts, tcp::S_CONNECT_BASE* c, const uint16_t cmd);

		void onLogin(tcp::IServer* ts, tcp::S_CONNECT_BASE* c);
		void onMove(tcp::IServer* ts, tcp::S_CONNECT_BASE* c);
		void onGetPlayerData(tcp::IServer* ts, tcp::S_CONNECT_BASE* c);
	};

	extern IGameBase* __Player;
	extern std::map<int, S_PLAYER_BASE*>  __Onlines;

}

#endif