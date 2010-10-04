#ifndef UNIT_H
#define UNIT_H

#include <vector>
#include <string>

#include "location.h"
#include "apomath.h"

using namespace std;

class Unit
{
	public:
		
		enum
		{
			MOVE_FRONT   = 4,
			MOVE_BACK    = 8,
			MOVE_LEFT    = 1,
			MOVE_RIGHT   = 2,
			ATTACK_BASIC = 1
		};
		
		Unit();
		
		Location velocity;
		Location position;
		
		// This should really be done soon.
//		map<std::string, Location> vectors;
//		map<std::string, int> ints;
		
		int angle; // integer representation of the angle variable.
		int keyState; // what is the unit doing right now, represented as player keyboard input
		int mouseButton;
		
		void updateInput(int, int, int, int);
		
		int getKeyAction(int type);
		int getMouseAction(int type);
		
		float getAngle(ApoMath&);
		std::string copyOrder(int ID);
};

#endif

