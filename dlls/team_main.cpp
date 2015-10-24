/*
Counter-Strike player
+team functions
*/

// Спасибо, HAWK

#include "extdll.h"
#include "util.h"

#include "cbase.h"
#include "player.h"
#include "trains.h"
#include "nodes.h"
#include "weapons.h"
#include "soundent.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"
#include "game.h"
#include "hltv.h"

int All_Count()
{
	int iRet = 0;

	for(int i=1;i<33;i++)
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex(i);

		if(pEnt)
			iRet++;

	}

	return iRet;
}

int T_Count()
{
	int iRet = 0;

	for(int i=1;i<33;i++)
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex(i);

		if(pEnt && pEnt->pev->team == 1)
			iRet++;

	}

	return iRet;
}

int T_Alive()
{
	int iRet = 0;

	for(int i=1;i<33;i++)
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex(i);

		if(pEnt && pEnt->pev->team == 1 && pEnt->IsAlive())
			iRet++;

	}

	return iRet;
}

int CT_Count()
{
	int iRet = 0;

	for(int i=1;i<33;i++)
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex(i);

		if(pEnt && pEnt->pev->team == 2)
			iRet++;

	}

	return iRet;
}

int CT_Alive()
{
	int iRet = 0;

	for(int i=1;i<33;i++)
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex(i);

		if(pEnt && pEnt->pev->team == 2 && pEnt->IsAlive())
			iRet++;

	}

	return iRet;
}

