#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_awp, CAWP );


enum awp_e
{
    AWP_IDLE = 0,
    AWP_SHOOT1,
    AWP_SHOOT2,
    AWP_SHOOT3,
    AWP_RELOAD,
    AWP_DRAW
};


void CAWP::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_awp" );
	Precache( );
	SET_MODEL( ENT(pev), "models/weapons/awp/w_awp.mdl" );
	m_iId = WEAPON_AWP;
	m_iDefaultAmmo = 120;
	FallInit();

	wepspread_s.MaxSpreadX = 5;
	wepspread_s.MaxSpreadY = 5;

	wepspread_s.SpreadX = 5;
	wepspread_s.SpreadY = 5;
	scope = false;
}

void CAWP::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/awp/v_awp.mdl" );
	PRECACHE_MODEL( "models/weapons/awp/p_awp.mdl" );
	PRECACHE_MODEL( "models/weapons/awp/w_awp.mdl" );

	m_iShell = PRECACHE_MODEL ("models/rshell_big.mdl");

	PRECACHE_SOUND( "weapons/awp1.wav" );
	PRECACHE_SOUND( "weapons/zoom.wav" );

	m_event = PRECACHE_EVENT( 1, "events/awp.sc" );
}

int CAWP::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_awp";
	p->iMaxAmmo1 = 30;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 10;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_AWP;
	p->iWeight = 30;

	return 1;
}

int CAWP::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CAWP::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 210 );
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
	scope = false;
	return DefaultDeploy( "models/weapons/awp/v_awp.mdl", "models/weapons/awp/p_awp.mdl", AWP_DRAW, "rifle" );
}

void CAWP::Holster( int skiplocal )
{
	m_pPlayer->pev->fov = 0;
	m_pPlayer->m_iFOV = m_pPlayer->pev->fov;
	scope = false;

#ifndef CLIENT_DLL
	MESSAGE_BEGIN( MSG_ONE, gmsgScopeToggle, NULL, m_pPlayer->pev );
		WRITE_BYTE( 0 );
	MESSAGE_END();
#endif

	m_fInReload = FALSE;
	m_pPlayer->pev->viewmodel = 0; 
	m_pPlayer->pev->weaponmodel = 0;
}

void CAWP::PrimaryAttack()
{
	if( m_iClip <= 0 )
		return;

	m_iClip--;

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc( m_pPlayer->GetGunPosition() );
	Vector vecAim( m_pPlayer->GetAutoaimVector( AUTOAIM_2DEGREES ) );
	Vector vecAcc( g_vecZero );

	if ( !( m_pPlayer->pev->flags & FL_ONGROUND ) )
		vecAcc = vec3_t( 0.85, 0.85, 0.85 );
	else if ( m_pPlayer->pev->velocity.Length2D() > 140 || !scope )
		vecAcc = vec3_t( 0.25, 0.25, 0.25 );
    else if ( m_pPlayer->pev->velocity.Length2D() > 10 )
		vecAcc = vec3_t( 0.10, 0.10, 0.10 );
    else
		vecAcc = vec3_t( 0, 0, 0 );

	Vector vecDir( m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAim, vecAcc, 8192,0, 0, 114, m_pPlayer->pev, m_pPlayer->random_seed ) );

	SendWeaponAnim( AWP_SHOOT1 );

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_event, 0.0,
		(float *)&g_vecZero, (float *)&g_vecZero,
		vecDir.x, vecDir.y, 0, 0, (m_iClip ? 0 : 1), 0 );

	m_pPlayer->pev->fov = 0;
	m_pPlayer->m_iFOV = m_pPlayer->pev->fov;
	scope = false;
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 210 );
#ifndef CLIENT_DLL
	MESSAGE_BEGIN( MSG_ONE, gmsgScopeToggle, NULL, m_pPlayer->pev );
		WRITE_BYTE( 0 );
	MESSAGE_END();
#endif

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.45;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.45;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.45;
}

void CAWP::SecondaryAttack()
{
	if( m_pPlayer->pev->fov == 0 )
	{
		g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 150 );
		m_pPlayer->pev->fov = 40;
		scope = true;
	}
	else if( m_pPlayer->pev->fov == 40 )
	{
		m_pPlayer->pev->fov = 10;
	}
	else
	{
		g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 210 );
		m_pPlayer->pev->fov = 0;
		scope = false;
	}

	m_pPlayer->m_iFOV = m_pPlayer->pev->fov;

#ifndef CLIENT_DLL
	MESSAGE_BEGIN( MSG_ONE, gmsgScopeToggle, NULL, m_pPlayer->pev );
		WRITE_BYTE( scope );
	MESSAGE_END();
#endif

	EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM, "weapons/zoom.wav", 1.0, ATTN_NORM, 0, 100 );
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CAWP::Reload( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == 10 )
		return;

	m_pPlayer->pev->fov = 0;
	m_pPlayer->m_iFOV = m_pPlayer->pev->fov;
	scope = false;
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 210 );

#ifndef CLIENT_DLL
	MESSAGE_BEGIN( MSG_ONE, gmsgScopeToggle, NULL, m_pPlayer->pev );
		WRITE_BYTE( 0 );
	MESSAGE_END();
#endif

	DefaultReload( 10, AWP_RELOAD, 3.3 );
}

void CAWP::WeaponIdle( void )
{

	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim( AWP_IDLE );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}