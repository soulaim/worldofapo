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
		
		Unit();
		
		Location velocity;
		Location position;
		
		// This should really be done soon.
//		map<std::string, Location> vectors;
//		map<std::string, int> ints;
		
		int angle; // integer representation of the angle variable.
		int upangle; // integer representation of how high the unit is looking.
		int keyState; // what is the unit doing right now, represented as player keyboard input

		int weapon_cooldown;
		
		void updateInput(int, int, int);
		
		bool shooting();
		bool movingFront();
		bool movingBack();
		bool movingLeft();
		bool movingRight();
		
		float getAngle(ApoMath&);
		float getUpAnle(ApoMath&);
};

#endif

