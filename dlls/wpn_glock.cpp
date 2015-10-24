#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"


LINK_ENTITY_TO_CLASS( weapon_glock18, Cglock18 );


enum glock18_e 
{
    GLOCK18_IDLE1 = 0,
    GLOCK18_IDLE2,
    GLOCK18_IDLE3,
    GLOCK18_SHOOT1,
    GLOCK18_SHOOT2,
    GLOCK18_SHOOT3,
    GLOCK18_SHOOT_EMPTY,
    GLOCK18_RELOAD,
    GLOCK18_DRAW,
    GLOCK18_HOLSTER,
    GLOCK18_ADD_SILENCER,
    GLOCK18_DRAW2,
    GLOCK18_RELOAD2
};


void Cglock18::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_glock18");
	Precache( );
	SET_MODEL(ENT(pev), "models/weapons/glock18/w_glock18.mdl");
	m_iId = WEAPON_GLOCK;
	m_iDefaultAmmo = 140;
	autofire = false;
	FallInit();

	wepspread_s.MaxSpreadX = 2;
	wepspread_s.MaxSpreadY = 3;

	wepspread_s.SpreadX = 1.7;
	wepspread_s.SpreadY = 1.7;
}

void Cglock18::Precache( void )
{
	PRECACHE_MODEL("models/weapons/glock18/v_glock18.mdl");
	PRECACHE_MODEL("models/weapons/glock18/p_glock18.mdl");
	PRECACHE_MODEL("models/weapons/glock18/w_glock18.mdl");

	m_iShell = PRECACHE_MODEL ("models/pshell.mdl");

    PRECACHE_SOUND( "weapons/glock18-1.wav"     );
    PRECACHE_SOUND( "weapons/glock18-2.wav"     );

	m_event = PRECACHE_EVENT( 1, "events/glock18.sc" );
}

int Cglock18::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ammo_glock18"; // The type of ammo it uses
	p->iMaxAmmo1 = 120; // Max ammo the player can carry
	p->pszAmmo2 = NULL; // No secondary ammo
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 20; // The clip size
	p->iSlot = 1; // The number in the HUD
	p->iPosition = 1; // The position in a HUD slot
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	p->iId = m_iId = WEAPON_GLOCK; // The weapon id
	p->iWeight = 5; // for autoswitching

	return 1;
}

int Cglock18::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL Cglock18::Deploy( )
{
	g_engfuncs.pfnSetClientMaxspeed(m_pPlayer->edict(), 250 );
	return DefaultDeploy("models/weapons/glock18/v_glock18.mdl", "models/weapons/glock18/p_glock18.mdl",
		GLOCK18_DRAW,"onehanded");
}

void Cglock18::PrimaryAttack()
{
	if(! ( m_pPlayer->m_afButtonPressed & IN_ATTACK ))
		return;

	if( autofire )
	{
		Shots = 0;
		SendWeaponAnim( GLOCK18_SHOOT1 );
		BrustAttack();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);
	}
	else
	{
		DefaultFire( m_pPlayer, 1, wepspread_s, 21, GLOCK18_SHOOT3, "weapons/glock18-2.wav", m_event, 20 );
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.085;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 1.085,2.085);
	}

	return;
}

void Cglock18::BrustAttack( void )
{
	DefaultFire( m_pPlayer, 1, wepspread_s, 21, -1, "weapons/glock18-2.wav", m_event, 20 );
	Shots++;
	if( Shots < 3 )
	{
		SetThink( &Cglock18::BrustAttack );
		pev->nextthink = gpGlobals->time + 0.085;
	}
	else
		SetThink( NULL );
}

void Cglock18::SecondaryAttack( void )
{
	if( autofire )
		ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_SemiAuto" );
	else
		ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_BurstFire" );
	autofire = !autofire;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void Cglock18::Reload( void )
{
	DefaultReload( 20, GLOCK18_RELOAD, 2.2 );
}

void Cglock18::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim( GLOCK18_IDLE1 );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}