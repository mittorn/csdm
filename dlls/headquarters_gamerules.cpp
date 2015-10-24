#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"teamplay_gamerules.h"
#include	"headquarters_gamerules.h"
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


CHeadQuartersRules :: CHeadQuartersRules()
{
	m_DisableDeathMessages = false;
	FirstBlood = false;
	g_VoiceGameMgr.Init(&g_GameMgrHelper, gpGlobals->maxClients);

	RefreshSkillData();
	m_flIntermissionEndTime = 0;
	g_flIntermissionStartTime = 0;
	State = STATE_IDLE;
	CurrentBase = LastBase = NULL;
	LastCurrentTeam = -1;
	SERVER_COMMAND( "exec server.cfg\n" );
}


void CHeadQuartersRules :: Think ( void )
{	
	if( gamemode.value != 4 )
		SERVER_COMMAND( "restart\n" );

	HeadQThink();

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

BOOL CHeadQuartersRules :: IsHeadQuarters( void )
{
	return TRUE;
}

void CHeadQuartersRules :: HeadQThink( void )
{
	if( !CurrentBase )
		SpawnNextBase();

	if( SpawnTime + 20 > gpGlobals->time )
	{
		State = STATE_NONACTIVE;
		return;
	}

	if( State == STATE_NONACTIVE )
	{
		State = STATE_IDLE;
		UTIL_ClientPrintAll( HUD_PRINTCENTER, "Capture base!" );
	}

	bool IDLE = true;

	if( State == STATE_IDLE )
	{
		for( int i = 1; i <= 32; i++ )
		{
			CBaseEntity *pEnt = UTIL_PlayerByIndex( i );
			if( pEnt && Vector( pEnt->pev->origin - CurrentBase->pev->origin ).Length() < 256 && pEnt->pev->team )
			{
				State = STATE_CAPTURE;
				StartTime = gpGlobals->time;
				EndTime = StartTime + 15;
				IDLE = false;
			}
		}
	}

	else if( State == STATE_CAPTURE )
	{
		int CTCount = 0, TCount = 0;
		for( int i = 1; i <= 32; i++ )
		{
			CBaseEntity *pEnt = UTIL_PlayerByIndex( i );
			if( pEnt && Vector( pEnt->pev->origin - CurrentBase->pev->origin ).Length() < 256 && pEnt->pev->team )
			{
				if( pEnt->pev->team == 1 )
					TCount++;
				else if( pEnt->pev->team == 2 )
					CTCount++;
				IDLE = false;
			}
		}

		if( TCount ==  0 && CTCount == 0 )
			State = STATE_IDLE;

		if( !TCount && CTCount )
			CurrentTeam = 2;
		else if( TCount && !CTCount )
			CurrentTeam = 1;
		else
			CurrentTeam = 0;

		if( !CurrentTeam )
		{
			EndTime += gpGlobals->time - LFT;
			StartTime += gpGlobals->time - LFT;
		}
		else if( CurrentTeam != LastCurrentTeam )
		{
			int Count = abs( TCount - CTCount );
			StartTime = gpGlobals->time;
			EndTime = StartTime + 15 / Count;
		}
		else
		{
			int Count = abs( TCount - CTCount );
			EndTime = StartTime + 15 / Count;
		}

		if( CurrentTeam )
			LastCurrentTeam = CurrentTeam;

		char MSG[256];
		sprintf( MSG, "%d %f %f %f", State, StartTime, gpGlobals->time, EndTime );
		UTIL_ClientPrintAll( HUD_PRINTCENTER, MSG );

		if( EndTime <= gpGlobals->time )
		{
			State = STATE_DEFENDING;
			StartTime = gpGlobals->time;
			EndTime = StartTime + 90;
			DefBaseCap = false;
		}
	}

	else if( State == STATE_DEFENDING )
	{
		if( CurrentTeam == 1 && !T_Alive() )
		{
			State = STATE_IDLE;
			SpawnNextBase();
			return;
		}
		else if( CurrentTeam == 2 && !CT_Alive() )
		{
			State = STATE_IDLE;
			SpawnNextBase();
			return;
		}

		bool f = false;
		for( int i = 1; i <= 32; i++ )
		{
			int Count = 0;
			bool StopDef = false;
			CBaseEntity *pEnt = UTIL_PlayerByIndex( i );
			if( pEnt && Vector( pEnt->pev->origin - CurrentBase->pev->origin ).Length() < 256 && pEnt->pev->team )
			{
				if( pEnt->pev->team == CurrentTeam )
					StopDef = true;
				else
				{
					f = true;
					Count++;
				}
			}

			if( f )
			{
				if( !DefBaseCap )
				{
					StartTime2 = gpGlobals->time;
					EndTime2 = gpGlobals->time + 15;
					DefBaseCap = true;
				}
			}
			else
			{
				DefBaseCap = false;
			}

			if( DefBaseCap )
			{
				if( StopDef )
				{
					EndTime += gpGlobals->time - LFT;
					StartTime += gpGlobals->time - LFT;
				}
				EndTime2 = StartTime2 + 15 / Count;
				if( gpGlobals->time >= EndTime2 )
				{
					SpawnNextBase();
					State = STATE_IDLE;
				}
			}
		}
		IDLE = false;
		char MSG[256];
		sprintf( MSG, "%d %f %f %f", State, StartTime, gpGlobals->time, EndTime );
		UTIL_ClientPrintAll( HUD_PRINTCENTER, MSG );
		if( EndTime <= gpGlobals->time )
		{
			SpawnNextBase();
			State = STATE_IDLE;
		}
	}

	if( IDLE )
		State = STATE_IDLE;

	LFT = gpGlobals->time;
}


void CHeadQuartersRules :: SpawnNextBase( void )
{
	if( CurrentBase )
		CurrentBase->pev->effects |= EF_NODRAW;
	CurrentBase = UTIL_FindEntityByClassname( CurrentBase, "func_headq" );
	if( !CurrentBase )
		CurrentBase = UTIL_FindEntityByClassname( CurrentBase, "func_headq" );
	CurrentBase->pev->effects &= ~EF_NODRAW;
	SpawnTime = gpGlobals->time;
}