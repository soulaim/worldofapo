
#include "unit.h"
#include <iostream>
#include <sstream>

Unit::Unit(): angle(0), keyState(0)
{
}

float Unit::getAngle(ApoMath& apomath)
{
	return apomath.getRad(angle);
}

void Unit::updateInput(int keyState_, int mousex_, int mousey_, int mouseButton_)
{
	//  keyState ^= keyState_;
	keyState = keyState_;
	mouseButton = mouseButton_;
	angle -= mousex_;
}


int Unit::getKeyAction(int type)
{
	return (keyState & type);
}

int Unit::getMouseAction(int type)
{
	return (mouseButton & type);
}


string Unit::copyOrder(int ID)
{
	stringstream hero_msg;
	hero_msg << "-2 UNIT " << ID << " " << angle << " " << keyState << " " << position.x.number << " " << position.y.number << " " << position.h.number << " " << velocity.x.number << " " << velocity.y.number << " " << velocity.h.number << " " << mouseButton << "#";
	
	return hero_msg.str();
}