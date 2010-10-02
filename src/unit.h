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
		int angle; // integer representation of the angle variable.
		int keyState; // what is the unit doing right now, represented as player keyboard input
		
		void updateInput(int, int, int);
		
		bool movingFront();
		bool movingBack();
		
		float getAngle(ApoMath&);
};

#endif

