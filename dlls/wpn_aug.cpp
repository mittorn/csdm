#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_aug, CAUG );


enum aug_e
{
    AUG_IDLE = 0,
    AUG_RELOAD,
    AUG_DRAW,
    AUG_SHOOT1,
    AUG_SHOOT2,
    AUG_SHOOT3
};


void CAUG::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_aug" );
	Precache( );
	SET_MODEL( ENT(pev), "models/weapons/aug/w_aug.mdl" );
	m_iId = WEAPON_AUG;
	m_iDefaultAmmo = 120;
	FallInit();

	wepspread_s.MaxSpreadX = 2;
	wepspread_s.MaxSpreadY = 5;

	wepspread_s.SpreadX = 2;
	wepspread_s.SpreadY = 2;

	Scope = false;
}

void CAUG::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/aug/v_aug.mdl" );
	PRECACHE_MODEL( "models/weapons/aug/p_aug.mdl" );
	PRECACHE_MODEL( "models/weapons/aug/w_aug.mdl" );

	m_iShell = PRECACHE_MODEL ("models/rshell.mdl");

	PRECACHE_SOUND( "weapons/aug-1.wav" );

	m_event = PRECACHE_EVENT( 1, "events/aug.sc" );
}

int CAUG::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_aug";
	p->iMaxAmmo1 = 90;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 30;
	p->iSlot = 0;
	p->iPosition = 5;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_AUG;
	p->iWeight = 25;

	return 1;
}

int CAUG::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CAUG::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 221 );
	return DefaultDeploy( "models/weapons/aug/v_aug.mdl", "models/weapons/aug/p_aug.mdl", AUG_DRAW, "carbine" );
}

void CAUG::Holster( int skiplocal )
{
	m_pPlayer->pev->fov = 0;
	m_pPlayer->m_iFOV = m_pPlayer->pev->fov;
	Scope = false;

	m_fInReload = FALSE;
	m_pPlayer->pev->viewmodel = 0; 
	m_pPlayer->pev->weaponmodel = 0;
}

void CAUG::PrimaryAttack()
{
	if( Scope )
	{
		DefaultFire( m_pPlayer, 1, wepspread_s, 32, AUG_SHOOT1, "weapons/aug-1.wav", m_event, 20 );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1825;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);
	}
	else
	{
		DefaultFire( m_pPlayer, 1, wepspread_s, 32, AUG_SHOOT1, "weapons/aug-1.wav", m_event, 20 );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.0825;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);
	}

}

void CAUG::SecondaryAttack( void )
{
	if( Scope )
		m_pPlayer->pev->fov = 0;	
	else
		m_pPlayer->pev->fov = 55;

	m_pPlayer->m_iFOV = m_pPlayer->pev->fov;
		

	Scope = !Scope;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CAUG::Reload( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == 30 )
		return;

	m_pPlayer->pev->fov = 0;
	m_pPlayer->m_iFOV = m_pPlayer->pev->fov;
	Scope = false;

	DefaultReload( 30, AUG_RELOAD, 3.3 );
}

void CAUG::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim( AUG_IDLE );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}