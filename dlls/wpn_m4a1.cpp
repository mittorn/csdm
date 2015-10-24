#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_m4a1, CM4A1 );


enum m4a1_e
{
    M4A1_IDLE1 = 0,
    M4A1_SHOOT1,
    M4A1_SHOOT2,
    M4A1_SHOOT3,
    M4A1_RELOAD,
    M4A1_DRAW,
    M4A1_ADD_SILENCER,
    M4A1_IDLE_UNSIL,
    M4A1_SHOOT1_UNSIL,
    M4A1_SHOOT2_UNSIL,
    M4A1_SHOOT3_UNSIL,
    M4A1_RELOAD_UNSIL,
    M4A1_DRAW_UNSIL,
    M4A1_DETACH_SILENCER
};


void CM4A1::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_m4a1" );
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/m4a1/w_m4a1.mdl");
	m_iId = WEAPON_M4A1;
	m_iDefaultAmmo = 120;
	FallInit();

	wepspread_s.MaxSpreadX = 5;
	wepspread_s.MaxSpreadY = 5;

	wepspread_s.SpreadX = 1.5;
	wepspread_s.SpreadY = 1.5;

	unsil = true;
}

void CM4A1::Precache( void )
{
	PRECACHE_MODEL("models/weapons/m4a1/v_m4a1.mdl");
	PRECACHE_MODEL("models/weapons/m4a1/p_m4a1.mdl");
	PRECACHE_MODEL("models/weapons/m4a1/w_m4a1.mdl");

	m_iShell = PRECACHE_MODEL ("models/rshell.mdl");

	PRECACHE_SOUND ("weapons/m4a1-1.wav");
	PRECACHE_SOUND ("weapons/m4a1_unsil-1.wav");

	m_event = PRECACHE_EVENT( 1, "events/m4a1.sc" );
}

int CM4A1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ammo_m4a1"; // The type of ammo it uses
	p->iMaxAmmo1 = 90; // Max ammo the player can carry
	p->pszAmmo2 = NULL; // No secondary ammo
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 30; // The clip size
	p->iSlot = 0; // The number in the HUD
	p->iPosition = 0; // The position in a HUD slot
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_M4A1; // The weapon id
	p->iWeight = 25; // for autoswitching

	return 1;
}

int CM4A1::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CM4A1::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 230 );

	if( unsil )
		return DefaultDeploy( "models/weapons/m4a1/v_m4a1.mdl", "models/weapons/m4a1/p_m4a1.mdl", M4A1_DRAW_UNSIL, "rifle" );
	else
		return DefaultDeploy( "models/weapons/m4a1/v_m4a1.mdl", "models/weapons/m4a1/p_m4a1.mdl", M4A1_DRAW, "rifle" );
	
}

void CM4A1::PrimaryAttack()
{
	if( unsil )
		DefaultFire( m_pPlayer, 1, wepspread_s, 31, M4A1_SHOOT1_UNSIL, "weapons/m4a1_unsil-1.wav", m_event, 20 );
	else
		DefaultFire( m_pPlayer, 1, wepspread_s, 31, M4A1_SHOOT1, "weapons/m4a1-1.wav", m_event, 10 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.0875;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);

}

void CM4A1::SecondaryAttack( void )
{
	if( unsil )
		SendWeaponAnim( M4A1_ADD_SILENCER );
	else
		SendWeaponAnim( M4A1_DETACH_SILENCER );
	unsil = !unsil;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.3;
    m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 2.3;
    m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 2.3;
}

void CM4A1::Reload( void )
{
	if( unsil )
		DefaultReload( 30, M4A1_RELOAD_UNSIL, 3.05 );
	else
		DefaultReload( 30, M4A1_RELOAD, 3.05 );
}

void CM4A1::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if( unsil )
		SendWeaponAnim( M4A1_IDLE_UNSIL );
	else
		SendWeaponAnim( M4A1_IDLE1 );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}