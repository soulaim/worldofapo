#ifndef UNIT_H
#define UNIT_H

#include <vector>
#include <string>
#include <sstream>
#include <memory>

#include "weapon.h"
#include "vec3.h"
#include "octree_object.h"
#include "movable_object.h"
#include "item.h"

class ApoMath;

class Unit : public HasProperties, public OctreeObject
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
		RELOAD       = 1<<16,
		WEAPON1      = 1<<20,
		WEAPON2      = 1<<21,
		WEAPON3      = 1<<22,
		WEAPON4      = 1<<23,
		WEAPON5      = 1<<24,
		
		MOUSE_LEFT = 1,
		MOUSE_RIGHT = 2,
		
		HUMAN_INPUT = 0,
		AI_RABID_ALIEN = 1,
		INANIMATE_OBJECT = 2,
		
		MOBILITY_CLEAR_VALUE = 0,
		MOBILITY_STANDING_ON_OBJECT = 1,
		MOBILITY_SQUASHED = 2,
		MOBILITY_STANDING_ON_GROUND = 4
	};
	
	
	Unit();
	
	void accelerateForward();
	void accelerateBackward();
	void accelerateRight();
	void accelerateLeft();
	void leapLeft();
	void leapRight();
	void jump();
	
	void processInput();
	void regenerate();
	
	void landingDamage();
	bool hasSupportUnderFeet() const;
	bool hasGroundUnderFeet() const;
	void applyFriction();
	void applyGravity();
	void postTick();
	void tick(const FixedPoint& yy_val);
	
	void zeroMovement();
	void setPosition(const Location&);
	const Location& getPosition() const;
	const Location& getVelocity() const;
	
	int getMaxHP() const;
	
	void updateMobility();
	const FixedPoint& getMobility() const;
	
	std::string soundInfo;
	
	// TODO: Should have a proper inventory system perhaps.
	size_t weapon;
	std::vector<Weapon> weapons;
	
	int controllerTypeID;
	int hitpoints;
	
	int keyState; // what is the unit doing right now, represented as player keyboard input
	int mouseButtons;
	
	int mouse_x_minor; // to support mouse sensitivity
	int mouse_y_minor; // to support mouse sensitivity
	
	int weapon_cooldown;
	int leap_cooldown;
	
	int last_damage_dealt_by;
	int birthTime;
	
	int mobility;
	FixedPoint mobility_val;
	
	FixedPoint scale; // unit size for collision detection etc.
	
	std::string name;
	
	void setName(const std::string&);
	
	void updateInput(int, int, int, int);
	int getKeyAction(int type) const;
	int getMouseAction(int type) const;
	bool human() const;
	
	void takeDamage(int);
	
	std::string copyOrder(int ID) const;
	void handleCopyOrder(std::stringstream& ss);
	
	// default attributes
	void setDefaultMonsterAttributes();
	void setDefaultPlayerAttributes();
	
	int getModifier(const std::string& attribute) const;
	void levelUp(); // very much a place holder! TODO:
	
	float getAngle();
	
	Location bb_top() const;
	Location bb_bot() const;
	void collides(OctreeObject&);
	
	void init();
	void switchWeapon(unsigned);
	
	bool operator<(const Unit& u) const;
};

#endif

