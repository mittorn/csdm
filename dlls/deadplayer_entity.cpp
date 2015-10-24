#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CDeadPlayer : public CBaseEntity
{
};

LINK_ENTITY_TO_CLASS( deadplayer_entity, CDeadPlayer );

void CreateDeadPlayerEnt( Vector ORIGIN, Vector ANGLES, CBaseEntity PK, CBaseEntity PV )
{
}