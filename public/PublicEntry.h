#ifndef  __PUBLICENTRY_H__
#define  __PUBLICENTRY_H__

#include "stdint.h"
#pragma pack(push,packing)
#pragma pack(1)
struct  S_TEST_BASE
{
	uint32_t   id;
	uint32_t   curtime;
	uint8_t    aa[1024];
};
#pragma pack(pop, packing)
namespace entry
{
	extern bool Init();
}

#endif