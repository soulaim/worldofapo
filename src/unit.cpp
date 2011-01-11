#include "unit.h"

#include "apomath.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

using namespace std;

Unit::Unit():
	controllerTypeID(HUMAN_INPUT),
	hitpoints(500),
	keyState(0),
	mouseButtons(0),
	mouse_x_minor(0),
	mouse_y_minor(0),
	weapon_cooldown(0),
	leap_cooldown(0),
	last_damage_dealt_by(-1),
	birthTime(0),
	mobility(0)
{
	type = OctreeObject::UNIT;
	
	intVals["MASS"] = 1000;
	
	// create attributes
	intVals["STR"] = -666;
	intVals["DEX"] = -666;
	intVals["VIT"] = -666;
	
	intVals["WIS"] = -666;
	intVals["INT"] = -666;
	
	scale = FixedPoint(1);
}

void Unit::setDefaultMonsterAttributes()
{
	// set attributes
	intVals["STR"] = 4;
	intVals["DEX"] = 4;
	intVals["VIT"] = 4;
	
	intVals["WIS"] = 4;
	intVals["INT"] = 4;
}

void Unit::setDefaultPlayerAttributes()
{
	// set attributes
	intVals["STR"] = 4;
	intVals["DEX"] = 4;
	intVals["VIT"] = 4;
	
	intVals["WIS"] = 4;
	intVals["INT"] = 4;
	
	intVals["REGEN"] = 10;
}

int Unit::getModifier(const string& attribute) const
{
	auto it = intVals.find(attribute);
	if(it == intVals.end())
	{
		throw std::logic_error("Asking for an attribute that doesn't exist: " + attribute);
		return 0;
	}
	
	assert(it->second != -666);
	
	return 6 + it->second;
}

int Unit::getMaxHP() const
{
	return 100 * getModifier("STR");
}

void Unit::levelUp()
{
	intVals["DEX"]++;
}

void Unit::takeDamage(int damage)
{
	hitpoints -= damage;
	intVals["D"] += damage;
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
	/*
	int& no_orders = intVals["NO_ORDERS"];
	
	if(no_orders > 0)
	{
		// no actions allowed!
		--no_orders;
		keyState = 0;
		mouseButtons = 0;
		return;
	}
	*/
	
	static ApoMath math;
	
	int changey = mousey_;
	int changex = mousex_;
	
	int x_major_change = changex / 1000;
	int y_major_change = changey / 1000;
	
	mouse_x_minor += changex - x_major_change * 1000;
	mouse_y_minor += changey - y_major_change * 1000;
	
	int xmc2 = mouse_x_minor / 1000;
	int ymc2 = mouse_y_minor / 1000;
	mouse_x_minor -= xmc2 * 1000;
	mouse_y_minor -= ymc2 * 1000;
	
	x_major_change += xmc2;
	y_major_change += ymc2;
	
	keyState = keyState_;
	mouseButtons = mouseButtons_;
	angle -= x_major_change;
	
	upangle += math.DEGREES_180; // Prevent going full circles when moving camera up or down.
	upangle -= y_major_change;
	
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
		id >> weapon >> collision_rule >> scale >>
		mouse_x_minor >> mouse_y_minor;
	
	HasProperties::handleCopyOrder(ss);
	
	assert(weapons.size() == 5);
	
	for(size_t i = 0; i < weapons.size(); ++i)
	{
		weapons[i].handleCopyOrder(ss);
	}
	
	ss >> name;
}

string Unit::copyOrder(int ID) const
{
	stringstream hero_msg;
	hero_msg << "-2 UNIT " << ID << " " << angle << " " << upangle << " " << keyState << " "
		<< position.x << " " << position.z << " " << position.y << " "
		<< velocity.x << " " << velocity.z << " " << velocity.y << " "
		<< mouseButtons << " " << weapon_cooldown << " " << leap_cooldown << " "
		<< controllerTypeID << " " << hitpoints << " " << birthTime << " "
		<< id << " " << weapon << " " << collision_rule << " " << scale << " "
		<< mouse_x_minor << " " << mouse_y_minor << " ";
	
	hero_msg << HasProperties::copyOrder();
	
	for(size_t i = 0; i < weapons.size(); ++i)
	{
		hero_msg << weapons[i].copyOrder();
	}
	
	// name must be the last element.
	hero_msg << " " << name << "#";
	
	return hero_msg.str();
}


FixedPoint Unit::getMobility()
{
	if(mobility & (MOBILITY_STANDING_ON_OBJECT | MOBILITY_STANDING_ON_GROUND))
	{
		int dex_modifier = getModifier("DEX");
		
		if(mobility & MOBILITY_SQUASHED)
			return FixedPoint(1, 6) * FixedPoint(dex_modifier, 10);
		else
			return FixedPoint(1) * FixedPoint(dex_modifier, 10);
	}
	
	return FixedPoint(0);
}


Location Unit::bb_top() const
{
	return Location(position.x + scale * 1, position.y + scale * 5, position.z + scale * 1);
}

Location Unit::bb_bot() const
{
	return Location(position.x - scale * 1, position.y, position.z - scale * 1);
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
				posCorrection.z -= z_diff * FixedPoint(9, 20);
			}
		}
		
	}
}

void Unit::init()
{
	weapons.clear();
	weapons.push_back(Weapon("data/items/weapon_shotgun.dat"));
	weapons.push_back(Weapon("data/items/weapon_mgun.dat"));
	weapons.push_back(Weapon("data/items/weapon_flame.dat"));
	weapons.push_back(Weapon("data/items/weapon_railgun.dat"));
	weapons.push_back(Weapon("data/items/weapon_rocket.dat"));
	
	for(size_t i=0; i<weapons.size(); i++)
	{
		intVals[weapons[i].strVals["AMMUNITION_TYPE"]] = 2000 / weapons[i].intVals["AMMO_VALUE"];
	}
	
	weapon = 2;
}

void Unit::switchWeapon(unsigned x)
{
	if(x <= 0 || x > weapons.size())
		return;
	weapon = x-1;
}

