
#include "world/world.h"
#include "graphics/visualworld.h"

#include <map>

using namespace std;


void getTurnValues(Unit& me, Unit& target, int& best_angle, int& best_upangle)
{
	// turn towards the human unit until facing him. then RUSH FORWARD!
	Location direction = target.getEyePosition() - me.getEyePosition();
	
	if(direction.length() == 0)
	{
		// wtf, same position as my target? :G
		return;
	}
	
	direction.normalize();
	
	Location myDirection;
	
	int hypo_angle = me.angle;
	int hypo_upangle = me.upangle;
	bool improved = true;
	FixedPoint best_error = FixedPoint(1000000);
	
	while(improved)
	{
		improved = false;
		hypo_angle += 5;
		myDirection.z = ApoMath::getSin(hypo_angle);
		myDirection.x = ApoMath::getCos(hypo_angle);
		
		FixedPoint error = (myDirection.x * 100 - direction.x * 100).squared() + (myDirection.z * 100 - direction.z * 100).squared();
		if(error < best_error)
		{
			best_error = error;
			improved = true;
		}
		else
		{
			hypo_angle -= 10;
			myDirection.z = ApoMath::getSin(hypo_angle);
			myDirection.x = ApoMath::getCos(hypo_angle);
			
			error = (myDirection.x * 100 - direction.x * 100).squared() + (myDirection.z * 100 - direction.z * 100).squared();
			if(error < best_error)
			{
				best_error = error;
				improved = true;
			}
			else
				hypo_angle += 5;
		}
	}
	
	best_error = FixedPoint(1000000);
	improved = true;
	while(improved)
	{
		improved = false;
		hypo_upangle += 5;
		myDirection.y = ApoMath::getSin(hypo_upangle);
		FixedPoint error = (direction.y * 100 + myDirection.y * 100).squared();
		if(error < best_error)
		{
			best_error = error;
			improved = true;
		}
		else
		{
			hypo_upangle -= 10;
			myDirection.y = ApoMath::getSin(hypo_upangle);
			error = (direction.y * 100 + myDirection.y * 100).squared();
			if(error < best_error)
			{
				best_error = error;
				improved = true;
			}
			else
				hypo_upangle += 5;
		}
	}
	
	best_angle = hypo_angle;
	best_upangle = hypo_upangle;
}


void World::AI_RabidAlien(Unit& unit)
{
	FixedPoint bestSquaredDistance = FixedPoint(200 * 200);
	int unitID = -1;
	int myLeaderID = -1;
	int my_team = unit["TEAM"];
	
	if(unit["L"] == -1)
		unit["L"] = unit.id;
	
	if(unit.intVals[unit.weapons[unit.weapon].strVals["AMMUNITION_TYPE"]] == 0)
	{
		if(unit.weapon > 1)
			unit.weapon--;
		else
		{
			unit.strVals["DAMAGED_BY"] = "voluntary suicide";
			unit.hitpoints = -1;
			unit.last_damage_dealt_by = unit.id;
		}
	}
	
	// find the nearest human controlled unit + some "leader" for my team
	if( (unit.birthTime + currentWorldFrame) % 70 == 0)
	{
		for(map<int, Unit>::iterator it = units.begin(), et = units.end(); it != et; ++it)
		{
			// don't let AI get worked up with units that aren't there.
			if(!it->second.exists())
				continue;
			
			if(it->second["TEAM"] != my_team)
			{
				FixedPoint tmp_dist = (it->second.position - unit.position).lengthSquared();
				if( tmp_dist < bestSquaredDistance )
				{
					bestSquaredDistance = tmp_dist;
					unitID = it->first;
				}
			}
			else
			{
				if(myLeaderID == -1)
				{
					myLeaderID = it->first;
				}
			}
		}
		
		unit["T"] = unitID;
		unit["L"] = myLeaderID;
		
		if(unitID == -1)
		{
			// try look
		}
		
	}
	else
	{
		unitID = unit["T"];
		myLeaderID = unit["L"];
		
		if(units.find(unitID) == units.end())
		{
			unit["T"] = -1;
			unitID = -1;
		}
		
		// there is always some leader
		if(units.find(myLeaderID) == units.end())
		{
			unit["L"] = -1;
			myLeaderID = unit.id;
		}
	}
	
	// if no opponent is near, gather to my leader!
	if(unitID == -1)
	{
		
		if(myLeaderID == unit.id)
		{
			// nothing to do :G
			unit.updateInput(0, 0, 0, 0);
		}
		else
		{
			int mousex, mousey;
			getTurnValues(unit, units[myLeaderID], mousex, mousey);
			
			unit.angle = mousex;
			unit.upangle = mousey;
			
			if( (units.find(myLeaderID)->second.position - unit.position).lengthSquared() < FixedPoint(1000) )
			{
				unit.updateInput(0, 0, 0, 0);
			}
			else
			{
				unit.updateInput(Unit::MOVE_FRONT, 0, 0, 0);
			}
		}
		
		return;
	}
	
	
	bestSquaredDistance = (units.find(unitID)->second.position - unit.position).lengthSquared();
	
	int best_angle, best_upangle;
	getTurnValues(unit, units[unitID], best_angle, best_upangle);
	
	int keyState = 0;
	int mousex = 0;
	int mousey = 0;
	int mousebutton = 0;
	unit.angle = best_angle;
	unit.upangle = best_upangle;
	
	if(bestSquaredDistance < FixedPoint(1000))
		keyState |= Unit::MOVE_BACK | Unit::LEAP_RIGHT;
	else
		keyState |= Unit::MOVE_FRONT;
	
	if( ((currentWorldFrame + unit.birthTime) % 140) < 20)
	{
		keyState |= Unit::JUMP;
	}
	
	if( ((currentWorldFrame + unit.birthTime) % 140) > 50 )
	{
		// mousex += ( ((unit.birthTime + currentWorldFrame) * 23) % 200) - 100;
		mousebutton = 1;
	}
	
	unit.updateInput(keyState, mousex, mousey, mousebutton);
}




void World::AI_TeamCreep(Unit& unit)
{
	FixedPoint bestSquaredDistance = FixedPoint(200 * 200);
	int unitID = -1;
	int myLeaderID = -1;
	int my_team = unit["TEAM"];
	
	if(unit["L"] == -1)
		unit["L"] = unit.id;
	
	if(unit.intVals[unit.weapons[unit.weapon].strVals["AMMUNITION_TYPE"]] == 0)
	{
		if(unit.weapon > 1)
			unit.weapon--;
		else
		{
			unit.strVals["DAMAGED_BY"] = "voluntary suicide";
			unit.hitpoints = -1;
			unit.last_damage_dealt_by = unit.id;
		}
	}
	
	// find the nearest human controlled unit + some "leader" for my team
	if( (unit.birthTime + currentWorldFrame) % 70 == 0)
	{
		for(map<int, Unit>::iterator it = units.begin(), et = units.end(); it != et; ++it)
		{
			if(it->second["TEAM"] != my_team)
			{
				if(it->second.controllerTypeID == Unit::BASE_BUILDING)
				{
					myLeaderID = it->first;
				}
				
				FixedPoint tmp_dist = (it->second.position - unit.position).lengthSquared();
				if( tmp_dist < bestSquaredDistance )
				{
					bestSquaredDistance = tmp_dist;
					unitID = it->first;
				}
			}
			
			/*
			else
			{
				if(myLeaderID == -1)
				{
					myLeaderID = it->first;
				}
			}
			*/
		}
		
		unit["T"] = unitID;
		unit["L"] = myLeaderID;
		
		if(unitID == -1)
		{
			// try look
		}
		
	}
	else
	{
		unitID = unit["T"];
		myLeaderID = unit["L"];
		
		if(units.find(unitID) == units.end())
		{
			unit["T"] = -1;
			unitID = -1;
		}
		
		// there is always some leader
		if(units.find(myLeaderID) == units.end())
		{
			unit["L"] = -1;
			myLeaderID = unit.id;
		}
	}
	
	// if no opponent is near, gather to my leader!
	if(unitID == -1)
	{
		
		if(myLeaderID == unit.id)
		{
			// nothing to do :G
			unit.updateInput(0, 0, 0, 0);
		}
		else
		{
			int mousex, mousey;
			getTurnValues(unit, units[myLeaderID], mousex, mousey);
			
			unit.angle = mousex;
			unit.upangle = mousey;
			
			if( (units.find(myLeaderID)->second.position - unit.position).lengthSquared() < FixedPoint(1000) )
			{
				unit.updateInput(0, 0, 0, 0);
			}
			else
			{
				unit.updateInput(Unit::MOVE_FRONT, 0, 0, 0);
			}
		}
		
		return;
	}
	
	
	bestSquaredDistance = (units.find(unitID)->second.position - unit.position).lengthSquared();
	
	int best_angle, best_upangle;
	getTurnValues(unit, units[unitID], best_angle, best_upangle);
	
	int keyState = 0;
	int mousex = 0;
	int mousey = 0;
	int mousebutton = 0;
	unit.angle = best_angle;
	unit.upangle = best_upangle;
	
	if(bestSquaredDistance < FixedPoint(1000))
		keyState |= Unit::MOVE_BACK | Unit::LEAP_RIGHT;
	else
		keyState |= Unit::MOVE_FRONT;
	
	if( ((currentWorldFrame + unit.birthTime) % 140) < 20)
	{
		keyState |= Unit::JUMP;
	}
	
	if( ((currentWorldFrame + unit.birthTime) % 140) > 50 )
	{
		// mousex += ( ((unit.birthTime + currentWorldFrame) * 23) % 200) - 100;
		mousebutton = 1;
	}
	
	unit.updateInput(keyState, mousex, mousey, mousebutton);
}




void World::AI_TowerBuilding(Unit& unit)
{
	AI_RabidAlien(unit);
	unit.resetAmmoCount(); // towers never run out of ammo.
	
	if(unit["T"] != -1)
		unit.updateInput(0, 0, 0, 1); // towers can shoot all the fucking time.
	else
		unit.updateInput(0, 0, 0, 0);
}




void World::AI_BaseBuilding(Unit& u)
{
	if(currentWorldFrame % 500 == 10)
	{
		int num = (currentWorldFrame % 1731) % 5;
		Location spawn_pos = u.position + Location(num - 3, ((num + 2) % 5) - 3, ((num + 4) % 5) - 3);
		
		// void addAIUnit(int id, const Location& pos, int team, VisualWorld::ModelType model_type, int controllerType, float scale, const std::string& name)
		int team = u["TEAM"];
		if(team == 0)
			addAIUnit(unitIDgenerator.nextID(), spawn_pos, team, VisualWorld::ModelType::ZOMBIE_MODEL, Unit::TEAM_CREEP, FixedPoint(1), "Recruit", 4, 1, 500);
		else
			addAIUnit(unitIDgenerator.nextID(), spawn_pos, team, VisualWorld::ModelType::ZOMBIE_MODEL, Unit::TEAM_CREEP, FixedPoint(1), "Recruit", 4, 1, 500);
	}
}