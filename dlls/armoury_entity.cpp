#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"


LINK_ENTITY_TO_CLASS( armoury_entity, CArmoury );

void CArmoury::Spawn( void )
{
	switch(pev->iuser1)
	{

	case 0:
		myweapon = (CBasePlayerWeapon*)Create("weapon_mp5navy",pev->origin,Vector(0,0,0),NULL);
		break;

	case 1:
		myweapon = (CBasePlayerWeapon*)Create("weapon_tmp",pev->origin,Vector(0,0,0),NULL);
		break;

	case 2:
		myweapon = (CBasePlayerWeapon*)Create("weapon_p90",pev->origin,Vector(0,0,0),NULL);
		break;

	case 3:
		myweapon = (CBasePlayerWeapon*)Create("weapon_mac10",pev->origin,Vector(0,0,0),NULL);
		break;

	case 4:
		myweapon = (CBasePlayerWeapon*)Create("weapon_ak47",pev->origin,Vector(0,0,0),NULL);
		break;

	case 5:
		myweapon = (CBasePlayerWeapon*)Create("weapon_sg552",pev->origin,Vector(0,0,0),NULL);
		break;

	case 6:
		myweapon = (CBasePlayerWeapon*)Create("weapon_m4a1",pev->origin,Vector(0,0,0),NULL);
		break;

	case 7:
		myweapon = (CBasePlayerWeapon*)Create("weapon_aug",pev->origin,Vector(0,0,0),NULL);
		break;

	case 8:
		myweapon = (CBasePlayerWeapon*)Create("weapon_scout",pev->origin,Vector(0,0,0),NULL);
		break;

	case 9:
		myweapon = (CBasePlayerWeapon*)Create("weapon_g3sg1",pev->origin,Vector(0,0,0),NULL);
		break;

	case 10:
		myweapon = (CBasePlayerWeapon*)Create("weapon_awp",pev->origin,Vector(0,0,0),NULL);
		break;

	case 11:
		myweapon = (CBasePlayerWeapon*)Create("weapon_m3",pev->origin,Vector(0,0,0),NULL);
		break;

	case 12:
		myweapon = (CBasePlayerWeapon*)Create("weapon_xm1014",pev->origin,Vector(0,0,0),NULL);
		break;

	case 13:
		myweapon = (CBasePlayerWeapon*)Create("weapon_m249",pev->origin,Vector(0,0,0),NULL);
		break;

	case 14:
		myweapon = (CBasePlayerWeapon*)Create("weapon_flashbang",pev->origin,Vector(0,0,0),NULL);
		break;

	case 15:
		myweapon = (CBasePlayerWeapon*)Create("weapon_hegrenade",pev->origin,Vector(0,0,0),NULL);
		break;

	case 16:
		myweapon = (CBasePlayerWeapon*)Create("item_kevlar",pev->origin,Vector(0,0,0),NULL);
		break;

	case 17:
		myweapon = (CBasePlayerWeapon*)Create("item_assaultsuit",pev->origin,Vector(0,0,0),NULL);
		break;

	case 18:
		myweapon = (CBasePlayerWeapon*)Create("weapon_smokegrenade",pev->origin,Vector(0,0,0),NULL);
		break;

	case 19:
		myweapon = (CBasePlayerWeapon*)Create("weapon_famas",pev->origin,Vector(0,0,0),NULL);
		break;

	case 20:
		myweapon = (CBasePlayerWeapon*)Create("weapon_galil",pev->origin,Vector(0,0,0),NULL);
		break;

	case 21:
		myweapon = (CBasePlayerWeapon*)Create("weapon_deagle",pev->origin,Vector(0,0,0),NULL);
		break;

	}

}

void CArmoury::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "item"))
	{
		pev->iuser1 = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}