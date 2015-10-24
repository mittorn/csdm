#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_mp5navy, CMP5 );


enum mp5n_e
{
    MP5N_IDLE = 0,
    MP5N_RELOAD,
    MP5N_DRAW,
    MP5N_SHOOT1,
    MP5N_SHOOT2,
    MP5N_SHOOT3
};


void CMP5::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_mp5navy" );
	Precache( );
	SET_MODEL( ENT(pev), "models/weapons/mp5/w_mp5.mdl" );
	m_iId = WEAPON_MP5;
	m_iDefaultAmmo = 150;
	FallInit();

	wepspread_s.MaxSpreadX = 2;
	wepspread_s.MaxSpreadY = 2;

	wepspread_s.SpreadX = 1.5;
	wepspread_s.SpreadY = 1.5;
}

void CMP5::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/mp5/v_mp5.mdl" );
	PRECACHE_MODEL( "models/weapons/mp5/p_mp5.mdl" );
	PRECACHE_MODEL( "models/weapons/mp5/w_mp5.mdl" );

	m_iShell = PRECACHE_MODEL ("models/rshell.mdl");

	PRECACHE_SOUND( "weapons/mp5-1.wav" );

	m_event = PRECACHE_EVENT( 1, "events/mp5.sc" );
}

int CMP5::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_mp5";
	p->iMaxAmmo1 = 120;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 30;
	p->iSlot = 0;
	p->iPosition = 7;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_MP5;
	p->iWeight = 25;

	return 1;
}

int CMP5::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CMP5::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 250 );
	return DefaultDeploy( "models/weapons/mp5/v_mp5.mdl", "models/weapons/mp5/p_mp5.mdl", MP5N_DRAW, "mp5" );
}

void CMP5::PrimaryAttack()
{
	DefaultFire( m_pPlayer, 1, wepspread_s, 26, MP5N_SHOOT1, "weapons/mp5-1.wav", m_event, 20 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.08;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);

}

void CMP5::Reload( void )
{
		DefaultReload( 30, MP5N_RELOAD, 2.63 );
}

void CMP5::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim( MP5N_IDLE );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}