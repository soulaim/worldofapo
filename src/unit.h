#ifndef UNIT_H
#define UNIT_H

#include <vector>
#include <string>
#include <sstream>

#include "location.h"
#include "apomath.h"
#include "lightsource.h"

class Unit : LightSource
{
	public:
		
		enum
		{
			MOVE_LEFT    = 1,
			MOVE_RIGHT   = 2,
			MOVE_FRONT   = 4,
			MOVE_BACK    = 8,
			JUMP         = 16,
			LEAP_LEFT    = 32,
			LEAP_RIGHT   = 64,
			ATTACK_BASIC = 1,
			
			HUMAN_INPUT = 0,
			AI_RABID_ALIEN = 1
		};
		
		
		Unit();
		
		const Location& getPosition() const; // definition demanded by LightSource
		
		Location velocity;
		Location position;

		
		std::string soundInfo;
		
		// This should really be done soon.
//		map<std::string, Location> vectors;
//		map<std::string, int> ints;
		
		int controllerTypeID;
		int hitpoints;
		
		int angle; // integer representation of the angle variable.
		int upangle; // integer representation of how high the unit is looking.
		int keyState; // what is the unit doing right now, represented as player keyboard input
		int mouseButtons;

		int weapon_cooldown;
		int leap_cooldown;
		
		int id;
		int birthTime;
		std::string name;
		
		void setName(const std::string&);
		
		void updateInput(int, int, int, int);
		int getKeyAction(int type);
		int getMouseAction(int type);
		bool human();
		
		std::string copyOrder(int ID);
		void handleCopyOrder(std::stringstream& ss);
		
		float getAngle(ApoMath&);

		Location hitbox_top() const;
		Location hitbox_bot() const;

		bool operator<(const Unit& u) const;

};

#endif

