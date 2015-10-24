// by csDev team

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"teamplay_gamerules.h"
#include	"destroy_gamerules.h"
#include	"game.h"
#include	"items.h"
#include	"voice_gamemgr.h"
#include	"hltv.h"
#include	"team_main.h"
#include	"func_break.h"

// 0 - None; 1 - Bomb; 2 - Destroyed
int BP1Status = 0;
int BP2Status = 0;

extern DLL_GLOBAL CGameRules	*g_pGameRules;
extern DLL_GLOBAL BOOL	g_fGameOver;
extern int gmsgDeathMsg;	// client dll messages
extern int gmsgScoreInfo;
extern int gmsgTeamMenu;
extern int gmsgGameMode;
extern int gmsgSayText;
extern int gmsgTeamNames;
extern int gmsgSpectator;
extern int gmsgServerName;
extern int gmsgDeleteAllDecals;
extern int gmsgPlaySpecSnd;
extern int gmsgTeamScores;
extern int gmsgStartTimer;
extern int gmsgKillTimer;
extern char *PistolList[3];
extern char *RifleList[9];
#define ITEM_RESPAWN_TIME	30
#define WEAPON_RESPAWN_TIME	20
#define AMMO_RESPAWN_TIME	20
#define MAX_INTERMISSION_TIME		120
extern cvar_t timeleft, fragsleft;
extern cvar_t mp_chattime;
extern float g_flIntermissionStartTime;
extern CVoiceGameMgr	g_VoiceGameMgr;

class CMultiplayGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pTalker)
	{
		if ( g_pGameRules->IsTeamplay() )
		{
			if ( g_pGameRules->PlayerRelationship( pListener, pTalker ) != GR_TEAMMATE )
			{
				return false;
			}
		}

		return true;
	}
};
static CMultiplayGameMgrHelper g_GameMgrHelper;

CHalfLifeDestroy :: CHalfLifeDestroy()
{
	m_DisableDeathMessages = false;
	FirstBlood = false;
	g_VoiceGameMgr.Init(&g_GameMgrHelper, gpGlobals->maxClients);

	RefreshSkillData();
	m_flIntermissionEndTime = 0;
	g_flIntermissionStartTime = 0;
	SERVER_COMMAND( "exec server.cfg\n" );

	TScores = CTScores = 0;
	ActStartRound();
}

void CHalfLifeDestroy :: InitHUD( CBasePlayer *pl )
{
	// notify other clients of player joining the game
	UTIL_ClientPrintAll( HUD_PRINTNOTIFY, UTIL_VarArgs( "%s has joined the game\n", 
		( pl->pev->netname && STRING(pl->pev->netname)[0] != 0 ) ? STRING(pl->pev->netname) : "unconnected" ) );

	UpdateGameMode( pl );

	MESSAGE_BEGIN( MSG_ONE, gmsgTeamScores, NULL, pl->edict() );
	WRITE_SHORT( TScores );
	WRITE_SHORT( CTScores );
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_ONE, gmsgStartTimer, NULL, pl->edict() );
	WRITE_COORD( RoundTime * 60 - ( gpGlobals->time - StartRoundTime ) );
	MESSAGE_END();

	// loop through all active players and send their score info to the new client
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		// FIXME:  Probably don't need to cast this just to read m_iDeaths
		CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );

		if ( plr )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgScoreInfo, NULL, pl->edict() );
			WRITE_BYTE( i );	// client number
			WRITE_SHORT( plr->pev->frags );
			WRITE_SHORT( plr->m_iDeaths );
			WRITE_SHORT( (plr->pev->iuser1 != 0) );
			WRITE_SHORT( plr->pev->team );
			MESSAGE_END();
		}
	}

	if ( g_fGameOver )
	{
		MESSAGE_BEGIN( MSG_ONE, SVC_INTERMISSION, NULL, pl->edict() );
		MESSAGE_END();
	}

	CLIENT_COMMAND( pl->edict(), "spectate\n" );
	
	MESSAGE_BEGIN( MSG_ONE, gmsgTeamNames, NULL, pl->edict() );  

		WRITE_BYTE( 3 );
		WRITE_STRING( "Terrorists" );
		WRITE_STRING( "Counter-Terrorists" );
		WRITE_STRING( "Spectator" );

	MESSAGE_END();

	MESSAGE_BEGIN( MSG_ONE, gmsgTeamMenu, NULL, pl->edict() );  
	MESSAGE_END();
}

void CHalfLifeDestroy :: Think ( void )
{
	if( gamemode.value != 3 )
		SERVER_COMMAND( "restart\n" );

	ActionThink();

	g_VoiceGameMgr.Update(gpGlobals->frametime);

	///// Check game rules /////
	static int last_frags;
	static int last_time;

	int frags_remaining = 0;
	int time_remaining = 0;

	if ( g_fGameOver )   // someone else quit the game already
	{
		// bounds check
		int time = (int)CVAR_GET_FLOAT( "mp_chattime" );
		if ( time < 1 )
			CVAR_SET_STRING( "mp_chattime", "1" );
		else if ( time > MAX_INTERMISSION_TIME )
			CVAR_SET_STRING( "mp_chattime", UTIL_dtos1( MAX_INTERMISSION_TIME ) );

		m_flIntermissionEndTime = g_flIntermissionStartTime + mp_chattime.value;

		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->time )
		{
			if ( m_iEndIntermissionButtonHit  // check that someone has pressed a key, or the max intermission time is over
				|| ( ( g_flIntermissionStartTime + MAX_INTERMISSION_TIME ) < gpGlobals->time) ) 
				ChangeLevel(); // intermission is over
		}

		return;
	}

	float flTimeLimit = timelimit.value * 60;
	float flFragLimit = fraglimit.value;

	time_remaining = (int)(flTimeLimit ? ( flTimeLimit - gpGlobals->time ) : 0);

	if ( flTimeLimit != 0 && gpGlobals->time >= flTimeLimit )
	{
		GoToIntermission();
		return;
	}

	if ( flFragLimit )
	{
		int bestfrags = 9999;
		int remain;

		// check if any player is over the frag limit
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

			if ( pPlayer && pPlayer->pev->frags >= flFragLimit )
			{
				GoToIntermission();
				return;
			}


			if ( pPlayer )
			{
				remain = flFragLimit - pPlayer->pev->frags;
				if ( remain < bestfrags )
				{
					bestfrags = remain;
				}
			}

		}
		frags_remaining = bestfrags;
	}

	// Updates when frags change
	if ( frags_remaining != last_frags )
	{
		g_engfuncs.pfnCvar_DirectSet( &fragsleft, UTIL_VarArgs( "%i", frags_remaining ) );
	}

	// Updates once per second
	if ( timeleft.value != last_time )
	{
		g_engfuncs.pfnCvar_DirectSet( &timeleft, UTIL_VarArgs( "%i", time_remaining ) );
	}

	last_frags = frags_remaining;
	last_time  = time_remaining;
}

BOOL CHalfLifeDestroy::IsDestroy()
{
	return TRUE;
}


void CHalfLifeDestroy :: PlayerSpawn( CBasePlayer *pPlayer )
{
	MESSAGE_BEGIN( MSG_ONE, gmsgStartTimer, NULL, pPlayer->edict() );
	WRITE_COORD( RoundTime * 60 - ( gpGlobals->time - StartRoundTime ) );
	MESSAGE_END();

	memset( pPlayer->Slots, 0, sizeof( pPlayer->Slots ) );
	CBaseEntity	*pWeaponEntity = NULL;

	pPlayer->pev->weapons |= (1<<WEAPON_SUIT);

	bool AddDefault = true;


	while ( pWeaponEntity = UTIL_FindEntityByClassname( pWeaponEntity, "game_player_equip" ) )
	{
		pWeaponEntity->Touch( pPlayer );
		AddDefault = false;
	}

	if( AddDefault )
	{
		if( mp_buymode.value == 1 || mp_buymode.value == 0 )
		{
			pPlayer->GiveNamedItem( "weapon_knife" );
			if( pPlayer->pev->team == 1 )
				pPlayer->GiveNamedItem( "weapon_glock18" );
			else
				pPlayer->GiveNamedItem( "weapon_usp" );
		}
		else if( mp_buymode.value == 2 )
		{
			pPlayer->GiveNamedItem( PistolList[ RANDOM_LONG( 0, 2 ) ] );
			pPlayer->GiveNamedItem( RifleList[ RANDOM_LONG( 0, 8 ) ] );

			pPlayer->GiveNamedItem( "weapon_knife" );
			pPlayer->GiveNamedItem( "weapon_hegrenade" );
			pPlayer->GiveNamedItem( "weapon_smokegrenade" );
			pPlayer->GiveNamedItem( "weapon_flashbang" );
			pPlayer->GiveNamedItem( "weapon_flashbang" );
		}
		if( pPlayer->pev->team == 1 )
			pPlayer->GiveNamedItem( "weapon_c4" );
	}
	
}

float CHalfLifeDestroy :: FlWeaponTryRespawn( CBasePlayerItem *pWeapon )
{
	if( gpGlobals->time + WEAPON_RESPAWN_TIME > StartRoundTime + RoundTime * 60 + 5 )
		return StartRoundTime + RoundTime * 60 + 5;
	return gpGlobals->time + WEAPON_RESPAWN_TIME;
}




void CHalfLifeDestroy :: SetNextAction( int Action, float Time )
{
	NextAction = Action;
	TimeNextAction = Time;
}

void CHalfLifeDestroy :: ActionThink()
{
	if( BP1Status == 2 && BP2Status == 2 )
		SetNextAction( END_ROUND, gpGlobals->time );
	if( TimeNextAction > 0 && TimeNextAction <= gpGlobals->time )
	{
		TimeNextAction = 0;
		if( NextAction == START_ROUND )
			ActStartRound();
		else if( NextAction == END_ROUND )
			ActEndRound();
		else if( NextAction == RESPAWN )
			ActRespawn();
	}
}

void CHalfLifeDestroy :: ActStartRound()
{
	BP1Status = BP2Status = 0;
	RoundTime = mp_roundtime.value;
	StartRoundTime = gpGlobals->time;
	SetNextAction( END_ROUND, RoundTime * 60 + gpGlobals->time );
	MESSAGE_BEGIN( MSG_ALL, gmsgStartTimer );
	WRITE_COORD( RoundTime * 60 );
	MESSAGE_END();
}

void CHalfLifeDestroy :: ActEndRound()
{
	SetNextAction( RESPAWN, 5 + gpGlobals->time );
	if( BP1Status == 2 && BP2Status == 2 )
	{
		TScores++;
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "#Twin" );
		MESSAGE_BEGIN( MSG_ALL, gmsgPlaySpecSnd );
		WRITE_STRING( "radio/terwin.wav" );
		WRITE_COORD( 1 );
		MESSAGE_END();
	}
	else
	{
		CTScores++;
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "#CTWin" );
		MESSAGE_BEGIN( MSG_ALL, gmsgPlaySpecSnd );
		WRITE_STRING( "radio/ctwin.wav" );
		WRITE_COORD( 1 );
		MESSAGE_END();
	}
	BP1Status = BP2Status = 0;
	MESSAGE_BEGIN( MSG_ALL, gmsgTeamScores );
	WRITE_SHORT( TScores );
	WRITE_SHORT( CTScores );
	MESSAGE_END();
}

void CHalfLifeDestroy :: ActRespawn()
{
	DeleteAllDecals();
	ResetAllEntity();
	RespawnPlayers();

	SetNextAction( START_ROUND, gpGlobals->time );
}

void CHalfLifeDestroy :: DeleteAllDecals()
{
	MESSAGE_BEGIN( MSG_ALL, gmsgDeleteAllDecals );
	MESSAGE_END();
}

void CHalfLifeDestroy :: ResetAllEntity()
{
	CBaseEntity* entity = NULL;
	entity = NULL;
	while ((entity = UTIL_FindEntityByClassname(entity, "grenade"))!= NULL)
		UTIL_Remove( entity );

	entity = NULL;
	while ((entity = UTIL_FindEntityByClassname(entity, "monster_c4"))!= NULL)
		UTIL_Remove( entity );

	entity = NULL;
	while ((entity = UTIL_FindEntityByClassname(entity, "weaponbox"))!= NULL)
		UTIL_Remove( entity );
	
	CBreakable *pBreakable = NULL;
	while ((pBreakable = (CBreakable*)UTIL_FindEntityByClassname( (CBaseEntity*)pBreakable, "func_breakable" )) != NULL)
		pBreakable->Restart();
}


void CHalfLifeDestroy :: SetTeamScores( int T, int CT )
{
	TScores = T;
	CTScores = CT;

	MESSAGE_BEGIN( MSG_ALL, gmsgTeamScores );
	WRITE_SHORT( TScores );
	WRITE_SHORT( CTScores );
	MESSAGE_END();
}


void CHalfLifeDestroy :: RespawnPlayers()
{
	for( int i=1; i < 32; i++ )
	{
		CBasePlayer *pEnt = ( CBasePlayer * )UTIL_PlayerByIndex( i );
		if(pEnt)
		{
			if( !(pEnt->pev->deadflag & DEAD_DEAD) )
			{
				edict_t *pentSpawnSpot = EntSelectSpawnPoint( pEnt );
				pEnt->pev->origin = VARS( pentSpawnSpot )->origin;
				pEnt->pev->angles = VARS( pentSpawnSpot )->angles;
				pEnt->pev->velocity = Vector( 0, 0, 0 );
				pEnt->pev->health = 100;
			}
			else
				pEnt->Spawn();
		}
	}
}