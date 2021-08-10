#ifndef  __GAMEDATA_H__
#define  __GAMEDATA_H__

#include <cstring>
#include "IData.h"


#define CMD_LOGIN      1000
#define CMD_MOVE       2000
#define CMD_PLAYERDATA 3000
#define CMD_LEAVE      4000


namespace app
{
#pragma pack(push,packing)
#pragma pack(1)
	struct S_VECTOR
	{
		float x;
		float y;
		float z;
	};
	struct  S_PLAYER_BASE
	{
		int32_t  memid;
		int32_t  socketfd;
		int32_t  state;
		int32_t  curhp;
		int32_t  maxhp;
		float  speed;
		S_VECTOR   pos;
		S_VECTOR   rot;
		inline void init()
		{
			memset(this, 0, sizeof(S_PLAYER_BASE));
		}
	};

	struct  S_PLAYER_MOVE
	{
		int32_t memid;
		float   speed;
		S_VECTOR pos;
		S_VECTOR rot;
	};
	struct  S_PLAYER_MEMID
	{
		int32_t memid;
	};

#pragma pack(pop, packing)

}
#endif