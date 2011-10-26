#ifndef UNIT_H
#define UNIT_H

#include "world/item.h"
#include "misc/vec3.h"
#include "physics/octree_object.h"
#include "physics/movable_object.h"
#include "world/objects/inventory.h"
#include "world/objects/itempicker.h"

#include <vector>
#include <string>
#include <sstream>
#include <memory>

class ApoMath;

class Unit : public HasProperties, public OctreeObject
{
    friend class UnitTicker;
    friend class MonsterCreator;

    mutable Location bb_top_;
    mutable Location bb_bot_;

    Inventory inventory;
    HasProperties stats;

public:

	enum
	{
		MOVE_LEFT    = 1,
		MOVE_RIGHT   = 2,
		MOVE_FRONT   = 4,
		MOVE_BACK    = 8,
		JUMP         = 16,
		INTERACT     = 32,
		// LEAP_RIGHT   = 64,
		RELOAD       = 1<<16,
		WEAPON1      = 1<<20,
		WEAPON2      = 1<<21,
		WEAPON3      = 1<<22,
		WEAPON4      = 1<<23,
		WEAPON5      = 1<<24,

		MOUSE_LEFT = 1,
		MOUSE_RIGHT = 2,

		HUMAN_INPUT = 0,
        AI_ALIEN = 1,
		INANIMATE_OBJECT = 2,

		MOBILITY_CLEAR_VALUE = 0,
		MOBILITY_STANDING_ON_OBJECT = 1,
		MOBILITY_SQUASHED = 2,
		MOBILITY_STANDING_ON_GROUND = 4
	};


	Unit();

    void preTick();
    void pushPickableItem(WorldItem& item);
    ItemPicker itemPick;

    // movement
	void accelerateForward();
	void accelerateBackward();
	void accelerateRight();
	void accelerateLeft();
	void jump();

    // inventory interface
    void activateCurrentItemPrimary(World&);
    void activateCurrentItemSecondary(World&);
    void activateCurrentItemReload(World&);

	Location getEyePosition() const;

	void processInput(World&);
	void regenerate();

	void landingDamage();

	bool exists();
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

    void increaseStat(World& world, int increasedStat);
	int getMaxHP() const;

	void updateMobility();
	const FixedPoint& getMobility() const;
    const Inventory& getInventory() const;
    const ItemPicker& getItemPicker() const;
    const HasProperties& getStats() const;
    HasProperties& getStatsEditor();

	std::string soundInfo;
	int controllerTypeID;

	int model_type; // the visual representation's ID

	int keyState; // what is the unit doing right now, represented as player keyboard input
	int mouseButtons;

	int mouse_x_minor; // to support mouse sensitivity
	int mouse_y_minor; // to support mouse sensitivity

	int last_damage_dealt_by;
	int birthTime;

	int mobility;
	FixedPoint mobility_val;

	FixedPoint scale; // unit size for collision detection etc.

	std::string name;

	void setName(const std::string&);

	void updateKeyState(int);
	void updateMouseMove(int, int);
	void updateMousePress(int);

	int getKeyAction(int type) const;
	int getMouseAction(int type) const;
	bool human() const;

	void takeDamage(int);

	std::string copyOrder(int ID) const;
	void handleCopyOrder(std::stringstream& ss);

	int getModifier(const std::string& attribute) const;
	bool gainExperience(World&, int);
    void levelUp(World&);

	float getAngle();

	const Location& bb_top() const;
	const Location& bb_bot() const;
	void collides(OctreeObject&);

	void init();
	void switchWeapon(World&, unsigned);
    void destroyInventoryItem(WorldItem* item);

	bool operator<(const Unit& u) const;
};

#endif

