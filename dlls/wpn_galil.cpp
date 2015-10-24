#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_galil, CGALIL );


enum galil_e
{
    GALIL_IDLE = 0,
    GALIL_RELOAD,
    GALIL_DRAW,
    GALIL_SHOOT1,
    GALIL_SHOOT2,
    GALIL_SHOOT3
};



void CGALIL::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_galil" );
	Precache( );
	SET_MODEL( ENT(pev), "models/weapons/galil/w_galil.mdl" );
	m_iId = WEAPON_GALIL;
	m_iDefaultAmmo = 125;
	FallInit();

	wepspread_s.MaxSpreadX = 2;
	wepspread_s.MaxSpreadY = 5;

	wepspread_s.SpreadX = 1.6;
	wepspread_s.SpreadY = 1.6;
}

void CGALIL::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/galil/v_galil.mdl" );
	PRECACHE_MODEL( "models/weapons/galil/p_galil.mdl" );
	PRECACHE_MODEL( "models/weapons/galil/w_galil.mdl" );

	m_iShell = PRECACHE_MODEL ("models/rshell.mdl");

	PRECACHE_SOUND( "weapons/galil-1.wav" );

	m_event = PRECACHE_EVENT( 1, "events/galil.sc" );
}

int CGALIL::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_galil";
	p->iMaxAmmo1 = 90;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 35;
	p->iSlot = 0;
	p->iPosition = 3;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_GALIL;
	p->iWeight = 25;

	return 1;
}

int CGALIL::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CGALIL::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 210 );
	return DefaultDeploy( "models/weapons/galil/v_galil.mdl", "models/weapons/galil/p_galil.mdl", GALIL_DRAW, "ak47" );
}

void CGALIL::PrimaryAttack()
{
	DefaultFire( m_pPlayer, 1, wepspread_s, 29, GALIL_SHOOT1, "weapons/galil-1.wav", m_event, 20 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.0875;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);

}

void CGALIL::Reload( void )
{
		DefaultReload( 35, GALIL_RELOAD, 2.45 );
}

void CGALIL::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim( GALIL_IDLE );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}