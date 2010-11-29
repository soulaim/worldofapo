#include "world.h"
#include "modelfactory.h"
#include "visualworld.h"

#include <iostream>

using namespace std;

void VisualWorld::init(const Level&)
{
	cerr << "VisualWorld init" << endl;
	assert(models.empty());
	assert(meadows.empty());

	particles.reserve(40000);

	/*
	Vec3 wind(0, 0, 0);
	size_t n = 10;
	size_t k = 500;
	winds.resize(n*k);
	for(size_t i = 0; i < n; ++i)
	{
		size_t X = lvl.pointheight_info.size() * 1.0 * rand() / RAND_MAX;
		size_t Z = lvl.pointheight_info[0].size() * 1.0 * rand() / RAND_MAX;

		for(size_t j = 0; j < k; ++j)
		{
			size_t x = 8*X + 20.0 * rand() / RAND_MAX - 10;
			size_t z = 8*Z + 20.0 * rand() / RAND_MAX - 10;
			float y = lvl.getHeight(x, z).getFloat();
			Vec3 v(x, y, z);
			meadows.push_back(v);
		}
	}
	*/
}

void VisualWorld::terminate()
{
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
	winds.clear();
}


void VisualWorld::viewTick(const std::map<int, Unit>& units, const std::map<int, Projectile>& projectiles, int currentWorldFrame)
{
	for(size_t i = 0; i < particles.size(); ++i)
	{
		particles[i].viewTick();
	}
	
	for(map<int, Unit>::const_iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		updateModel(models[iter->first], iter->second, currentWorldFrame);
	}
	
	for(map<int, Projectile>::const_iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		Model* model = models[iter->first];
		model->setAction("idle");
		model->tick(currentWorldFrame);
	}
}


void VisualWorld::updateModel(Model* model, const Unit& unit, int currentWorldFrame)
{
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
	
	// update state of model
	model->tick(currentWorldFrame);
}

void VisualWorld::tickParticles()
{
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


void VisualWorld::addLight(int id, const Location& location)
{
//	cerr << "Adding light " << id << " at " << location << endl;
	LightObject& light = lights[id];
	light.setDiffuse(1.f, 1.f, 1.f);
	light.setSpecular(0.f, 0.f, 0.f);
	light.setLife(200); // Some frames of LIGHT!
	light.setPower(5); // this doesnt actually do anything yet, but lets set it anyway.
	light.activateLight(); // ACTIVATE :D
	light.position = location;
	light.position.y += FixedPoint(3, 2);
}

void VisualWorld::weaponFireLight(int id, const Location& pos, int life, int r, int g, int b)
{
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

void VisualWorld::genParticleEmitter(const Location& pos, const Location& vel, int life, int max_rand, int scale, int r, int g, int b, int scatteringCone, int particlesPerFrame, int particleLife)
{
	ParticleSource pe;
	pe.intVals["PPF"] = particlesPerFrame;
	pe.intVals["CUR_LIFE"] = life;
	pe.intVals["MAX_LIFE"] = life;
	pe.intVals["PLIFE"]    = particleLife;
	
	pe.intVals["SRED"]     = r;
	pe.intVals["ERED"]     = r / 2;
	
	pe.intVals["SGREEN"]   = g;
	pe.intVals["EGREEN"]   = g / 2;
	
	pe.intVals["SBLUE"]    = b;
	pe.intVals["EBLUE"]    = b / 2;
	
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
	worldMessages.push_back(message);
}

void VisualWorld::removeUnit(int id)
{
	auto it = models.find(id);
	if(it != models.end())
	{
		ModelFactory::destroy(it->second);
		models.erase(it);
	}
}

void VisualWorld::add_event(const WorldEvent& event)
{
	events.push_back(event);
}

