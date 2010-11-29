#ifndef VISUALWORLD_H
#define VISUALWORLD_H

#include "model.h"
#include "light_object.h"
#include "particle_source.h"
#include "worldevent.h"
#include "level.h"

// TODO: perhaps this could be a base class with a draw() function, then dedicated server and client could have different implementations of this class.
class VisualWorld
{
	void updateModel(Model*, const Unit&, int currentWorldFrame); // view frame update

public:
	void genParticleEmitter(const Location& pos, const Location& vel, int life, int max_rand, int scale, int r, int g, int b, int scatteringCone = 500, int particlesPerFrame = 5, int particleLife = 50);
	
	void addLight(int id, const Location& location);
	void weaponFireLight(int id, const Location& pos, int life, int r, int g, int b);
	void tickLights(const std::map<int, Unit>& units);
	
	void init(const Level& lvl);
	void terminate();
	void tickParticles();
	void viewTick(const std::map<int, Unit>& units, const std::map<int, Projectile>& projectiles, int currentWorldFrame);

	void removeUnit(int id);

	void add_message(const std::string& message);
	void add_event(const WorldEvent& event);

public:
	std::map<int, Model*> models;
	std::map<int, LightObject> lights;

	std::vector<std::string> worldMessages;
	std::vector<WorldEvent> events;

	std::vector<ParticleSource> psources;
	mutable std::vector<Particle> particles; // TODO: maybe get rid of mutable with some kind of predrawing which sorts the particles.

	std::vector<Vec3> meadows;
	std::vector<Vec3> winds;
};

#endif

