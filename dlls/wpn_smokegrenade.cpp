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
    SMOKEGRENADE_IDLE = 0,
    SMOKEGRENADE_PULLPIN,
    SMOKEGRENADE_THROW,
    SMOKEGRENADE_DEPLOY
};


LINK_ENTITY_TO_CLASS( weapon_smokegrenade, CSmokeGrenade );


void CSmokeGrenade::Spawn( )
{
	Precache( );
	m_iId = WEAPON_SMOKEGRENADE;
	SET_MODEL(ENT(pev), "models/weapons/smokegrenade/w_smokegrenade.mdl");

#ifndef CLIENT_DLL
	pev->dmg = 0;
#endif

	m_iDefaultAmmo = 1;

	FallInit();// get ready to fall down.
}


void CSmokeGrenade::Precache( void )
{
	PRECACHE_MODEL("models/weapons/smokegrenade/w_smokegrenade.mdl");
	PRECACHE_MODEL("models/weapons/smokegrenade/v_smokegrenade.mdl");
	PRECACHE_MODEL("models/weapons/smokegrenade/p_smokegrenade.mdl");

	PRECACHE_SOUND( "weapons/pinpull.wav" );
	PRECACHE_SOUND( "weapons/sg_explode.wav" );
}

int CSmokeGrenade::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "SmokeGrenade";
    p->iMaxAmmo1 = 1;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = WEAPON_NOCLIP;
    p->iSlot     = 3;
    p->iPosition = 2;
    p->iId       = m_iId = WEAPON_SMOKEGRENADE;
    p->iWeight   = 2;
    p->iFlags    = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

    return 1;
}


BOOL CSmokeGrenade::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 250 );
	m_flReleaseThrow = -1;
	return DefaultDeploy( "models/weapons/smokegrenade/v_smokegrenade.mdl", "models/weapons/smokegrenade/p_smokegrenade.mdl", SMOKEGRENADE_DEPLOY, "grenade" );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
}

BOOL CSmokeGrenade::CanHolster( void )
{
	// can only holster hand grenades when not primed!
	return ( m_flStartThrow == 0 );
}


void CSmokeGrenade::Holster( int skiplocal )
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
	{
		//SendWeaponAnim( HANDGRENADE_HOLSTER );
	}
	else
	{
		// no more grenades!
		m_pPlayer->pev->weapons &= ~( 1<<WEAPON_SMOKEGRENADE );
		SetThink( &CBasePlayerItem::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
	
}


void CSmokeGrenade::PrimaryAttack()
{
	if ( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
    {
        m_flStartThrow   = gpGlobals->time;
        m_flReleaseThrow = 0;

        SendWeaponAnim( SMOKEGRENADE_PULLPIN );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
    }
}

void CSmokeGrenade::WeaponIdle( void )
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

		CGrenade::SmokeShoot( m_pPlayer->pev, vecSrc, vecThrow, 1.5, "models/weapons/smokegrenade/w_smokegrenade.mdl", 100 );

        SendWeaponAnim( SMOKEGRENADE_THROW, UseDecrement() );

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
			SendWeaponAnim( SMOKEGRENADE_DEPLOY );
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
		SendWeaponAnim( SMOKEGRENADE_IDLE, UseDecrement() );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10.0, 15.0 );
	}
}
