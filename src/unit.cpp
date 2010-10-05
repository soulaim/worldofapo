
#include "unit.h"
#include <iostream>
#include <sstream>

using namespace std;

Unit::Unit():
	angle(0),
	upangle(0),
	keyState(0),
	weapon_cooldown(0),
	leap_cooldown(0)
{
}

float Unit::getAngle(ApoMath& apomath)
{
	return apomath.getRad(angle);
}

void Unit::updateInput(int keyState_, int mousex_, int mousey_, int mouseButton_)
{
	keyState = keyState_;
	mouseButton = mouseButton_;
	angle -= mousex_;

	static ApoMath dorka2; // TODO: EIS VOISOASDIASD;
	dorka2.init(3000);
	upangle += dorka2.DEGREES_180; // Prevent going full circles when moving camera up or down.

	upangle -= mousey_;

	if(upangle < dorka2.DEGREES_180+5)
		upangle = dorka2.DEGREES_180+5;
	if(upangle > dorka2.DEGREES_360-6)
		upangle = dorka2.DEGREES_360-6;
	upangle -= dorka2.DEGREES_180;
//	cerr << "upangle:" << upangle << "\n";
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
	ss >> angle >> upangle >> keyState >> position.x.number >> position.z.number >> position.y.number >> velocity.x.number >> velocity.z.number >> velocity.y.number >> mouseButton >> weapon_cooldown >> leap_cooldown;
}

string Unit::copyOrder(int ID)
{
	stringstream hero_msg;
	hero_msg << "-2 UNIT " << ID << " " << angle << " " << upangle << " " << keyState << " " << position.x.number << " " << position.z.number << " " << position.y.number << " " << velocity.x.number << " " << velocity.z.number << " " << velocity.y.number << " " << mouseButton << " " << weapon_cooldown << " " << leap_cooldown << "#";
	
	return hero_msg.str();
}

