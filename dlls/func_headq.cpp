#include "extdll.h"
#include "util.h"
#include "cbase.h"

//============ func_headq ==================
class CFuncHeadQ : public CBaseEntity
{
public:
    void Spawn( void );
    void Precache( void );
};

LINK_ENTITY_TO_CLASS( func_headq, CFuncHeadQ ); //link func_jail to CFuncJail

void CFuncHeadQ::Precache( void )
{
    pev->solid = SOLID_NOT;
	PRECACHE_MODEL("models/headquarters.mdl");
}
void CFuncHeadQ::Spawn( void )
{
    Precache();
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize( pev, g_vecZero, g_vecZero );
    SET_MODEL(ENT(pev), "models/headquarters.mdl" );
	pev->effects |= EF_NODRAW;
}