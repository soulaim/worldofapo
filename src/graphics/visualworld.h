#ifndef VISUALWORLD_H
#define VISUALWORLD_H

#include "model.h"
#include "light_object.h"
#include "particle_source.h"
#include "worldevent.h"
#include "level.h"
#include "grasscluster.h"
#include "leveldescriptor.h"

#include "camera.h"
#include "hasproperties.h"
#include "idgenerator.h"

#include "world_item.h"

// TODO: perhaps this could be a base class with a draw() function, then dedicated server and client could have different implementations of this class.
class VisualWorld : public HasProperties
{
	void updateModel(Model*, const Unit&); // view frame update
	int active;
	
public:
	
	enum ModelType
	{
		INVISIBLE_MODEL,
		PLAYER_MODEL,
		BULLET_MODEL,
		ZOMBIE_MODEL,
		ITEM_MODEL,
		STONEBEAST_MODEL,
		WEAPON_MODEL,
		TROLL_MODEL = PLAYER_MODEL
	};
	
	ModelType getModelType(const std::string&);
	Model* getModel(int id);
	
	void genParticleEmitter(const Location& pos, const Location& vel, int life, int max_rand, int scale, const std::string& s_color_s, const std::string& s_color_e, const std::string& e_color_s, const std::string& e_color_e, int scatteringCone = 500, int particlesPerFrame = 5, int particleLife = 50);
	
	void createModel(int id, const Location& location, ModelType type, float scale);
	void setModelScale(int id, float scale);
	
	void addLight(const Location& location, Location direction = Location());
	void weaponFireLight(int id, const Location& pos, int life, int r, int g, int b);
	void tickLights(const std::map<int, Unit>& units);
	void explosion(const Location& pos, const Location& direction = Location());
	
	void init();
	void terminate();
	void tickParticles();
	void viewTick(const std::map<int, Unit>& units, const std::map<int, Projectile>& projectiles, const std::map<int, WorldItem>& items, int currentWorldFrame);

	void removeUnit(int id);

	void add_message(const std::string& message);
	void add_event(const WorldEvent& event);
	
	bool isActive();
	void disable();
	void enable();

	void decorate(const Level& lvl);
	
public:
	std::map<int, Model*> models;
	std::map<int, LightObject> lights;
	std::vector<std::string> worldMessages;
	LevelDescriptor levelDesc;
	
	std::vector<WorldEvent> events; // TODO: why is this here
	
	std::vector<ParticleSource> psources;
	mutable std::vector<Particle> particles; // TODO: maybe get rid of mutable with some kind of predrawing which sorts the particles.
	
	std::vector<GrassCluster> meadows;
	
	Camera camera;
	void setCamera(const Camera& camera);
	void bindCamera(Unit* unit);
	
private:
	IDGenerator lightIDgenerator;
};

#endif

