
#ifndef H_WEAPON
#define H_WEAPON

#include "world/item.h"
#include "misc/hasproperties.h"
#include "misc/fixed_point.h"
#include "misc/vec3.h"
#include "world/projectile.h"

#include <iostream>
#include <string>
#include <map>
#include <fstream>

class World;
class Unit;

class Weapon : public Item
{
	Projectile proto_projectile;
public:
	
	Weapon(const std::string& fileName)
	{
		load(fileName);
		
		if(intVals["AMMO_VALUE"] == 0)
		{
			std::cerr << "ERROR: " << strVals["NAME"] << ": Ammo value not defined or defined as zero! ILLEGAL!" << std::endl;
			intVals["AMMO_VALUE"] = 1;
		}
		
		if(intVals["CHILD_TPF"] == 0)
			intVals["CHILD_TPF"] = 1;
		
		generatePrototypeProjectile();
	}
	
	void generatePrototypeProjectile();
	void reloadComplete(Unit& user);
	void prepareReload(Unit& user);
	void fire(World& world, Unit& user);
	
	/*
	virtual void onActivate(World&, Unit&) = 0;
	virtual void onSecondaryActivate(World&, Unit&) = 0;
	
	virtual void onCollect(World&, Unit&) = 0;
	virtual void onDrop(World&, Unit&) = 0;
	virtual void onConsume(World&, Unit&) = 0;
	*/
	
	void onActivate(World& world, Unit& user);
	void onSecondaryActivate(World&, Unit&);
	
	bool onCollect(World&, Unit&);
	void onDrop(World&, Unit&);
	void onConsume(World&, Unit&);
	
	void tick(Unit& user); // its a little bit silly that a reference to the user is required..
	
	FixedPoint isReloading();
	FixedPoint onCooldown();
	
	// from movable object
	/*
	virtual FixedPoint getGravity();
	virtual FixedPoint getFriction();
	*/
	
	// from octree object
	/*
	virtual Location bb_top() const = 0;
	virtual Location bb_bot() const = 0;
	virtual void collides(OctreeObject&) = 0;
	*/
	
	/*
	FixedPoint getGravity();
	FixedPoint getFriction();
	
	Location bb_top() const;
	Location bb_bot() const;
	void collides(OctreeObject&);
	*/
};

#endif
