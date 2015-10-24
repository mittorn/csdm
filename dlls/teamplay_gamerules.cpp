// by csDev team

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"teamplay_gamerules.h"
#include	"game.h"
#include	"items.h"
#include	"voice_gamemgr.h"
#include	"hltv.h"
#include	"team_main.h"

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

CHalfLifeTeamplay :: CHalfLifeTeamplay()
{
	m_DisableDeathMessages = false;
	FirstBlood = false;
	g_VoiceGameMgr.Init(&g_GameMgrHelper, gpGlobals->maxClients);

	RefreshSkillData();
	m_flIntermissionEndTime = 0;
	g_flIntermissionStartTime = 0;
	SERVER_COMMAND( "exec server.cfg\n" );
}

BOOL CHalfLifeTeamplay::IsTeamplay()
{
	return TRUE;
}

void CHalfLifeTeamplay :: Think ( void )
{	
	if( gamemode.value != 2 )
		SERVER_COMMAND( "restart\n" );

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

int CHalfLifeTeamplay::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	if( pPlayer->pev->team == pTarget->pev->team )
		return GR_TEAMMATE;
	return GR_NOTTEAMMATE;
}

BOOL CHalfLifeTeamplay :: ClientCommand( CBasePlayer *pPlayer, const char *pcmd )
{
	if(g_VoiceGameMgr.ClientCommand(pPlayer, pcmd))
		return TRUE;

	else if ( FStrEq( pcmd, "jointeam" ) )
	{
		int teamindex = atoi( CMD_ARGV( 1 ) );

		if( teamindex == 1 )
			ChangePlayerTeam( pPlayer, 1 );

		else if( teamindex == 2 )
			ChangePlayerTeam( pPlayer, 2 );

		else if( teamindex == 3 )
			CLIENT_COMMAND( pPlayer->edict(), "spectate\n" );

		
		else
		{
			int CountT = T_Count();
			int CountCT = CT_Count();

			if( CountT < CountCT )
				CLIENT_COMMAND( pPlayer->edict(), "jointeam 1\n" );
			else
				CLIENT_COMMAND( pPlayer->edict(), "jointeam 2\n" );
		}

		//pPlayer->pev->team = teamindex;

		return 1;
	}

	else if ( FStrEq( pcmd, "teammenu" ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgTeamMenu, NULL, pPlayer->edict() );  
		MESSAGE_END();
	}

	return FALSE;
}

void CHalfLifeTeamplay :: InitHUD( CBasePlayer *pl )
{
	// notify other clients of player joining the game
	UTIL_ClientPrintAll( HUD_PRINTNOTIFY, UTIL_VarArgs( "%s has joined the game\n", 
		( pl->pev->netname && STRING(pl->pev->netname)[0] != 0 ) ? STRING(pl->pev->netname) : "unconnected" ) );

	UpdateGameMode( pl );

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


void CHalfLifeTeamplay :: ChangePlayerTeam( CBasePlayer *pPlayer, int Index )
{
	if( pPlayer->pev->team == Index )
		return;
	pPlayer->pev->team = Index;
	pPlayer->StopObserver();

	if( pPlayer->IsAlive() && !pPlayer->pev->iuser1 )
	{
		m_DisableDeathMessages = true;
		entvars_t *pevWorld = VARS( INDEXENT(0) );
		pPlayer->TakeDamage( pevWorld, pevWorld, 900, DMG_GENERIC | DMG_NEVERGIB );
		pPlayer->m_iDeaths--;
		m_DisableDeathMessages = false;
	}

	if( Index == 1 )
		CLIENT_COMMAND( pPlayer->edict(), "model Terrorists\n" );
	else
		CLIENT_COMMAND( pPlayer->edict(), "model Counter-Terrorists\n" );

	MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
		WRITE_BYTE( pPlayer->entindex() );
		WRITE_SHORT( pPlayer->pev->frags );
		WRITE_SHORT( pPlayer->m_iDeaths );
		WRITE_SHORT( 0 );
		WRITE_SHORT( Index );
	MESSAGE_END();
}


void CHalfLifeTeamplay::DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pevInflictor )
{
	if ( m_DisableDeathMessages )
		return;
	CHalfLifeMultiplay::DeathNotice( pVictim, pKiller, pevInflictor );
}


BOOL CHalfLifeTeamplay::FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker )
{
	if ( pAttacker && PlayerRelationship( pPlayer, pAttacker ) == GR_TEAMMATE )
	{
		// my teammate hit me.
		if ( (friendlyfire.value == 0) && (pAttacker != pPlayer) )
		{
			// friendly fire is off, and this hit came from someone other than myself,  then don't get hurt
			return FALSE;
		}
	}

	return CHalfLifeMultiplay::FPlayerCanTakeDamage( pPlayer, pAttacker );
}