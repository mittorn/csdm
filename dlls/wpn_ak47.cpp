#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_ak47, CAK47 );


enum ak47_e
{
    AK47_IDLE1 = 0,
    AK47_RELOAD,
    AK47_DRAW,
    AK47_SHOOT1,
    AK47_SHOOT2,
    AK47_SHOOT3
};


void CAK47::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_ak47" );
	Precache( );
	SET_MODEL( ENT(pev), "models/weapons/ak47/w_ak47.mdl" );
	m_iId = WEAPON_AK47	;
	m_iDefaultAmmo = 120;
	FallInit();

	wepspread_s.MaxSpreadX = 2;
	wepspread_s.MaxSpreadY = 5;

	wepspread_s.SpreadX = 2;
	wepspread_s.SpreadY = 2;
}

void CAK47::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/ak47/v_ak47.mdl" );
	PRECACHE_MODEL( "models/weapons/ak47/p_ak47.mdl" );
	PRECACHE_MODEL( "models/weapons/ak47/w_ak47.mdl" );

	m_iShell = PRECACHE_MODEL ("models/rshell.mdl");

	PRECACHE_SOUND( "weapons/ak47-1.wav" );

	m_event = PRECACHE_EVENT( 1, "events/ak47.sc" );
}

int CAK47::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_ak47";
	p->iMaxAmmo1 = 90;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 30;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_AK47;
	p->iWeight = 25;

	return 1;
}

int CAK47::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CAK47::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 221 );
	return DefaultDeploy( "models/weapons/ak47/v_ak47.mdl", "models/weapons/ak47/p_ak47.mdl", AK47_DRAW, "ak47" );
}

void CAK47::PrimaryAttack()
{
	DefaultFire( m_pPlayer, 1, wepspread_s, 35, AK47_SHOOT1, "weapons/ak47-1.wav", m_event, 20 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);

}

void CAK47::Reload( void )
{
		DefaultReload( 30, AK47_RELOAD, 2.45 );
}

void CAK47::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim( AK47_IDLE1 );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}