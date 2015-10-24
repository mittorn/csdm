#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_usp, CUSP );


enum usp_e 
{
    USP_IDLE = 0,
    USP_SHOOT1,
    USP_SHOOT2,
    USP_SHOOT3,
    USP_SHOOTLAST,
    USP_RELOAD,
    USP_DRAW,
    USP_ADD_SILENCER,
    USP_IDLE_UNSIL,
    USP_SHOOT1_UNSIL,
    USP_SHOOT2_UNSIL,
    USP_SHOOT3_UNSIL,
    USP_SHOOTLAST_UNSIL,
    USP_RELOAD_UNSIL,
    USP_DRAW_UNSIL,
    USP_DETACH_SILENCER
};


void CUSP::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_usp");
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/usp/w_usp.mdl");
	m_iId = WEAPON_USP;
	m_iDefaultAmmo = 112;
	FallInit();

	unsil = true;

	wepspread_s.MaxSpreadX = 5;
	wepspread_s.MaxSpreadY = 5;

	wepspread_s.SpreadX = 2;
	wepspread_s.SpreadY = 2;
}

void CUSP::Precache( void )
{
	PRECACHE_MODEL("models/weapons/usp/v_usp.mdl");
	PRECACHE_MODEL("models/weapons/usp/p_usp.mdl");
	PRECACHE_MODEL("models/weapons/usp/w_usp.mdl");

	m_iShell = PRECACHE_MODEL( "models/pshell.mdl" );

	PRECACHE_SOUND ("weapons/usp1.wav");
	PRECACHE_SOUND ("weapons/usp_unsil-1.wav");

	m_event = PRECACHE_EVENT( 1, "events/usp.sc" );
}

int CUSP::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ammo_usp"; // The type of ammo it uses
	p->iMaxAmmo1 = 100; // Max ammo the player can carry
	p->pszAmmo2 = NULL; // No secondary ammo
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 12; // The clip size
	p->iSlot = 1; // The number in the HUD
	p->iPosition = 0; // The position in a HUD slot
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_USP; // The weapon id
	p->iWeight = 5; // for autoswitching

	return 1;
}

int CUSP::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CUSP::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 250 );

	if( unsil )
		return DefaultDeploy("models/weapons/usp/v_usp.mdl", "models/weapons/usp/p_usp.mdl", USP_DRAW_UNSIL,"onehanded");
	else
		return DefaultDeploy("models/weapons/usp/v_usp.mdl", "models/weapons/usp/p_usp.mdl", USP_DRAW,"onehanded");
}



void CUSP::PrimaryAttack()
{
	if(! ( m_pPlayer->m_afButtonPressed & IN_ATTACK ))
		return;

	if( unsil )
		DefaultFire( m_pPlayer, 1, wepspread_s, 30, USP_SHOOT1_UNSIL, "weapons/usp_unsil-1.wav", m_event, 20 );
	else
		DefaultFire( m_pPlayer, 1, wepspread_s, 30, USP_SHOOT1, "weapons/usp1.wav", m_event, 10 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.085;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);
}

void CUSP::SecondaryAttack( void )
{
	if( unsil )
		SendWeaponAnim( USP_ADD_SILENCER );
	else
		SendWeaponAnim( USP_DETACH_SILENCER );
	unsil = !unsil;
	
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0;
    m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 3.0;
    m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 3.0;
}

void CUSP::Reload( void )
{
	if( unsil )
	{
		DefaultReload( 12, USP_RELOAD_UNSIL, 2.7 );
	}
	else
	{
		DefaultReload( 12, USP_RELOAD, 2.7 );
	}
}

void CUSP::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;
	if( unsil )
	{
		SendWeaponAnim( USP_IDLE_UNSIL );
	}
	else
	{
		SendWeaponAnim( USP_IDLE );
	}

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}