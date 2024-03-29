#include "world/world.h"
#include "graphics/models/modelfactory.h"
#include "graphics/visualworld.h"
#include "world/random.h"

#include <iostream>

using namespace std;

void VisualWorld::init()
{
	cerr << "VisualWorld init" << endl;

	load("configs/visualworld.conf");

	assert(models.empty());
	assert(meadows.empty());

	particles.reserve(40000);

	enable();
}

void VisualWorld::pushProjectilePath(const Location& start, const Location& end) {
    projectilePaths.push_back(ProjectilePath(start, end));
}

void VisualWorld::insertLevelLight(const Location& position) {
    if(!this->active)
        return;

    int id = lightIDgenerator.nextID();
    int life = ((427 + 84121 * id) % 40) + 10;
    float r = ((427 + 84121 * id) % 256) / 256.0f;
    float g = ((427 + 5325 * id) % 256) / 256.0f;
    float b = ((427 + 9423 * id) % 256) / 256.0f;

	LightObject tmp_light;
	tmp_light.unitBind = -1;
	tmp_light.lifeType = LightSource::IMMORTAL;

    tmp_light.behaviour = LightSource::CONSTANT;
    if(id & 3)
        tmp_light.behaviour = LightSource::RISE_AND_DIE;

	tmp_light.setDiffuse(1.0f + r * 4, 1.0f + g * 4, 1.0f + b * 4);
	tmp_light.setLife(life);
	tmp_light.activateLight();
	tmp_light.position = position;
	lights[id] = tmp_light;
}

void VisualWorld::decorate(const Level&)
{
	if(active == 0)
		return;

    /*
	cerr << "Decorating world with some grass" << flush;
	vec3<float> wind(0, 0, 0);
	size_t k = 200;

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

				vec3<float> v(X + x, y - 0.1f, Z + z);
				meadows.back().bushes.push_back(v);
			}

			meadows.back().center = vec3<float>(X, lvl.getHeight(X, Z).getFloat(), Z);
			meadows.back().radius = radius;
			meadows.back().preload();
		}
	}
    */

	levelDesc.preload();
}


void VisualWorld::setModelScale(int id, float scale)
{
	if(active == 0)
		return;

	models[id]->myScale = scale;
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

	psources.clear();
	particles.clear();

	meadows.clear();

	lightIDgenerator.setNextID(0);
}

// NOTE: only for units so far.
VisualWorld::ModelType VisualWorld::getModelType(const std::string& name)
{
	if(name.substr(0, 5) == "Stone")
		return VisualWorld::ModelType::STONEBEAST_MODEL;
	if(name.substr(0, 6) == "Moogle")
		return VisualWorld::ModelType::ZOMBIE_MODEL;
	if(name.substr(0, 5) == "Troll")
		return VisualWorld::ModelType::TROLL_MODEL;

	cerr << "name: " << name << ", creating player model." << endl;
	return VisualWorld::ModelType::PLAYER_MODEL;
}

void VisualWorld::createModel(int id, const Location& location, ModelType type, float scale)
{
	if(active == 0)
		return;

	// lets try to not leak memory
	if(models.find(id) != models.end())
		delete models[id];

	vec3<float> position;
	position.x = location.x.getFloat();
	position.y = location.y.getFloat();
	position.z = location.z.getFloat();

	models[id] = ModelFactory::create(type);
	models[id]->realUnitPos = position;
	models[id]->currentModelPos = position;
	models[id]->setScale(scale);
}

Model* VisualWorld::getModel(int id)
{
	auto it = models.find(id);
	if(it == models.end())
		return 0;
	return it->second;
}


void VisualWorld::explosion(const Location& pos, const Location& direction)
{
	if(active == 0)
		return;

	if(intVals["EXPLOSION_LIGHTS"])
	{
		addLight(pos, direction);
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


void VisualWorld::viewTick(const std::map<int, Unit>& units, const std::map<int, Projectile>& projectiles, const std::map<int, WorldItem>& items, int currentWorldFrame)
{
	if(active == 0)
		return;

	static int previousWorldFrame = currentWorldFrame;
	int timeDiff = currentWorldFrame - previousWorldFrame;

    for(size_t i=0; i<projectilePaths.size(); ++i) {
        projectilePaths[i].tick(timeDiff);
        if(!projectilePaths[i].alive()) {
            projectilePaths[i] = projectilePaths.back();
            projectilePaths.pop_back();
            --i;
        }
    }

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

	for(map<int, WorldItem>::const_iterator iter = items.begin(); iter != items.end(); ++iter)
	{
		Model* model = models[iter->first];
		model->increase_rot_y(3.0f * timeDiff);
	}

	// update all models, which ever type they might be.
	for(auto iter = models.begin(); iter != models.end(); ++iter)
	{
		iter->second->tick(currentWorldFrame);
	}

	previousWorldFrame = currentWorldFrame;
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
				light.position = units.find(light.unitBind)->second.getEyePosition();
                light.position.y += 4;
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


void VisualWorld::addLight(const Location& location, Location direction)
{

	if(active == 0)
		return;

	int id = lightIDgenerator.nextID();

	LightObject& light = lights[id];
	light.setDiffuse(5.3f, 5.3f, 5.3f);
	light.setSpecular(0.f, 0.f, 0.f);
	light.setLife(470);
	light.setPower(5);
	light.activateLight();
	light.lifeType = LightSource::MORTAL;
	light.position = location;
	light.position.y += FixedPoint(3, 2);
	light.velocity = direction;

}

int VisualWorld::getTorchID(const Unit& unit) {
    for(map<int, LightObject>::iterator it = lights.begin(); it != lights.end(); ++it) {
        if(it->second.unitBind == unit.id)
            return it->first;
    }
    return -1;
}

void VisualWorld::activateTorch(const Unit& unit, int power) {
	if(active == 0)
		return;

    float multiplier = power / 10.f;
    int id = this->getTorchID(unit);
    if(id == -1)
        id = lightIDgenerator.nextID();

	LightObject& light = lights[id];
	light.setDiffuse(multiplier * 1.3f, multiplier * 0.7f, multiplier * 0.7f);
	light.setSpecular(0.f, 0.f, 0.f);
	light.setLife(25 * 120); // two minutes of light.
	light.setPower(5);
	light.activateLight();
	light.lifeType = LightSource::MORTAL;
    light.behaviour = LightSource::ONLY_DIE;
	light.position = unit.getEyePosition();
    light.unitBind = unit.id;
}

void VisualWorld::weaponFireLight(const Location& pos, int life, int r, int g, int b)
{
	if(active == 0)
		return;

    int id = this->lightIDgenerator.nextID();
	LightObject& light = lights[id];
	light.setDiffuse(r / 255.f, g / 255.f, b / 255.f);
	light.setSpecular(0.f, 0.f, 0.f);
	light.setLife(life);
	light.activateLight();
	light.position = pos;

	//light.behaviour = LightSource::RISE_AND_DIE;
	light.behaviour = LightSource::ONLY_DIE;
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

void VisualWorld::enable()
{
	active = 1;
}

void VisualWorld::disable()
{
	active = 0;
}
