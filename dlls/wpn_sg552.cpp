#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_sg552, CSG552 );


enum sg552_e
{
    SG552_IDLE = 0,
    SG552_RELOAD,
    SG552_DRAW,
    SG552_SHOOT1,
    SG552_SHOOT2,
    SG552_SHOOT3
};


void CSG552::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_sg552" );
	Precache( );
	SET_MODEL( ENT(pev), "models/weapons/sg552/w_sg552.mdl" );
	m_iId = WEAPON_SG552;
	m_iDefaultAmmo = 120;
	FallInit();

	wepspread_s.MaxSpreadX = 2;
	wepspread_s.MaxSpreadY = 5;

	wepspread_s.SpreadX = 2;
	wepspread_s.SpreadY = 2;

	Scope = false;
}

void CSG552::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/sg552/v_sg552.mdl" );
	PRECACHE_MODEL( "models/weapons/sg552/p_sg552.mdl" );
	PRECACHE_MODEL( "models/weapons/sg552/w_sg552.mdl" );

	m_iShell = PRECACHE_MODEL ("models/rshell.mdl");

	PRECACHE_SOUND( "weapons/sg552-1.wav" );

	m_event = PRECACHE_EVENT( 1, "events/sg552.sc" );
}

int CSG552::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_sg552";
	p->iMaxAmmo1 = 90;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 30;
	p->iSlot = 0;
	p->iPosition = 6;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_SG552;
	p->iWeight = 25;

	return 1;
}

int CSG552::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CSG552::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 210 );
	return DefaultDeploy( "models/weapons/sg552/v_sg552.mdl", "models/weapons/sg552/p_sg552.mdl", SG552_DRAW, "mp5" );
}

void CSG552::Holster( int skiplocal )
{
	m_pPlayer->pev->fov = 0;
	m_pPlayer->m_iFOV = m_pPlayer->pev->fov;
	Scope = false;

	m_fInReload = FALSE;
	m_pPlayer->pev->viewmodel = 0; 
	m_pPlayer->pev->weaponmodel = 0;
}

void CSG552::PrimaryAttack()
{
	if( Scope )
	{
		DefaultFire( m_pPlayer, 1, wepspread_s, 33, SG552_SHOOT1, "weapons/sg552-1.wav", m_event, 20 );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1825;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);
	}
	else
	{
		DefaultFire( m_pPlayer, 1, wepspread_s, 33, SG552_SHOOT1, "weapons/sg552-1.wav", m_event, 20 );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.0825;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);
	}

}

void CSG552::SecondaryAttack( void )
{
	if( Scope )
		m_pPlayer->pev->fov = 0;	
	else
		m_pPlayer->pev->fov = 55;
		
	m_pPlayer->m_iFOV = m_pPlayer->pev->fov;

	Scope = !Scope;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CSG552::Reload( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == 30 )
		return;

	m_pPlayer->pev->fov = 0;
	m_pPlayer->m_iFOV = m_pPlayer->pev->fov;
	Scope = false;

	DefaultReload( 30, SG552_RELOAD, 3.3 );
}

void CSG552::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim( SG552_IDLE );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}