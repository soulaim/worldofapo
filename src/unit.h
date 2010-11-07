#ifndef UNIT_H
#define UNIT_H

#include <vector>
#include <string>
#include <sstream>
#include <memory>

#include "world.h"
#include "weapon.h"
#include "location.h"
#include "apomath.h"
#include "lightsource.h"
#include "octree_object.h"
#include "movable_object.h"
#include "item.h"

class Unit : public MovableObject, HasProperties, public OctreeObject
{
	friend class World;
	
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
			WEAPON1      = 1<<20,
			WEAPON2      = 1<<21,
			WEAPON3      = 1<<22,
			ATTACK_BASIC = 1,
			
			HUMAN_INPUT = 0,
			AI_RABID_ALIEN = 1
		};
		
		
		Unit();
		
		void zeroMovement();
		void setPosition(const Location&);
		const Location& getPosition() const;
		const Location& getVelocity() const;
		
		// const Location& getPosition() const; // definition demanded by LightSource
		
		std::string soundInfo;
		
		// TODO: Should have a proper inventory system perhaps.
		Weapon* weapon;
		std::vector<Weapon*> weapons;
		
		int controllerTypeID;
		int hitpoints;
		
		int keyState; // what is the unit doing right now, represented as player keyboard input
		int mouseButtons;

		int weapon_cooldown;
		int leap_cooldown;
		
		int last_damage_dealt_by;
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

		Location bb_top() const;
		Location bb_bot() const;
		void collides(OctreeObject&);

		void init(World& w);
		void switchWeapon(unsigned);

		bool operator<(const Unit& u) const;
};

#endif

