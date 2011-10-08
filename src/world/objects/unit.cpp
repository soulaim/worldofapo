#include "world/objects/unit.h"
#include "misc/apomath.h"
#include "world/world.h"

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
	hitpoints(500),
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

	// create attributes
	intVals["STR"] = -666;
	intVals["DEX"] = -666;
	intVals["VIT"] = -666;

	intVals["WIS"] = -666;
	intVals["INT"] = -666;
	intVals["REGEN"] = 0;

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

// TODO: create a separate class for doing this shit.
void Unit::setDefaultMonsterAttributes()
{
	// set attributes
	intVals["STR"] = 4;
	intVals["DEX"] = 4;
	intVals["VIT"] = 4;

	intVals["WIS"] = 4;
	intVals["INT"] = 4;

	intVals["REGEN"] = 5;
}

// TODO: create a separate class for doing this shit.
void Unit::setDefaultPlayerAttributes()
{
	// set attributes
	intVals["STR"] = 4;
	intVals["DEX"] = 4;
	intVals["VIT"] = 4;

	intVals["WIS"] = 4;
	intVals["INT"] = 4;

	intVals["REGEN"] = 10;
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


Location Unit::getEyePosition()
{
	return Location(position.x, position.y + (bb_top().y - position.y) * FixedPoint(3, 4), position.z);
}


void Unit::regenerate()
{
	int unit_max_hp = getMaxHP();
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
			hitpoints = -1;
		}
		else
		{
			velocity.x *= FixedPoint(10, 100);
			velocity.z *= FixedPoint(10, 100);
			takeDamage(damage_int * damage_int / 500);
		}

		// allow deny only after surviving the first hit with ground.
		// not a deny if thrown down a cliff!
		if(hitpoints > 0)
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
	return ((*this)["RESPAWN"] == 0);
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
	auto it = intVals.find(attribute);
	if(it == intVals.end())
	{
		throw std::logic_error("Asking for an attribute that doesn't exist: " + attribute);
		return 0;
	}

	if(it->second == -666)
	{
		cerr << name << ": " << (*this)("AREA") << ", " << (*this)["TEAM"] << ", has attribute " << attribute << " at value -666" << endl;
	}

	assert(it->second != -666);


	return 6 + it->second;
}

int Unit::getMaxHP() const
{
	return 100 * getModifier("STR");
}

void Unit::levelUp()
{
	intVals["STR"]++;
	intVals["DEX"]++;
	hitpoints = getMaxHP();
}

void Unit::takeDamage(int damage)
{
	hitpoints -= damage;
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
		mouseButtons >> controllerTypeID >> hitpoints >> birthTime >>
		id >> collision_rule >> staticObject >> model_type >> scale >>
		mouse_x_minor >> mouse_y_minor >> mobility_val;

	HasProperties::handleCopyOrder(ss);
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
	hero_msg << " " << hitpoints;
	hero_msg << " " << birthTime;
	hero_msg << " " << id;
	hero_msg << " " << collision_rule;
	hero_msg << " " << staticObject;
	hero_msg << " " << model_type;
	hero_msg << " " << scale;
	hero_msg << " " << mouse_x_minor << " " << mouse_y_minor << " " << mobility_val;

	hero_msg << " " << HasProperties::copyOrder();
    hero_msg << " " << inventory.copyOrder();
	hero_msg << " " << name << "#";

	return hero_msg.str();
}


void Unit::updateMobility()
{
	if(mobility & (MOBILITY_STANDING_ON_OBJECT | MOBILITY_STANDING_ON_GROUND))
	{
		int dex_modifier = getModifier("DEX");

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
	if(!exists())
		return;

	if(o.type == UNIT)
	{
		Unit* u = static_cast<Unit*>(&o);
		if(!u->exists()) // to make sure no collisions occur with dead heroes (spawning time)
			return;
	}

	// if one of the objects doesn't want to collide, then don't react.
	if(!(collision_rule & o.collision_rule))
		return;

	// if this object doesnt want to be moved by collisions, don't react.
	if(staticObject)
		return;


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
}

void Unit::switchWeapon(World& world, unsigned x)
{
    inventory.setActiveItem(world, *this, x);
}

