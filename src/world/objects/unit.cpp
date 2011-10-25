#include "world/objects/unit.h"
#include "misc/apomath.h"
#include "world/world.h"
#include "itempicker.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

using namespace std;


Location bump(int x)
{
	Location res;
	res.x = FixedPoint( ((x * 4217) % 200) - 100, 100 );
	res.y = 0; // FixedPoint( ((x * 8423) % 200) - 100, 100 );
	res.z = FixedPoint( ((x * 2489) % 200) - 100, 100 );
	return res;
}


Unit::Unit():
	controllerTypeID(HUMAN_INPUT),
	keyState(0),
	mouseButtons(0),
	mouse_x_minor(0),
	mouse_y_minor(0),
	last_damage_dealt_by(-1),
	birthTime(0),
	mobility(0)
{
	type = OctreeObject::UNIT;
	intVals["MASS"] = 1000;
    init();

	scale = FixedPoint(1);
}

void Unit::activateCurrentItemPrimary(World& world) {
    inventory.useActiveItemPrimary(world, *this);
}

void Unit::activateCurrentItemSecondary(World& world) {
    inventory.useActiveItemSecondary(world, *this);
}

void Unit::activateCurrentItemReload(World& world) {
    inventory.reloadAction(world, *this);
}

const Inventory& Unit::getInventory() const {
    return this->inventory;
}

const ItemPicker& Unit::getItemPicker() const {
    return this->itemPick;
}

void Unit::preTick() {
    this->itemPick.reset();
}

void Unit::pushPickableItem(WorldItem& item) {
    this->itemPick.pushItem( item, (item.position - this->position).lengthSquared() );
}

// TODO: Hide this functionality behind some unit ticker class
void Unit::accelerateForward()
{
	FixedPoint mobility_scale = FixedPoint(24, 100) * getMobility();
	velocity.x += ApoMath::getCos(angle) * mobility_scale;
	velocity.z += ApoMath::getSin(angle) * mobility_scale;
}

// TODO: Hide this functionality behind some unit ticker class
void Unit::accelerateBackward()
{
	FixedPoint mobility_scale = FixedPoint(11, 100) * getMobility();
	velocity.x -= ApoMath::getCos(angle) * mobility_scale;
	velocity.z -= ApoMath::getSin(angle) * mobility_scale;
}

// TODO: Hide this functionality behind some unit ticker class
void Unit::accelerateLeft()
{
	FixedPoint mobility_scale = FixedPoint(18, 100) * getMobility();
	int dummy_angle = angle - ApoMath::DEGREES_90;

	velocity.x -= ApoMath::getCos(dummy_angle) * mobility_scale;
	velocity.z -= ApoMath::getSin(dummy_angle) * mobility_scale;
}

// TODO: Hide this functionality behind some unit ticker class
void Unit::accelerateRight()
{
	FixedPoint mobility_scale = FixedPoint(18, 100) * getMobility();
	int dummy_angle = angle + ApoMath::DEGREES_90;

	velocity.x -= ApoMath::getCos(dummy_angle) * mobility_scale;
	velocity.z -= ApoMath::getSin(dummy_angle) * mobility_scale;
}


// TODO: Hide this functionality behind some unit ticker class
void Unit::jump()
{
	if(getMobility() > FixedPoint(0))
	{
		soundInfo = "jump";
		velocity.y = FixedPoint(1100, 1000);
	}
}

void Unit::processInput(World& world)
{
	soundInfo = "";

	if(getKeyAction(Unit::WEAPON1))
	{
		switchWeapon(world, 0);
	}

	if(getKeyAction(Unit::WEAPON2))
	{
		switchWeapon(world, 1);
	}

	if(getKeyAction(Unit::WEAPON3))
	{
		switchWeapon(world, 2);
	}

	if(getKeyAction(Unit::WEAPON4))
	{
		switchWeapon(world, 3);
	}

	if(getKeyAction(Unit::WEAPON5))
	{
		switchWeapon(world, 4);
	}


	if(getKeyAction(Unit::MOVE_FRONT))
	{
		accelerateForward();
	}

	if(getKeyAction(Unit::MOVE_BACK))
	{
		accelerateBackward();
	}

	if(getKeyAction(Unit::MOVE_LEFT))
	{
		accelerateLeft();
	}

	if(getKeyAction(Unit::MOVE_RIGHT))
	{
		accelerateRight();
	}

	if(getKeyAction(Unit::MOVE_RIGHT | Unit::MOVE_LEFT | Unit::MOVE_FRONT | Unit::MOVE_BACK) && (soundInfo == ""))
		soundInfo = "walk";

	if(getKeyAction(Unit::JUMP))
	{
		jump();
	}

}

void Unit::increaseStat(int increasedStat) {
    cerr << "TODO: Unit::increaseStat() " << increasedStat << endl;
}

Location Unit::getEyePosition() const
{
	return Location(position.x, position.y + (bb_top().y - position.y) * FixedPoint(3, 4), position.z);
}

const HasProperties& Unit::getStats() const {
    return stats;
}

void Unit::destroyInventoryItem(WorldItem* item) {
    inventory.destroyItem(item);
}

HasProperties& Unit::getStatsEditor() {
    return stats;
}

void Unit::regenerate()
{
	int unit_max_hp = getMaxHP();
    int& hitpoints = intVals["HEALTH"];
	if(hitpoints < unit_max_hp)
	{
		int regen = intVals["REGEN"];
		int missing_hp = unit_max_hp-hitpoints;
		hitpoints += (regen<missing_hp)?regen:missing_hp;
	}
}

void Unit::landingDamage()
{
	if(velocity.y < FixedPoint(-12, 10))
	{
		strVals["DAMAGED_BY"] = "falling";

		FixedPoint damage_fp = velocity.y + FixedPoint(12, 10);
		int damage_int = damage_fp.getDesimal() + damage_fp.getInteger() * FixedPoint::FIXED_POINT_ONE;

		if(damage_int < -500)
		{
			// is hitting the ground REALLY HARD. Nothing could possibly survive. Just insta-kill.
			intVals["HEALTH"] = -1;
		}
		else
		{
			velocity.x *= FixedPoint(10, 100);
			velocity.z *= FixedPoint(10, 100);
			takeDamage(damage_int * damage_int / 500);
		}

		// allow deny only after surviving the first hit with ground.
		// not a deny if thrown down a cliff!
		if(intVals["HEALTH"] > 0)
		{
			last_damage_dealt_by = id;
		}
	}
}

void Unit::applyFriction()
{
	FixedPoint friction = FixedPoint(75, 100);
	velocity.x *= friction;
	velocity.z *= friction;
}

void Unit::applyGravity()
{
	// gravity
	velocity.y -= FixedPoint(100,1000);

	// air resistance
	FixedPoint friction = FixedPoint(995, 1000);
	velocity.x *= friction;
	velocity.z *= friction;
}

void Unit::postTick()
{
	mobility = Unit::MOBILITY_CLEAR_VALUE;
	position += posCorrection;
	posCorrection = Location();
}

void Unit::tick(const FixedPoint& yy_val)
{
    WorldItem* item = inventory.getItemActive();
    if(item) item->tick(*this);

    position += velocity;
	if(getMobility() == 0)
		return;
	velocity.z *= yy_val;
	velocity.x *= yy_val;
}

bool Unit::exists()
{
	return true;
}

bool Unit::hasSupportUnderFeet() const
{
	return mobility & Unit::MOBILITY_STANDING_ON_GROUND || mobility & Unit::MOBILITY_STANDING_ON_OBJECT;
}

bool Unit::hasGroundUnderFeet() const
{
	return mobility & Unit::MOBILITY_STANDING_ON_GROUND;
}

int Unit::getModifier(const string& attribute) const
{
	auto it = stats.intVals.find(attribute);
	if(it == stats.intVals.end())
	{
		throw std::logic_error("Asking for an attribute that doesn't exist: " + attribute);
		return 0;
	}

	return 4 + it->second;
}

int Unit::getMaxHP() const
{
    auto it = stats.intVals.find("CONSTITUTION");
	return 100 + 10 * it->second;
}

bool Unit::gainExperience(int exp) {
    intVals["EXPERIENCE"] += exp;
    if(intVals["EXPERIENCE"] > intVals["EXPLIMIT"]) {
        intVals["EXPLIMIT"] *= 9;
        intVals["EXPLIMIT"] /= 7;
        levelUp();
        return true;
    }
    return false;
}

void Unit::levelUp()
{
    ++intVals["CHAR_LEVEL"];
    intVals["STAT_POINTS"] += 3;
}

void Unit::takeDamage(int damage)
{
	intVals["HEALTH"] -= damage;
	intVals["D"] += damage;
}

void Unit::zeroMovement()
{
	velocity.x = 0;
	velocity.y = 0;
	velocity.z = 0;
}

void Unit::setPosition(const Location& a)
{
	position = a;
}

const Location& Unit::getPosition() const
{
	return position;
}

const Location& Unit::getVelocity() const
{
	return velocity;
}

void Unit::setName(const string& newName)
{
	name = newName;
}

bool Unit::human() const
{
	return controllerTypeID == HUMAN_INPUT;
}

float Unit::getAngle()
{
	return ApoMath::getDegrees(angle);
}

void Unit::updateMouseMove(int mousex_, int mousey_)
{
	int changey = mousey_;
	int changex = mousex_;

	int x_major_change = changex / 1000;
	int y_major_change = changey / 1000;

	mouse_x_minor += changex - x_major_change * 1000;
	mouse_y_minor += changey - y_major_change * 1000;

	int xmc2 = mouse_x_minor / 1000;
	int ymc2 = mouse_y_minor / 1000;
	mouse_x_minor -= xmc2 * 1000;
	mouse_y_minor -= ymc2 * 1000;

	x_major_change += xmc2;
	y_major_change += ymc2;

	angle -= x_major_change;

	upangle += ApoMath::DEGREES_180; // Prevent going full circles when moving camera up or down.
	upangle -= y_major_change;

	if(upangle < ApoMath::DEGREES_180 + 25)
		upangle = ApoMath::DEGREES_180 + 25;
	if(upangle > ApoMath::DEGREES_360 - 25)
		upangle = ApoMath::DEGREES_360 - 25;
	upangle -= ApoMath::DEGREES_180;
}

void Unit::updateMousePress(int mouseButtons_)
{
	mouseButtons = mouseButtons_;
}

void Unit::updateKeyState(int keyState_)
{
	keyState = keyState_;
}


int Unit::getKeyAction(int type) const
{
	return (keyState & type);
}

int Unit::getMouseAction(int type) const
{
	return (mouseButtons & type);
}


void Unit::handleCopyOrder(stringstream& ss)
{
	ss >> angle >> upangle >> keyState >>
		position.x >> position.z >> position.y >>
		velocity.x >> velocity.z >> velocity.y >>
		mouseButtons >> controllerTypeID >> birthTime >>
		id >> collision_rule >> staticObject >> model_type >> scale >>
		mouse_x_minor >> mouse_y_minor >> mobility_val;

	HasProperties::handleCopyOrder(ss);
    stats.handleCopyOrder(ss);
    inventory.handleCopyOrder(ss);
	ss >> name;
}

string Unit::copyOrder(int ID) const
{
	stringstream hero_msg;

	hero_msg << "-2 UNIT " << ID;
	hero_msg << " " << angle;
	hero_msg << " " << upangle;
	hero_msg << " " << keyState;
	hero_msg << " " << position.x << " " << position.z << " " << position.y;
	hero_msg << " " << velocity.x << " " << velocity.z << " " << velocity.y;
	hero_msg << " " << mouseButtons;
	hero_msg << " " << controllerTypeID;
	hero_msg << " " << birthTime;
	hero_msg << " " << id;
	hero_msg << " " << collision_rule;
	hero_msg << " " << staticObject;
	hero_msg << " " << model_type;
	hero_msg << " " << scale;
	hero_msg << " " << mouse_x_minor << " " << mouse_y_minor << " " << mobility_val;

	hero_msg << " " << HasProperties::copyOrder();
    hero_msg << " " << stats.copyOrder();
    hero_msg << " " << inventory.copyOrder();
	hero_msg << " " << name << "#";

	return hero_msg.str();
}


void Unit::updateMobility()
{
	if(mobility & (MOBILITY_STANDING_ON_OBJECT | MOBILITY_STANDING_ON_GROUND))
	{
		int dex_modifier = getModifier("MOVEMENT");

		if(mobility & MOBILITY_SQUASHED)
			mobility_val = FixedPoint(1, 6) * FixedPoint(dex_modifier, 10);
		else
			mobility_val = FixedPoint(1) * FixedPoint(dex_modifier, 10);
	}
	else
		mobility_val = FixedPoint(0);
}

const FixedPoint& Unit::getMobility() const
{
	return mobility_val;
}


const Location& Unit::bb_top() const
{
    bb_top_.x = position.x + scale;
    bb_top_.y = position.y + scale * 5;
    bb_top_.z = position.z + scale;
	return bb_top_;
}

const Location& Unit::bb_bot() const
{
    bb_bot_.x = position.x - scale;
    bb_bot_.y = position.y;
    bb_bot_.z = position.z - scale;
	return bb_bot_;
}

void Unit::collides(OctreeObject& o)
{
	// if this object doesnt want to be moved by collisions, don't react.
	if(staticObject || collision_rule == OctreeObject::CollisionRule::NO_COLLISION)
		return;

	// if the other object doesn't want to collide..
	if(!o.collision_rule) {
		return;
    }

	if(position == o.position)
	{
		velocity += bump(id);
	}


	// if my collision rule is STRING_SYSTEM, make changes to myself accordingly.
	if(collision_rule == CollisionRule::STRING_SYSTEM)
	{
		Location direction = (position - o.position);
		if(direction.length() == FixedPoint(0))
		{
			// unresolvable collision. leave it be.
			return;
		}

		direction.normalize();
		direction *= FixedPoint(1, 5);
		velocity += direction;
	}
	else if(collision_rule == CollisionRule::HARD_OBJECT)
	{
		const Location& myTop = bb_top();
		const Location& myBot = bb_bot();

		const Location& hisTop = o.bb_top();
		const Location& hisBot = o.bb_bot();

		// is always positive, otherwise there would be no collision.
		FixedPoint y_diff = min(hisTop.y - myBot.y, myTop.y - hisBot.y);
		FixedPoint x_diff = min(hisTop.x - myBot.x, myTop.x - hisBot.x);
		FixedPoint z_diff = min(hisTop.z - myBot.z, myTop.z - hisBot.z);

		if(y_diff < x_diff && y_diff < z_diff)
		{
			// least offending axis is y
			// velocity.y  = o.velocity.y * FixedPoint(1, 2) - FixedPoint(25, 1000);

			if(hisTop.y < myTop.y)
			{
				velocity.y /= 2;

				// i'm on top
				// if bottom object moves, move the top object with it.
				posCorrection.y += y_diff * FixedPoint(9, 20) * 2;
				posCorrection   += o.velocity + o.posCorrection;

				mobility |= MOBILITY_STANDING_ON_OBJECT;
			}
			else
			{
				// i'm on bot
				posCorrection.y -= y_diff * FixedPoint(9, 20);
				mobility |= MOBILITY_SQUASHED;
			}
		}
		else if(x_diff < z_diff)
		{

			// least offence by x
			// velocity.x  = o.velocity.x * FixedPoint(1, 2);

			if(hisTop.x < myTop.x)
			{
				// i'm on right
				posCorrection.x += x_diff * FixedPoint(9, 20);
			}
			else
			{
				// i'm on left
				posCorrection.x -= x_diff * FixedPoint(9, 20);
			}
		}
		else
		{
			// least offence by z
			// velocity.z  = o.velocity.z * FixedPoint(1, 2);

			if(hisTop.z < myTop.z)
			{
				posCorrection.z += z_diff * FixedPoint(9, 20);
			}
			else
			{
				posCorrection.z -= z_diff * FixedPoint(9, 20);
			}
		}

	}
}

void Unit::init()
{
    intVals["SANITY"] = 100;
	intVals["HEALTH"] = 100;
    intVals["EXPLIMIT"] = 5;

    // these two are taken into account.
    stats.intVals["CONSTITUTION"] = 0;
    stats.intVals["MOVEMENT"] = 0;
    stats.intVals["TELEPATHIC"] = 0;

    // :G
    stats.intVals["BALLISTIC"] = 0;
    stats.intVals["BEAM"] = 0;
    stats.intVals["ENGINEER"] = 0;
    stats.intVals["ZEN"] = 0;
    stats.intVals["SNEAK"] = 0;
    stats.intVals["PERCEPTION"] = 0;
    stats.intVals["DARKVISION"] = 0;

	intVals["REGEN"] = 0;
}

void Unit::switchWeapon(World& world, unsigned x)
{
    inventory.setActiveItem(world, *this, x);
}

