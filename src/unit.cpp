
#include "unit.h"
#include <iostream>
#include <sstream>

using namespace std;

Unit::Unit():
	controllerTypeID(HUMAN_INPUT),
	hitpoints(1000),
	angle(0),
	upangle(0),
	keyState(0),
	weapon_cooldown(0),
	leap_cooldown(0)
{
}


void Unit::setName(const string& newName)
{
	name = newName;
}

bool Unit::human()
{
	if(controllerTypeID == HUMAN_INPUT)
		return true;
	return false;
}

float Unit::getAngle(ApoMath& apomath)
{
	return apomath.getDegrees(angle);
}

void Unit::updateInput(int keyState_, int mousex_, int mousey_, int mouseButton_)
{
	keyState = keyState_;
	mouseButton = mouseButton_;
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


int Unit::getKeyAction(int type)
{
	return (keyState & type);
}

int Unit::getMouseAction(int type)
{
	return (mouseButton & type);
}


void Unit::handleCopyOrder(stringstream& ss)
{
	ss >> angle >> upangle >> keyState >> position.x.number >> position.z.number >> position.y.number >> velocity.x.number >> velocity.z.number >> velocity.y.number >> mouseButton >> weapon_cooldown >> leap_cooldown >> controllerTypeID >> hitpoints >> birthTime >> id;
	
	// name must be the last element. it is read until the end of the message.
	getline(ss, name);
}

string Unit::copyOrder(int ID)
{
	stringstream hero_msg;
	hero_msg << "-2 UNIT " << ID << " " << angle << " " << upangle << " " << keyState << " " << position.x.number << " " << position.z.number << " " << position.y.number << " " << velocity.x.number << " " << velocity.z.number << " " << velocity.y.number << " " << mouseButton << " " << weapon_cooldown << " " << leap_cooldown << " " << controllerTypeID << " " << hitpoints << " " << birthTime << " " << id << " ";
	
	// name must be the last element.
	hero_msg << name << "#";
	
	return hero_msg.str();
}

