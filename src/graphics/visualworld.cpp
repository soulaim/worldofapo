#include "world.h"
#include "modelfactory.h"
#include "visualworld.h"
#include "random.h"

#include <iostream>

using namespace std;

void VisualWorld::init()
{
	cerr << "VisualWorld init" << endl;
	
	load("visualworld.conf");
	
	assert(models.empty());
	assert(meadows.empty());

	particles.reserve(40000);
	
	enable();
}

void VisualWorld::decorate(const Level& lvl)
{
	if(active == 0)
		return;

	cerr << "Decorating world with some lights" << endl;

	RandomMachine random;
	random.setSeed(14);

	// Find the highest point in lvl and add a strong light there.
	LightObject tmp_light;
	tmp_light.unitBind = -1;
	tmp_light.lifeType = LightSource::IMMORTAL;
	tmp_light.behaviour = LightSource::CONSTANT;
	tmp_light.setDiffuse(8.0, 2.0, 2.0);
	tmp_light.setLife(150);
	tmp_light.activateLight();
	tmp_light.position = Location(FixedPoint(500), FixedPoint(0), FixedPoint(500));
	tmp_light.position.y = lvl.getHeight(tmp_light.position.x, tmp_light.position.z) + FixedPoint(5);
	
	lights[lightIDgenerator.nextID()] = tmp_light;

	// Add some random lights.
	for(int i = 0; i < intVals["RANDOM_LIGHTS"]; ++i)
	{
		LightObject tmp_light;
		tmp_light.unitBind = -1;
		tmp_light.lifeType = LightSource::IMMORTAL;
		tmp_light.behaviour = LightSource::RISE_AND_DIE;
		tmp_light.setDiffuse(random.getFloat(1.0f, 8.0f), random.getFloat(1.0f, 8.0f), random.getFloat(1.0, 8.0f));
		tmp_light.setLife(150);
		tmp_light.activateLight();
		
		tmp_light.position = lvl.getRandomLocation(56710 + i);
		
		lights[lightIDgenerator.nextID()] = tmp_light;
	}

	cerr << "Decorating world with some grass" << flush;
	Vec3 wind(0, 0, 0);
//	size_t n = 10;
	size_t k = 200;
//	winds.resize(n*k);
//	meadows.resize(n);
	
	int meadow_box_size = 40;
	
	int x_limit = lvl.max_x().getInteger();
	int z_limit = lvl.max_z().getInteger();
	
	for(int x_box = 0; x_box < x_limit; x_box += meadow_box_size)
	{
		cerr << "." << flush;
		for(int z_box = 0; z_box < z_limit; z_box += meadow_box_size)
		{
			meadows.push_back(GrassCluster());
			
			size_t X = x_box + meadow_box_size / 2;
			size_t Z = z_box + meadow_box_size / 2;
			
			float radius = 25.0;
			for(size_t j = 0; j < k; ++j)
			{
				float x = random.getFloat(-meadow_box_size/2.f, meadow_box_size/2.f);
				float z = random.getFloat(-meadow_box_size/2.f, meadow_box_size/2.f);
				
				if(X + x > x_limit)
					continue;
				if(Z + z > x_limit)
					continue;
				
				float y = lvl.getHeight(X + x, Z + z).getFloat();
				
				float y_competitors[4];
				y_competitors[0] = lvl.getHeight(X + x + 1.f, Z + z).getFloat();
				y_competitors[1] = lvl.getHeight(X + x - 1.f, Z + z).getFloat();
				y_competitors[2] = lvl.getHeight(X + x, Z + z + 1.f).getFloat();
				y_competitors[3] = lvl.getHeight(X + x, Z + z - 1.f).getFloat();
				
				bool nono = false;
				for(int qq = 0; qq < 4; ++qq)
				{
					if( (y_competitors[qq] - y) * (y_competitors[qq] - y) > 0.2f)
					{
						nono = true;
					}
				}
				if(nono)
				{
					continue;
				}
				
				Vec3 v(X + x, y - 0.1f, Z + z);
				meadows.back().bushes.push_back(v);
			}
			
			meadows.back().center = Vec3(X, lvl.getHeight(X, Z).getFloat(), Z);
			meadows.back().radius = radius;
			meadows.back().preload();
		}
	}
	
	levelDesc.preload();
}

void VisualWorld::terminate()
{
	for(auto it = meadows.begin(); it != meadows.end(); ++it)
	{
		it->unload();
	}

	levelDesc.unload();

	for(auto it = models.begin(); it != models.end(); ++it)
	{
		ModelFactory::destroy(it->second);
	}
	models.clear();

	lights.clear();
	worldMessages.clear();
	
	psources.clear();
	particles.clear();

	meadows.clear();

	lightIDgenerator.setNextID(0);
}


void VisualWorld::explosion(int id, const Location& pos, const Location& direction)
{
	if(active == 0)
		return;
	
	if(intVals["EXPLOSION_LIGHTS"])
	{
		addLight(id, pos, direction);
	}
	
	// TODO: move the visual part of the explosion to the visualworld by storing a WorldEvent (or a maybe something derived from it)
	int complexity = intVals["PARTICLE_EFFECT_COMPLEXITY"];
	
	stringstream ss_explosion_life; ss_explosion_life << "BOOM_" << complexity << "_LIFE";
	int explosion_life = intVals[ss_explosion_life.str()];
	
	stringstream ss_explosion_ppf; ss_explosion_ppf << "BOOM_" << complexity << "_PPF";
	int ppf = intVals[ss_explosion_ppf.str()];
	
	stringstream ss_explosion_plife; ss_explosion_plife << "BOOM_" << complexity << "_PLIFE";
	int plife = intVals[ss_explosion_plife.str()];
	
	genParticleEmitter(pos, direction, explosion_life, 3500, 7500, "WHITE", "ORANGE", "ORANGE", "DARK_RED", 1200, ppf, plife);
	
	genParticleEmitter(pos, direction, 5, 3500, 7500, "GREY", "GREY", "GREY", "GREY", 1200, 50, 150);
	
}


void VisualWorld::viewTick(const std::map<int, Unit>& units, const std::map<int, Projectile>& projectiles, int currentWorldFrame)
{
	if(active == 0)
		return;
	
	camera.tick();
	
	for(size_t i = 0; i < particles.size(); ++i)
	{
		particles[i].viewTick();
	}
	
	for(map<int, Unit>::const_iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		updateModel(models[iter->first], iter->second);
	}
	
	for(map<int, Projectile>::const_iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		Model* model = models[iter->first];
		model->setAction("idle");
	}
	
	// update all models, which ever type they might be.
	for(auto iter = models.begin(); iter != models.end(); ++iter)
	{
		iter->second->tick(currentWorldFrame);
	}
}

bool VisualWorld::isActive()
{
	return active;
}

void VisualWorld::setCamera(const Camera& cam)
{
	camera = cam;
}

void VisualWorld::bindCamera(Unit* unit)
{
	camera.bind(unit, Camera::FIRST_PERSON);
}

void VisualWorld::updateModel(Model* model, const Unit& unit)
{
	if(active == 0)
		return;
	
	assert(model != 0);
	
	if(unit.getKeyAction(Unit::MOVE_FRONT))
	{
		if(unit.getKeyAction(Unit::MOVE_LEFT))
			model->setAction("run_forwardleft");
		else if(unit.getKeyAction(Unit::MOVE_RIGHT))
			model->setAction("run_forwardright");
		else
			model->setAction("run_forward");
	}
	else if(unit.getKeyAction(Unit::MOVE_BACK))
	{
		model->setAction("run_backward");
	}
	else if(unit.getKeyAction(Unit::MOVE_LEFT))
	{
		model->setAction("run_left");
	}
	else if(unit.getKeyAction(Unit::MOVE_RIGHT))
	{
		model->setAction("run_right");
	}
	else
	{
		model->setAction("idle2");
	}
}

void VisualWorld::tickParticles()
{
	
	if(active == 0)
		return;
	
	
	for(size_t i=0; i<psources.size(); ++i)
	{
		psources[i].tick(particles);
		if(!psources[i].alive())
		{
			psources[i] = psources.back();
			psources.pop_back();
			--i;
		}
	}
	
	for(size_t i=0; i<particles.size(); ++i)
	{
		if(!particles[i].alive())
		{
			particles[i] = particles.back();
			particles.pop_back();
			--i;
		}
		else
			particles[i].tick();
	}
}

void VisualWorld::tickLights(const std::map<int, Unit>& units)
{
	
	if(active == 0)
		return;
	
	
	vector<int> deadLights;
	for(auto iter = lights.begin(); iter != lights.end(); iter++)
	{
		LightObject& light = iter->second;
		light.updateIntensity();
		
		// TODO should have a world.tickMovableObject(&light); call here
		
		// update light qualities
		if(!light.tickLight())
		{
			// if light has died out, should maybe like prepare to erase it or something.
			light.deactivateLight();
			deadLights.push_back(iter->first);
			
			// cerr << "DEAD LIGHT! ERASING" << endl;
		}
		
		if(light.unitBind != -1)
		{
			if(units.find(light.unitBind) == units.end())
			{
				// master of the light is dead. must kill the light.
				light.deactivateLight();
				deadLights.push_back(iter->first);
			}
			else
			{
				// light.drawPos  = models.find(light.unitBind)->second.currentModelPos;
				light.position = units.find(light.unitBind)->second.getPosition();
				light.position.y += FixedPoint(7, 2);
			}
		}
	}
	
	for(size_t i = 0; i < deadLights.size(); ++i)
	{
//		cerr << "Removing light " << deadLights[i] << endl;
		lights.erase(deadLights[i]);
	}
	deadLights.clear();
}


void VisualWorld::addLight(int id, const Location& location, Location direction)
{
	
	if(active == 0)
		return;
	
	
	// cerr << "Adding light " << id << " at " << location << endl;
	LightObject& light = lights[id];
	light.setDiffuse(5.3f, 5.3f, 5.3f);
	light.setSpecular(0.f, 0.f, 0.f);
	light.setLife(270); // Some frames of LIGHT!
	light.setPower(5); // this doesnt actually do anything yet, but lets set it anyway.
	light.activateLight(); // ACTIVATE :D
	light.position = location;
	light.position.y += FixedPoint(3, 2);
	light.velocity = direction;
}

void VisualWorld::weaponFireLight(int id, const Location& pos, int life, int r, int g, int b)
{
	
	if(active == 0)
		return;
	
	
	LightObject& light = lights[id];
	light.setDiffuse(r / 255.f, g / 255.f, b / 255.f);
	light.setSpecular(0.f, 0.f, 0.f);
	light.setLife(life);
	light.activateLight();
	light.position = pos;
	light.position.y += FixedPoint(2);
	
	light.behaviour = LightSource::RISE_AND_DIE;
	// light.behaviour = LightSource::ONLY_DIE;

}

void VisualWorld::genParticleEmitter(const Location& pos, const Location& vel, int life, int max_rand, int scale, const string& s_color_s, const string& s_color_e, const string& e_color_s, const string& e_color_e, int scatteringCone, int particlesPerFrame, int particleLife)
{
	
	if(active == 0)
		return;
	
	
	ParticleSource pe;
	pe.intVals["PPF"] = particlesPerFrame;
	pe.intVals["CUR_LIFE"] = life;
	pe.intVals["MAX_LIFE"] = life;
	pe.intVals["PLIFE"]    = particleLife;
	
	pe.setColors(s_color_s, s_color_e, e_color_s, e_color_e);
	
	pe.getIntProperty("MAX_RAND") = max_rand;
	pe.getIntProperty("SCALE") = scale;
	
	pe.getIntProperty("PSP_1000") = scatteringCone;
	
	/*
	pe.getIntProperty("RAND_X_1000") = 500;
	pe.getIntProperty("RAND_Y_1000") = 0;
	*/
	
	pe.position = pos;
	pe.velocity = vel;
	
	psources.push_back(pe);
}


void VisualWorld::add_message(const std::string& message)
{
	
	if(active == 0)
		return;
	
	
	worldMessages.push_back(message);
}

void VisualWorld::removeUnit(int id)
{
	camera.unitDie(id);
	
	auto it = models.find(id);
	if(it != models.end())
	{
		ModelFactory::destroy(it->second);
		models.erase(it);
	}
}

void VisualWorld::add_event(const WorldEvent& event)
{
	/*
	if(active == 0)
		return;
	*/
	
	events.push_back(event);
}

void VisualWorld::enable()
{
	active = 1;
}

void VisualWorld::disable()
{
	active = 0;
}
