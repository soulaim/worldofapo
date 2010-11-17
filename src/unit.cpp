
#include "unit.h"
#include <iostream>
#include <sstream>

Unit::Unit():
	controllerTypeID(HUMAN_INPUT),
	hitpoints(1000),
	keyState(0),
	mouseButtons(0),
	weapon_cooldown(0),
	leap_cooldown(0),
	last_damage_dealt_by(-1),
	birthTime(0)
	
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

void Unit::setName(const std::string& newName)
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


void Unit::handleCopyOrder(std::stringstream& ss)
{
	ss >> angle >> upangle >> keyState >>
		position.x >> position.z >> position.y >>
		velocity.x >> velocity.z >> velocity.y >>
		mouseButtons >> weapon_cooldown >> leap_cooldown >> controllerTypeID >> hitpoints >> birthTime >> id;
	
	// name must be the last element. it is read until the end of the message.
	getline(ss, name);
}

std::string Unit::copyOrder(int ID)
{
	std::stringstream hero_msg;
	hero_msg << "-2 UNIT " << ID << " " << angle << " " << upangle << " " << keyState << " "
		<< position.x << " " << position.z << " " << position.y << " "
		<< velocity.x << " " << velocity.z << " " << velocity.y << " "
		<< mouseButtons << " " << weapon_cooldown << " " << leap_cooldown << " " << controllerTypeID << " " << hitpoints << " " << birthTime << " " << id << " ";
	
	// name must be the last element.
	hero_msg << name << "#";
	
	return hero_msg.str();
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
	if (o.type == OctreeObject::UNIT)
	{
		Unit& u = (Unit&) o;
		Location direction = (position - u.position);
		if(direction.length() == FixedPoint(0))
		{
			// unresolvable collision. leave it be.
			return;
		}
		direction.normalize();
		direction *= FixedPoint(1, 5);
		
		velocity += direction;
	}
	else if (o.type == OctreeObject::MEDIKIT)
	{
		hitpoints += 100;
	}
}

void Unit::init(World& w)
{
	weapons.push_back(new Weapon(w, *this, "data/items/weapon_flame.dat"));
	weapons.push_back(new Weapon(w, *this, "data/items/weapon_mgun.dat"));
	weapons.push_back(new Weapon(w, *this, "data/items/weapon_shotgun.dat"));
	weapons.push_back(new Weapon(w, *this, "data/items/weapon_railgun.dat"));
	
	// TODO ALERT: Should keep track of the INDEX of the current weapon. Not the pointer. More difficult to transmit in world-copy otherwise (which is not done at all currently).
	weapon = weapons[0];
}

void Unit::switchWeapon(unsigned x) {
	if (x > weapons.size())
		return;
	weapon = weapons[x-1];
}

