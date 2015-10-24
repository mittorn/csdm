/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

enum hegrenade_e
{
    FLASHBANG_IDLE = 0,
    FLASHBANG_PULLPIN,
    FLASHBANG_THROW,
    FLASHBANG_DEPLOY
};


LINK_ENTITY_TO_CLASS( weapon_flashbang, CFlashBang );


void CFlashBang::Spawn( )
{
	Precache( );
	m_iId = WEAPON_FLASHBANG;
	SET_MODEL(ENT(pev), "models/weapons/flashbang/w_flashbang.mdl");

#ifndef CLIENT_DLL
	pev->dmg = 0;
#endif

	m_iDefaultAmmo = 1;

	FallInit();// get ready to fall down.
}


void CFlashBang::Precache( void )
{
	PRECACHE_MODEL("models/weapons/flashbang/w_flashbang.mdl");
	PRECACHE_MODEL("models/weapons/flashbang/v_flashbang.mdl");
	PRECACHE_MODEL("models/weapons/flashbang/p_flashbang.mdl");

	PRECACHE_SOUND( "weapons/pinpull.wav" );
	PRECACHE_SOUND( "weapons/flashbang_explode.wav" );
}

int CFlashBang::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "Flashbang";
    p->iMaxAmmo1 = 2;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = WEAPON_NOCLIP;
    p->iSlot     = 3;
    p->iPosition = 0;
    p->iId       = m_iId = WEAPON_FLASHBANG;
    p->iWeight   = 1;
    p->iFlags    = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

    return 1;
}


BOOL CFlashBang::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 250 );
	m_flReleaseThrow = -1;
	return DefaultDeploy( "models/weapons/flashbang/v_flashbang.mdl", "models/weapons/flashbang/p_flashbang.mdl", FLASHBANG_DEPLOY, "grenade" );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
}

BOOL CFlashBang::CanHolster( void )
{
	// can only holster hand grenades when not primed!
	return ( m_flStartThrow == 0 );
}


void CFlashBang::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
	{
		//SendWeaponAnim( HANDGRENADE_HOLSTER );
	}
	else
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~( 1<<WEAPON_FLASHBANG );
		SetThink( &CBasePlayerItem::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
	
}


void CFlashBang::PrimaryAttack()
{
	if ( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
    {
        m_flStartThrow   = gpGlobals->time;
        m_flReleaseThrow = 0;

        SendWeaponAnim( FLASHBANG_PULLPIN );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
    }
}

void CFlashBang::WeaponIdle( void )
{
	if ( m_flReleaseThrow == 0 && m_flStartThrow )
		 m_flReleaseThrow = gpGlobals->time;

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_flStartThrow )
	{
        Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

        if ( angThrow.x < 0 )
            angThrow.x = -10 + angThrow.x * ( ( 90 - 10 ) / 90.0 );
        else
            angThrow.x = -10 + angThrow.x * ( ( 90 + 10 ) / 90.0 );

        float flVel = ( 90 - angThrow.x ) * 6;

        if ( flVel > 750 )
        {
            flVel = 750;
        }

        UTIL_MakeVectors( angThrow );

        Vector vecSrc   = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16;
        Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

		CGrenade::FlashShoot( m_pPlayer->pev, vecSrc, vecThrow, 1.5, "models/weapons/flashbang/w_flashbang.mdl", 100 );

        SendWeaponAnim( FLASHBANG_THROW, UseDecrement() );

        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        m_flStartThrow = 0;

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
        m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + 0.75;

        m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ]--;

        if ( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
        {
           m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 0.5;
           m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
           m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 0.5;
        }

        return;
	}
	else if ( m_flReleaseThrow > 0 )
	{
		// we've finished the throw, restart.
		m_flStartThrow = 0;

		if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
		{
			SendWeaponAnim( FLASHBANG_DEPLOY );
		}
		else
		{

			RetireWeapon();
			return;
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		m_flReleaseThrow = -1;
		return;
	}

	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		SendWeaponAnim( FLASHBANG_IDLE, UseDecrement() );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0, 15.0 );
	}
}
