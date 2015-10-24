#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_m3, CM3 );


enum m3_e
{
    M3_IDLE1 = 0,
    M3_SHOOT1,
    M3_SHOOT2,
    M3_INSERT,
    M3_AFTER_RELOAD,
    M3_START_RELOAD,
    M3_DRAW
};


void CM3::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_m3" );
	Precache( );
	SET_MODEL( ENT(pev), "models/weapons/m3/w_m3.mdl" );
	m_iId = WEAPON_M3;
	m_iDefaultAmmo = 40;
	FallInit();

	wepspread_s.MaxSpreadX = 2;
	wepspread_s.MaxSpreadY = 5;

	wepspread_s.SpreadX = 2;
	wepspread_s.SpreadY = 2;
}

void CM3::Precache( void )
{
	PRECACHE_MODEL( "models/weapons/m3/v_m3.mdl" );
	PRECACHE_MODEL( "models/weapons/m3/p_m3.mdl" );
	PRECACHE_MODEL( "models/weapons/m3/w_m3.mdl" );

	m_iShell = PRECACHE_MODEL( "models/shotgunshell.mdl" );

	PRECACHE_SOUND( "weapons/m3-1.wav" );

	m_event = PRECACHE_EVENT( 1, "events/m3.sc" );
}

int CM3::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = "ammo_m3";
	p->iMaxAmmo1 = 32;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 8;
	p->iSlot = 0;
	p->iPosition = 8;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_M3;
	p->iWeight = 25;

	return 1;
}

int CM3::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CM3::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 220 );
	return DefaultDeploy( "models/weapons/m3/v_m3.mdl", "models/weapons/m3/p_m3.mdl", M3_DRAW, "rifle" );
}

void CM3::PrimaryAttack()
{
	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	SendWeaponAnim( M3_SHOOT1 );
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 ); 
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON,"weapons/m3-1.wav", 1.0, ATTN_NORM, 0, 100 );

	m_iClip -= 1;

	Vector vecSrc( m_pPlayer->GetGunPosition() );
	Vector vecAim( m_pPlayer->GetAutoaimVector( AUTOAIM_2DEGREES ) );
	Vector vecAcc( 0.08716, 0.04362, 0.00  );
	Vector vecDir;

	vecDir = m_pPlayer->FireBulletsPlayer( 9, vecSrc, vecAim, vecAcc, 8192,0, 0, 20, m_pPlayer->pev, m_pPlayer->random_seed );
#ifndef CLIENT_DLL
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSrc );

		WRITE_BYTE( TE_DLIGHT );

		WRITE_COORD( vecSrc.x );
		WRITE_COORD( vecSrc.y );
		WRITE_COORD( vecSrc.z );
		WRITE_BYTE( 30 );

		WRITE_BYTE( 210 );
		WRITE_BYTE( 200 );
		WRITE_BYTE( 60 );

		WRITE_BYTE( 1 );
		WRITE_BYTE( 1 );

	MESSAGE_END();
#endif
	if( !LeftSpread )
		LeftSpread = 1;

	m_pPlayer->pev->punchangle.x -= 5;

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_event, 0.0,
		(float *)&g_vecZero, (float *)&g_vecZero,
		vecDir.x, vecDir.y, 0, 0, (m_iClip ? 0 : 1), 0 );

    m_flNextPrimaryAttack   = UTIL_WeaponTimeBase() + 0.875;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);

}

void CM3::Reload( void )
{
	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == 8 )
		return;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	// check to see if we're ready to reload
	if (m_fInSpecialReload == 0)
	{
		SendWeaponAnim( M3_START_RELOAD );
		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.6;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
		return;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
		// was waiting for gun to move to side
		m_fInSpecialReload = 2;

		SendWeaponAnim( M3_INSERT );

		m_flNextReload = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
	else
	{
		// Add them to the clip
		m_iClip += 1;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;
		m_fInSpecialReload = 1;
	}
}


void CM3::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flPumpTime && m_flPumpTime < gpGlobals->time )
	{
		// play pumping sound
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/scock1.wav", 1, ATTN_NORM, 0, 95 + RANDOM_LONG(0,0x1f));
		m_flPumpTime = 0;
	}

	if (m_flTimeWeaponIdle <  UTIL_WeaponTimeBase() )
	{
		if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload( );
		}
		else if (m_fInSpecialReload != 0)
		{
			if (m_iClip != 8 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload( );
			}
			else
			{
				// reload debounce has timed out
				SendWeaponAnim( M3_AFTER_RELOAD );
				
				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
			}
		}
		else
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (60.0/12.0);
			SendWeaponAnim( M3_IDLE1 );
		}
	}
}