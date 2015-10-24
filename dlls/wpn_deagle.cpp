#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_deagle, CDEAGLE );


enum deagle_e 
{
    DEAGLE_IDLE1 = 0,
    DEAGLE_SHOOT1,
    DEAGLE_SHOOT2,
    DEAGLE_SHOOT_EMPTY,
    DEAGLE_RELOAD,
    DEAGLE_DRAW
};


void CDEAGLE::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_deagle" );
	Precache( );
	SET_MODEL( ENT(pev), "models/weapons/deagle/w_deagle.mdl" );
	m_iId = WEAPON_DEAGLE;
	m_iDefaultAmmo = 42;
	FallInit();

	wepspread_s.MaxSpreadX = 2;
	wepspread_s.MaxSpreadY = 5;

	wepspread_s.SpreadX = 3.5;
	wepspread_s.SpreadY = 3.5;
}

void CDEAGLE::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/deagle/v_deagle.mdl" );
	PRECACHE_MODEL( "models/weapons/deagle/p_deagle.mdl" );
	PRECACHE_MODEL( "models/weapons/deagle/w_deagle.mdl" );

	m_iShell = PRECACHE_MODEL ("models/pshell.mdl");

	PRECACHE_SOUND( "weapons/deagle-1.wav" );

	m_event = PRECACHE_EVENT( 1, "events/deagle.sc" );
}

int CDEAGLE::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_deagle";
	p->iMaxAmmo1 = 35;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 7;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_DEAGLE;
	p->iWeight = 7;

	return 1;
}

int CDEAGLE::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CDEAGLE::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 250 );
	return DefaultDeploy( "models/weapons/deagle/v_deagle.mdl", "models/weapons/deagle/p_deagle.mdl", DEAGLE_DRAW, "onehanded" );
}

void CDEAGLE::PrimaryAttack()
{
	if(! ( m_pPlayer->m_afButtonPressed & IN_ATTACK ) )
		return;
	DefaultFire( m_pPlayer, 1, wepspread_s, 47, DEAGLE_SHOOT1, "weapons/deagle-1.wav", m_event, 20 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.18;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);

}

void CDEAGLE::Reload( void )
{
		DefaultReload( 7, DEAGLE_RELOAD, 2.2 );
}

void CDEAGLE::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim( DEAGLE_IDLE1 );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}