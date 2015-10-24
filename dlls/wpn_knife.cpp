#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_knife, CKnife );


enum knife_e
{
    KNIFE_IDLE1 = 0,
    KNIFE_SLASH1,
    KNIFE_SLASH2,
    KNIFE_DRAW,
    KNIFE_STAB,
    KNIFE_STAB_MISS,
    KNIFE_MIDSLASH1,
    KNIFE_MIDSLASH2
};


void CKnife::Spawn( )
{
	pev->classname = MAKE_STRING( "weapon_knife" );
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/knife/w_knife.mdl");
	m_iId = 1;
	m_iDefaultAmmo = -1;
	FallInit();
}

void CKnife::Precache( void )
{
    PRECACHE_MODEL( "models/weapons/knife/v_knife.mdl" );
    PRECACHE_MODEL( "models/weapons/knife/p_knife.mdl" );
	PRECACHE_MODEL( "models/weapons/knife/w_knife.mdl" );

    PRECACHE_SOUND( "weapons/knife_deploy1.wav"  );
    PRECACHE_SOUND( "weapons/knife_hit1.wav"     );
    PRECACHE_SOUND( "weapons/knife_hit2.wav"     );
    PRECACHE_SOUND( "weapons/knife_hit3.wav"     );
    PRECACHE_SOUND( "weapons/knife_hit4.wav"     );
    PRECACHE_SOUND( "weapons/knife_slash1.wav"   );
    PRECACHE_SOUND( "weapons/knife_slash2.wav"   );
    PRECACHE_SOUND( "weapons/knife_stab.wav"     );
    PRECACHE_SOUND( "weapons/knife_hitwall1.wav" );

    m_event = PRECACHE_EVENT( 1, "events/knife.sc" );
}

int CKnife::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = 1;
	p->iWeight = 0;

	return 1;
}

int CKnife::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CKnife::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 250 );
	EMIT_SOUND_DYN( ENT( pev ), CHAN_ITEM, "weapons/knife_deploy1.wav", VOL_NORM, 2.4, 0, PITCH_NORM );
	return DefaultDeploy("models/weapons/knife/v_knife.mdl", "models/weapons/knife/p_knife.mdl",
		KNIFE_DRAW,"knife");
}

void CKnife::PrimaryAttack()
{
	Attack( 20, 0.35, 0.5, 48, KNIFE_MIDSLASH1 );
}

void CKnife::SecondaryAttack()
{
	Attack( 60, 1.1, 1.1, 30, KNIFE_STAB_MISS );
}

void CKnife::Attack( int iDamage, float NextPrimary, float NextSecondary, float iDistance, int iAnim )
{
	SendWeaponAnim( iAnim );
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	EMIT_SOUND_DYN( ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_slash1.wav", VOL_NORM, ATTN_NORM, 0, 94 );

	TraceResult tr;
	UTIL_MakeVectors ( m_pPlayer->pev->v_angle );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * iDistance;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

	#ifndef CLIENT_DLL

	if ( tr.flFraction >= 1 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );
		if ( tr.flFraction < 1 )
		{
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
			if ( !pHit || pHit->IsBSPModel() )
				vecEnd = tr.vecEndPos;
		}
	}

	if ( tr.flFraction < 1 )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
		ClearMultiDamage( );
		pEntity->TraceAttack(m_pPlayer->pev, iDamage, gpGlobals->v_forward, &tr, DMG_CLUB ); 
		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );


		if (pEntity)
		{
			if ( pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE )
			{
				// play thwack or smack sound
				switch( RANDOM_LONG( 1, 4) )
				{
				case 1:
					EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94 );
					break;
				case 2:
					EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94 );
					break;
				case 3:
					EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_hit3.wav", VOL_NORM, ATTN_NORM, 0, 94 );
					break;
				case 4:
					EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_hit4.wav", VOL_NORM, ATTN_NORM, 0, 94 );
					break;
				}
			}
			else
				EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 94 ); 
		}
	}

	#endif
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + NextPrimary;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + NextSecondary;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
}

void CKnife::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim( KNIFE_IDLE1 );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 50.0;
}