#include "unit.h"

#include <iostream>
#include <sstream>
#include <algorithm>

using namespace std;

Unit::Unit():
	controllerTypeID(HUMAN_INPUT),
	hitpoints(1),
	keyState(0),
	mouseButtons(0),
	weapon_cooldown(0),
	leap_cooldown(0),
	last_damage_dealt_by(-1),
	birthTime(0),
	mobility(0)
	
{
	type = OctreeObject::UNIT;
}

void Unit::zeroMovement()
{
	velocity.x = 0;
	velocity.y = 0;
	velocity.z = 0;
}

void Unit::setPosition(const Location& a)
{
	position = a;
}

const Location& Unit::getPosition() const
{
	return position;
}

const Location& Unit::getVelocity() const
{
	return velocity;
}

void Unit::setName(const string& newName)
{
	name = newName;
}

bool Unit::human() const
{
	if(controllerTypeID == HUMAN_INPUT)
		return true;
	return false;
}

float Unit::getAngle(ApoMath& apomath)
{
	return apomath.getDegrees(angle);
}

void Unit::updateInput(int keyState_, int mousex_, int mousey_, int mouseButtons_)
{
	keyState = keyState_;
	mouseButtons = mouseButtons_;
	angle -= mousex_;

	ApoMath math;
	upangle += math.DEGREES_180; // Prevent going full circles when moving camera up or down.

	upangle -= mousey_;

	if(upangle < math.DEGREES_180+5)
		upangle = math.DEGREES_180+5;
	if(upangle > math.DEGREES_360-6)
		upangle = math.DEGREES_360-6;
	upangle -= math.DEGREES_180;
}


int Unit::getKeyAction(int type) const
{
	return (keyState & type);
}

int Unit::getMouseAction(int type) const
{
	return (mouseButtons & type);
}


void Unit::handleCopyOrder(stringstream& ss)
{
	ss >> angle >> upangle >> keyState >>
		position.x >> position.z >> position.y >>
		velocity.x >> velocity.z >> velocity.y >>
		mouseButtons >> weapon_cooldown >> leap_cooldown >>
		controllerTypeID >> hitpoints >> birthTime >>
		id >> weapon >> collision_rule;

	HasProperties::handleCopyOrder(ss);

	for(size_t i = 0; i < weapons.size(); ++i)
	{
		weapons[i].handleCopyOrder(ss);
	}

	// name must be the last element. it is read until the end of the message.
	getline(ss, name);
}

string Unit::copyOrder(int ID) const
{
	stringstream hero_msg;
	hero_msg << "-2 UNIT " << ID << " " << angle << " " << upangle << " " << keyState << " "
		<< position.x << " " << position.z << " " << position.y << " "
		<< velocity.x << " " << velocity.z << " " << velocity.y << " "
		<< mouseButtons << " " << weapon_cooldown << " " << leap_cooldown << " "
		<< controllerTypeID << " " << hitpoints << " " << birthTime << " "
		<< id << " " << weapon << " " << collision_rule << " ";

	hero_msg << HasProperties::copyOrder();

	for(size_t i = 0; i < weapons.size(); ++i)
	{
		hero_msg << weapons[i].copyOrder();
	}

	// name must be the last element.
	hero_msg << name << "#";
	
	return hero_msg.str();
}


FixedPoint Unit::getMobility()
{
	if(mobility & (MOBILITY_STANDING_ON_OBJECT | MOBILITY_STANDING_ON_GROUND))
	{
		if(mobility & MOBILITY_SQUASHED)
			return FixedPoint(1, 6);
		else
			return FixedPoint(1);
	}
	
	return FixedPoint(0);
}


Location Unit::bb_top() const
{
	return Location(position.x+1, position.y+5, position.z+1);
}

Location Unit::bb_bot() const
{
	return Location(position.x-1, position.y, position.z-1);
}

void Unit::collides(OctreeObject& o)
{
	// if one of the objects doesn't want to collide, then don't react.
	if(!(collision_rule & o.collision_rule))
		return;
	
	// if this object doesnt want to be moved by collisions, don't react.
	if(staticObject)
		return;
	
	// if my collision rule is STRING_SYSTEM, make changes to myself accordingly.
	if(collision_rule == CollisionRule::STRING_SYSTEM)
	{
		Location direction = (position - o.position);
		if(direction.length() == FixedPoint(0))
		{
			// unresolvable collision. leave it be.
			return;
		}
		
		direction.normalize();
		direction *= FixedPoint(1, 5);
		velocity += direction;
	}
	else if(collision_rule == CollisionRule::HARD_OBJECT)
	{
		Location myTop = bb_top();
		Location myBot = bb_bot();
		
		Location hisTop = o.bb_top();
		Location hisBot = o.bb_bot();
		
		// is always positive, otherwise there would be no collision.
		FixedPoint y_diff = min(hisTop.y - myBot.y, myTop.y - hisBot.y);
		FixedPoint x_diff = min(hisTop.x - myBot.x, myTop.x - hisBot.x);
		FixedPoint z_diff = min(hisTop.z - myBot.z, myTop.z - hisBot.z);
		
		if(y_diff < x_diff && y_diff < z_diff)
		{
			// least offending axis is y
			// velocity.y  = o.velocity.y * FixedPoint(1, 2) - FixedPoint(25, 1000);
			
			if(hisTop.y < myTop.y)
			{
				velocity.y /= 2;
				
				// i'm on top
				// if bottom object moves, move the top object with it.
				posCorrection.y += y_diff * FixedPoint(9, 20) * 2;
				posCorrection   += o.velocity + o.posCorrection;
				
				mobility |= MOBILITY_STANDING_ON_OBJECT;
			}
			else
			{
				// i'm on bot
				posCorrection.y -= y_diff * FixedPoint(9, 20);
				mobility |= MOBILITY_SQUASHED;
			}
		}
		else if(x_diff < z_diff)
		{
			// least offence by x
			// velocity.x  = o.velocity.x * FixedPoint(1, 2);
			
			if(hisTop.x < myTop.x)
			{
				// i'm on right
				posCorrection.x += x_diff * FixedPoint(9, 20);
			}
			else
			{
				// i'm on left
				posCorrection.x -= x_diff * FixedPoint(9, 20);
			}
		}
		else
		{
			// least offence by z
			// velocity.z  = o.velocity.z * FixedPoint(1, 2);
			
			if(hisTop.z < myTop.z)
			{
				posCorrection.z += z_diff * FixedPoint(9, 20);
			}
			else
			{
				posCorrection.z -= z_diff * FixedPoint(11, 20);
			}
		}
		
	}
}

void Unit::init(World& w)
{
	weapons.push_back(Weapon(&w, this, "data/items/weapon_shotgun.dat"));
	weapons.push_back(Weapon(&w, this, "data/items/weapon_mgun.dat"));
	weapons.push_back(Weapon(&w, this, "data/items/weapon_flame.dat"));
	weapons.push_back(Weapon(&w, this, "data/items/weapon_railgun.dat"));
	
	weapon = 0;
}

void Unit::switchWeapon(unsigned x)
{
	if(x <= 0 || x > weapons.size())
		return;
	weapon = x-1;
}

