
#include "world/logic/unit_ai.h"
#include "world/world.h"
#include "world/objects/unit.h"

void UnitAI::tick(World&, Unit& unit) {
    switch(unit.controllerTypeID)
	{
		case Unit::TEAM_CREEP:
		{
			break;
		}

		case Unit::TOWER_BUILDING:
		{
			break;
		}

		case Unit::INANIMATE_OBJECT:
		{
			break;
		}

		default:
			break;
	}
}
