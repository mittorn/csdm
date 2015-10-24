#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

extern int gmsgNewExplode;
extern int gmsgPlaySpecSnd;
extern int gmsgNewPrgBar;
extern int gmsgKillPrgBar;
extern int BP1Status;
extern int BP2Status;

class CMonsterC4 : public CGrenade
{
public:
	void Spawn( void );
	void Precache( void ) { };
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() | FCAP_CONTINUOUS_USE; }
	void C4Think();
	void Exp();
	float TimeExp;
	int IndexBP;
	bool Defuse;
	CBasePlayer *Defuser;
	float TimeStartDefuse;
};

LINK_ENTITY_TO_CLASS( monster_c4, CMonsterC4 );

void CMonsterC4 :: Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( pActivator->pev->team == 1 )
	{
		ClientPrint( pActivator->pev, HUD_PRINTCENTER, "#TerroristDefuse" );
		return;
	}
	if( Defuser && pActivator != Defuser )
	{
		ClientPrint( pActivator->pev, HUD_PRINTCENTER, "#AlreadyDefuse" );
		return;
	}
	if( Defuser )
		return;
	Defuse = true;
	Defuser = ( CBasePlayer * )pActivator;
	TimeStartDefuse = gpGlobals->time;

	MESSAGE_BEGIN( MSG_ONE, gmsgNewPrgBar, NULL, Defuser->pev );
	WRITE_BYTE( 10 );
	MESSAGE_END();
}

void CMonsterC4 :: Spawn( void )
{
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;

	SET_MODEL(ENT(pev), "models/weapons/c4/w_c4.mdl");
	UTIL_SetSize(pev, Vector( -8, -8, -8), Vector(8, 8, 8));
	UTIL_SetOrigin( pev, pev->origin );
	Defuse = false;
	Defuser = NULL;

	pev->takedamage = DAMAGE_NO;
	pev->dmg = 999;
	pev->health = 1;
	TimeExp = gpGlobals->time + 30;
	SetThink( &CMonsterC4::C4Think );
	pev->nextthink = gpGlobals->time + 0.01;

	MESSAGE_BEGIN( MSG_ALL, gmsgPlaySpecSnd );
	WRITE_STRING( "radio/bombpl.wav" );
	WRITE_COORD( 1 );
	MESSAGE_END();

}

#define N	35
void CMonsterC4 :: C4Think()
{
	float TimeToExp = TimeExp - gpGlobals->time;
	if( TimeToExp <= 0 )
	{
		if( Defuse )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgKillPrgBar, NULL, Defuser->pev );
			MESSAGE_END();
		}

		Exp();
		return;
	}
	float BipTime[ N ] = { 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7,
	7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 24, 26, 28, 30 };

	for( int i = 0; i < N; i++ )
		if( fabs( TimeToExp - BipTime[ i ] ) <= 0.03 )
			EMIT_SOUND( ENT(pev), CHAN_BODY, "weapons/c4_click.wav", 1, 0.3 );

	if( Defuse && !(Defuser->m_afButtonLast & IN_USE) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgKillPrgBar, NULL, Defuser->pev );
		MESSAGE_END();

		Defuse = false;
		Defuser = NULL;
	}

	if( Defuse )
	{
		Vector DefAngles = Defuser->pev->angles;

		if( DefAngles.y > 360 )
			DefAngles.y -= 360;
		if( DefAngles < 0 )
			DefAngles.y += 360;

		Vector Angles =  UTIL_VecToAngles( pev->origin - Defuser->pev->origin ) - DefAngles;
		float Length = Vector( pev->origin - Defuser->pev->origin ).Length();

		if( Angles.y > 360 )
			Angles.y -= 360;
		if( Angles < 0 )
			Angles.y += 360;

		if( !( ( Angles.y <= 45 || Angles.y >= 315 ) && Length <= 64 ) )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgKillPrgBar, NULL, Defuser->pev );
			MESSAGE_END();

			Defuse = false;
			Defuser = NULL;
		}
	}

	if( Defuse && TimeStartDefuse + 10 <= gpGlobals->time )
	{
		Defuse = false;
		Defuser = NULL;

		if( IndexBP == 1 )
			BP1Status = 0;
		else
			BP2Status = 0;

		pev->effects |= EF_NODRAW;
		SetThink( NULL );
		UTIL_Remove( this );

		MESSAGE_BEGIN( MSG_ALL, gmsgPlaySpecSnd );
		WRITE_STRING( "radio/bombdef.wav" );
		WRITE_COORD( 1 );
		MESSAGE_END();

		return;
	}

	SetThink( &CMonsterC4::C4Think );
	pev->nextthink = gpGlobals->time + 0.01;
}

void CMonsterC4::Exp()
{
	if( IndexBP == 1 )
		BP1Status = 2;
	else if( IndexBP == 2 )
		BP2Status = 2;
	float		flRndSound;// sound randomizer

	pev->model = iStringNull;//invisible
	pev->solid = SOLID_NOT;// intangible

	pev->takedamage = DAMAGE_NO;

	int iContents = UTIL_PointContents ( pev->origin );
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_EXPLOSION );		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_SHORT( 0 );
		WRITE_BYTE( (pev->dmg - 50) * .60  ); // scale * 10
		WRITE_BYTE( 15  ); // framerate
		WRITE_BYTE( TE_EXPLFLAG_NONE );
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );

		WRITE_BYTE( TE_DLIGHT );

		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_BYTE( 50 );

		WRITE_BYTE( 210 );
		WRITE_BYTE( 200 );
		WRITE_BYTE( 60 );

		WRITE_BYTE( 1 );
		WRITE_BYTE( 1 );

	MESSAGE_END();

	FireBulletsWater( pev->origin + Vector( 0, 0, 120 ), pev->origin, 1, 1 );

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );
	entvars_t *pevOwner;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	pev->owner = NULL; // can't traceline attack owner if this is set

	RadiusDamage ( pev, pevOwner, pev->dmg, CLASS_NONE, DMG_BLAST );

	if (iContents != CONTENTS_WATER)
	{
		MESSAGE_BEGIN( MSG_ALL, gmsgNewExplode );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( 6 );
		MESSAGE_END();

		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_SPARKS );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z -= 5 );
		MESSAGE_END();
	}



	flRndSound = RANDOM_FLOAT( 0 , 1 );

	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM);	break;
		case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM);	break;
		case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM);	break;
	}

	pev->effects |= EF_NODRAW;
	UTIL_Remove( this );
}

LINK_ENTITY_TO_CLASS( weapon_c4, CC4 );


enum c4_e 
{
    C4_IDLE1 = 0,
    C4_DRAW,
    C4_DROP,
    C4_PRESS_BUTTON
};


void CC4::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_c4" );
	Precache( );
	SET_MODEL( ENT(pev), "models/weapons/c4/w_c4.mdl" );
	m_iId = WEAPON_C4;
	m_iDefaultAmmo = 1;
	FallInit();
	ActivateBomb = false;
}

void CC4::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/c4/v_c4.mdl" );
	PRECACHE_MODEL( "models/weapons/c4/p_c4.mdl" );
	PRECACHE_MODEL( "models/weapons/c4/w_c4.mdl" );
	PRECACHE_SOUND( "weapons/c4_click.wav" );
	UTIL_PrecacheOther( "monster_c4" );

	m_event = PRECACHE_EVENT( 1, "events/c4.sc" );
}

int CC4::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "C4Ammo";
	p->iMaxAmmo1 = 1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 4;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_C4;
	p->iWeight = 3;

	return 1;
}

int CC4::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
		WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CC4::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 221 );
	return DefaultDeploy( "models/weapons/c4/v_c4.mdl", "models/weapons/c4/p_c4.mdl", C4_DRAW, "c4" );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
	SetThink( NULL );
}

void CC4::Holster( int skiplocal )
{
	if( !(m_pPlayer->m_afButtonLast & IN_USE) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgKillPrgBar, NULL, m_pPlayer->pev );
		MESSAGE_END();
	}

	SetThink( NULL );
}

void CC4::PrimaryAttack()
{
	
	if( ( m_pPlayer->OnBombPlant1 && BP1Status ) || ( m_pPlayer->OnBombPlant2 && BP2Status ) )
		return;
	
	if( !( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;

	
	ActivateBomb = true;

	MESSAGE_BEGIN( MSG_ONE, gmsgNewPrgBar, NULL, m_pPlayer->pev );
	WRITE_BYTE( 4 );
	MESSAGE_END();

	SendWeaponAnim( C4_PRESS_BUTTON );
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	SetThink( &CC4::DropingBomb );

	pev->nextthink = gpGlobals->time + 3;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
}

void CC4::DropingBomb()
{
	SendWeaponAnim( C4_DROP );
	
	SetThink( &CC4::DropBomb );
	pev->nextthink = gpGlobals->time + 1;
}

void CC4::DropBomb()
{
	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = Vector( 0, 0, -1 );

	TraceResult tr;

	UTIL_TraceLine( vecSrc, vecSrc + vecAiming * 1128, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	if (tr.flFraction < 1.0)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		if ( pEntity && !(pEntity->pev->flags & FL_CONVEYOR) )
		{
			CMonsterC4 *pEnt = ( CMonsterC4 * )CBaseEntity::Create( "monster_c4", tr.vecEndPos, Vector( 0, 0, 0 ), m_pPlayer->edict() );
			if( m_pPlayer->OnBombPlant1 )
			{
				BP1Status = 1;
				pEnt->IndexBP = 1;
			}
			else
			{
				BP2Status = 1;
				pEnt->IndexBP = 2;
			}

			ActivateBomb = false;
			SetThink( NULL );

			g_pGameRules->GetNextBestWeapon( m_pPlayer, this );
		}
	}
}

void CC4::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;
		
	if( !( m_pPlayer->pev->button & IN_ATTACK ) && ActivateBomb )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgKillPrgBar, NULL, m_pPlayer->pev );
		MESSAGE_END();
		ActivateBomb = false;
		SetThink( NULL );
	}
	
	if(
		( !m_pPlayer->OnBombPlant1 && !m_pPlayer->OnBombPlant2 ) ||
		( m_pPlayer->OnBombPlant1 && BP1Status ) ||
		( m_pPlayer->OnBombPlant2 && BP2Status )
		)
	{
		if( ActivateBomb )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgKillPrgBar, NULL, m_pPlayer->pev );
			MESSAGE_END();
			ActivateBomb = false;
			SetThink( NULL );
		}
	}
	
	if( ActivateBomb )
	{
		return;
	}

	SendWeaponAnim( C4_IDLE1 );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase();
}