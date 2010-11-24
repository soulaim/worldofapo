#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <vector>
#include <string>
#include <memory>

#include "unit.h"
#include "projectile.h"
#include "model.h"
#include "level.h"
#include "apomath.h"
#include "octree.h"
#include "movable_object.h"

#include "light_object.h"
#include "particle_source.h"

struct WorldEvent
{
	// Identifications for events where we want to do some SFX.
	enum EventType
	{
		DAMAGE_BULLET,
		DAMAGE_DEVOUR,
		DEATH_PLAYER,
		DEATH_ENEMY,
		CENTER_CAMERA
	};

	EventType type;
	
	Location t_position;
	Location t_velocity;
	
	Location a_position;
	Location a_velocity;
	
	int actor_id;
	int target_id;
};

class VisualWorld
{
	void updateModel(Model*, const Unit&, int currentWorldFrame); // view frame update

public:
	std::map<int, Model*> models;
	std::map<int, LightObject> lights;
	std::vector<std::string> worldMessages;
	
	std::vector<ParticleSource> psources;
	mutable std::vector<Particle> particles; // TODO: maybe get rid of mutable with some kind of predrawing which sorts the particles.

	void genParticleEmitter(const Location& pos, const Location& vel, int life, int max_rand, int scale, int r, int g, int b, int scatteringCone = 500, int particlesPerFrame = 5, int particleLife = 50);
	
	void addLight(int id, Location& location);
	void weaponFireLight(int id, const Location& pos, int life, int r, int g, int b);
	void tickLights(const std::map<int, Unit>& units);
	
	void init();
	void terminate();
	void tickParticles();
	void viewTick(const std::map<int, Unit>& units, const std::map<int, Projectile>& projectiles, int currentWorldFrame);

	void removeUnit(int id);
	void add_message(const std::string& message);

	std::vector<WorldEvent> events;
	void add_event(const WorldEvent& event);
};

class World
{
	void tickUnit(Unit&, Model*);       // world frame update
	void tickProjectile(Projectile&, Model*); // world frame update
	
	static FixedPoint heightDifference2Velocity(const FixedPoint& h_diff);
	void generateInput_RabidAlien(Unit& unit);
	void doDeathFor(Unit& unit);

public:
	int currentWorldFrame;
	VisualWorld visualworld;

	enum ModelType
	{
		INVISIBLE_MODEL,
		PLAYER_MODEL,
		BULLET_MODEL
	};
	
	World();
	void init();

	std::shared_ptr<Octree> octree;
	
	std::map<int, Unit> units;
	std::map<int, Projectile> projectiles;
	
	Level lvl;
	
	
	int getZombies();
	std::vector<Location> humanPositions() const;
	
	void instantForceOutwards(const FixedPoint& power, const Location& pos); // this is shit
	void atDeath(MovableObject&, HasProperties&);
	
	void worldTick(int tickCount);
	
	void addUnit(int id, bool player = true);
	void addProjectile(Location& location, int id, size_t model_prototype);
	void removeUnit(int id);
	
	void setNextUnitID(int id);
	int currentUnitID() const;
	int nextUnitID();
	int nextPlayerID();
	
	int show_errors;
	
	void terminate();
	
	void add_message(const std::string& message);
	void add_event(const WorldEvent& event);
	
	unsigned long checksum() const;
	
private:
	std::vector<int> deadUnits;
	ApoMath apomath;
	
	int _unitID_next_unit;
	int _playerID_next_player;
	
};

#endif

