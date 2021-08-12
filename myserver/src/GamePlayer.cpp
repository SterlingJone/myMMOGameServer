#include "GamePlayer.h"
#include "GameCommand.h"
#include "PublicEntry.h"

#include <list>
#include <time.h>


namespace app
{
	IGameBase* __Player = NULL;
	std::map<int, S_PLAYER_BASE*>  __Onlines;//玩家登陆在线数据
	std::list<S_PLAYER_BASE*>  __PlayersPool;//对象回收池
	int  temptime = 0;
	int  __GlobalMemid = 10000000;
	
	GamePlayer::GamePlayer()
	{
	}

    GamePlayer::~GamePlayer()
	{
	}

	void GamePlayer::Init()
	{
	}

	void playerLeave(int32_t memid)
	{
		auto it = __Onlines.begin();
		while (it != __Onlines.end())
		{
			auto player = it->second;
			auto c = __IServer->FindClient(player->socketfd, true);
			if (c == nullptr || player->memid == memid)
			{
				++it;
				continue;
			}
			S_PLAYER_MEMID mem;
			mem.memid = memid;
			__IServer->CreatePackage(c->index, CMD_LEAVE, &mem, sizeof(S_PLAYER_MEMID));
			
			++it;
		}
	}
	
	void GamePlayer::Update()
	{
		int value = clock() - temptime;
		if (value < 100) return;
		temptime = clock();

		auto it = __Onlines.begin();
		while (it != __Onlines.end())
		{
			auto player = it->second;
			auto c = __IServer->FindClient(player->socketfd, true);
			if (c == nullptr)
			{
				++it;
				continue;
			}
			if (c->state == common::E_SSS_NeedSave)
			{

				LOGINFO("GamePlayer leave....%d line:%d \n", player->memid, __LINE__);
				playerLeave(player->memid);
				it = __Onlines.erase(it);
				player->init();
				__PlayersPool.push_back(player);

				c->Reset();
				continue;
			}

			++it;
		}
	}

	bool app::GamePlayer::ServerCommand(tcp::IServer* ts, tcp::S_CONNECT_BASE* c, const uint16_t cmd)
	{
		if (ts->IsCloseClient(c->index, common::E_SSS_Secure))
		{
			LOGINFO("GamePlayer err....line:%d \n", __LINE__);
			return false;
		}

		switch (cmd)
		{
			case CMD_LOGIN:
			{
				onLogin(ts, c);
				break;
			}
			case CMD_MOVE:
			{
				onMove(ts, c);
				break;
			}
			case CMD_PLAYERDATA:
			{
				onGetPlayerData(ts, c);
				break;
			}
			return false;
		}
	}

	S_PLAYER_BASE* findPlayer(int32_t memid, tcp::S_CONNECT_BASE* c)
	{
		auto it = __Onlines.find(memid);
		if (it == __Onlines.end())
		{
			return NULL;
		}
		auto player = it->second;
		if (player->socketfd != c->socketfd)
		{
			LOGINFO("findplayer err...%d-%d-%d-%d \n", memid, c->index, player->socketfd, c->socketfd);
			return NULL;
		}
		return player;
	}
	//1000 登陆
	void app::GamePlayer::onLogin(tcp::IServer* ts, tcp::S_CONNECT_BASE* c)
	{
		if (c->state == common::E_SSS_Login)
		{
			LOGINFO("onlogin is login....\n");
			return;
		}

		S_PLAYER_BASE* player = NULL;
		if (__PlayersPool.empty())
		{
			player = new S_PLAYER_BASE();
			player->init();
		}
		else
		{
			player = __PlayersPool.front();
			__PlayersPool.pop_front();
			player->init();
		}
		srand(time(NULL));

		c->state = common::E_SSS_Login;
		player->memid = __GlobalMemid;
		player->socketfd = c->socketfd;
		player->curhp = 3000;
		player->maxhp = 6000;
		__GlobalMemid++;
		__Onlines.insert(std::make_pair(player->memid, player));

		ts->CreatePackage(c->index, CMD_LOGIN, player, sizeof(S_PLAYER_BASE));

		
		LOGINFO("GamePlayer login ...%d-%d\n", player->memid, (int)c->socketfd);
	}
	//2000 玩家移动
	void app::GamePlayer::onMove(tcp::IServer* ts, tcp::S_CONNECT_BASE* c)
	{
		S_PLAYER_MOVE move;
		ts->ReadPackage(c->index,&move,sizeof(S_PLAYER_MOVE));
	
		auto player = findPlayer(move.memid,c);
		if (player == NULL) return;

		player->speed = move.speed;
		player->pos = move.pos;
		player->rot = move.rot;

		auto it = __Onlines.begin();
		while (it != __Onlines.end())
		{
			auto other = it->second;
			auto c2 = ts->FindClient(other->socketfd, true);
			if (c2 == nullptr || other->memid == move.memid)
			{
				++it;
				continue;
			}

			ts->CreatePackage(c2->index, CMD_MOVE, &move, sizeof(S_PLAYER_MOVE));
			++it;
		}
	}
	//3000 获取其他玩家数据
	void GamePlayer::onGetPlayerData(tcp::IServer* ts, tcp::S_CONNECT_BASE* c)
	{
		S_PLAYER_MEMID  info;
		ts->ReadPackage(c->index, &info, sizeof(S_PLAYER_MEMID));
		auto it = __Onlines.find(info.memid);
		if (it == __Onlines.end())
		{
			return;
		}

		auto player = it->second;
		auto c2 = ts->FindClient(player->socketfd, true);
		if (c2 == NULL)
		{
			return;
		}

		ts->CreatePackage(c->index, CMD_PLAYERDATA, player, sizeof(S_PLAYER_BASE));

	}


}
