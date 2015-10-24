#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_famas, CFAMAS );


enum famas_e 
{
    FAMAS_IDLE = 0,
    FAMAS_RELOAD,
    FAMAS_DRAW,
    FAMAS_SHOOT1,
    FAMAS_SHOOT2,
    FAMAS_SHOOT3
};



void CFAMAS::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_famas" );
	Precache( );
	SET_MODEL( ENT(pev), "models/weapons/famas/w_famas.mdl" );
	m_iId = WEAPON_FAMAS;
	m_iDefaultAmmo = 115;
	FallInit();

	wepspread_s.MaxSpreadX = 2;
	wepspread_s.MaxSpreadY = 5;

	wepspread_s.SpreadX = 1.6;
	wepspread_s.SpreadY = 1.6;

	Brust = false;
}

void CFAMAS::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/famas/v_famas.mdl" );
	PRECACHE_MODEL( "models/weapons/famas/p_famas.mdl" );
	PRECACHE_MODEL( "models/weapons/famas/w_famas.mdl" );

	m_iShell = PRECACHE_MODEL ("models/rshell.mdl");

	PRECACHE_SOUND( "weapons/famas-1.wav" );
	PRECACHE_SOUND( "weapons/famas-burst.wav" );

	m_event = PRECACHE_EVENT( 1, "events/famas.sc" );
}

int CFAMAS::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_famas";
	p->iMaxAmmo1 = 90;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 25;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_FAMAS;
	p->iWeight = 25;

	return 1;
}

int CFAMAS::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CFAMAS::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 210 );
	return DefaultDeploy( "models/weapons/famas/v_famas.mdl", "models/weapons/famas/p_famas.mdl", FAMAS_DRAW, "carbine" );
}

void CFAMAS::PrimaryAttack()
{
	if( m_iClip <= 0 )
		return;
	if( Brust )
	{
		Shots = 0;
		EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/famas-burst.wav", 1.0, ATTN_NORM, 0, 100 );
		BrustAttack();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3825;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);
		
	}
	else
	{
		DefaultFire( m_pPlayer, 1, wepspread_s, 29, FAMAS_SHOOT1, "weapons/famas-1.wav", m_event, 20 );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.0825;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);
	}

}

void CFAMAS::BrustAttack( void )
{
	DefaultFire( m_pPlayer, 1, wepspread_s, 29, FAMAS_SHOOT1, NULL, m_event, 20 );
	Shots++;
	if( Shots < 3 )
	{
		SetThink( &CFAMAS::BrustAttack );
		pev->nextthink = gpGlobals->time + 0.0825;
	}
	else
		SetThink( NULL );
}

void CFAMAS::SecondaryAttack( void )
{
	if( Brust )
		ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_FullAuto" );
	else
		ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_BurstFire" );

	Brust = !Brust;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CFAMAS::Reload( void )
{
		DefaultReload( 25, FAMAS_RELOAD, 3.3 );
}

void CFAMAS::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim( FAMAS_IDLE );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}