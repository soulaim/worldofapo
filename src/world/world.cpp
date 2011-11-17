
#include "world.h"
#include "graphics/models/modelfactory.h"
#include "graphics/visualworld.h"
#include "world/objects/world_item.h"
#include "objects/itempicker.h"
#include "logic/monster_creator.h"

#include <iostream>
#include <stdexcept>

using namespace std;

#define LAZY_UPDATE 0

void World::checksum(vector<World::CheckSumType>& checksums) const
{
	CheckSumType hash = 5381;

	for (auto it = units.begin(); it != units.end(); ++it)
	{
		int id = it->first;
		Location pos = it->second.getPosition();
		hash = ((hash << 5) + hash) + id;
		hash = ((hash << 5) + hash) + pos.x.getInteger();
		hash = ((hash << 5) + hash) + pos.y.getInteger();
		hash = ((hash << 5) + hash) + pos.z.getInteger();
	}
	checksums.push_back(hash);

	hash = 5381;
	for (auto it = units.begin(); it != units.end(); ++it)
	{
		hash = ((hash << 5) + hash) + it->second["HEALTH"];
	}
	checksums.push_back(hash);

	hash = 5381;
	for(auto it = items.begin(); it != items.end(); it++)
	{
		int id = it->first;
		Location pos = it->second.getPosition();
		hash = ((hash << 5) + hash) + id;
		hash = ((hash << 5) + hash) + pos.x.getInteger();
		hash = ((hash << 5) + hash) + pos.y.getInteger();
		hash = ((hash << 5) + hash) + pos.z.getInteger();
	}
	checksums.push_back(hash);

	hash = 5381;
	for(auto it = projectiles.begin(); it != projectiles.end(); it++)
	{
		int id = it->first;
		Location pos = it->second.getPosition();
		hash = ((hash << 5) + hash) + id;
		hash = ((hash << 5) + hash) + pos.x.getInteger();
		hash = ((hash << 5) + hash) + pos.y.getInteger();
		hash = ((hash << 5) + hash) + pos.z.getInteger();
	}
	checksums.push_back(hash);
}

void World::handle(const SoundEvent& event) {
    int id = getLocalPlayerID();
    if(id >= 0) {
        std::map<int, Unit>::iterator it = units.find(id);
        if(it == units.end())
            return;

        PlaySoundEvent e;
        e.sound = event.sound;
        e.distance = (it->second.getEyePosition() - event.source).length().getFloat();
        e.magnitude = event.magnitude;
        sendMsg(e);
    }
}

int World::getLocalPlayerID() {
    return this->localPlayerID;
}

void World::awardExperience(Unit& deadUnit) {
    for(std::map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter) {
        if(iter->first >= 10000) {
            return;
        }

        iter->second.gainExperience(*this, deadUnit.intVals["REWARD"] + 1);
    }
}

void World::instantForceOutwards(const FixedPoint& power, const Location& pos, const string& name, int owner)
{
	// find out who is inflicted an how.
	for(std::map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
	{
		const Location& pos2 = iter->second.getPosition();
		Location velocity_vector = (pos2 - pos);

		FixedPoint distance = velocity_vector.length();
		if(distance < FixedPoint(1))
			distance = FixedPoint(1);

		velocity_vector.normalize();

		velocity_vector *= power;
		velocity_vector /= distance;

		FixedPoint force = velocity_vector.length();
		FixedPoint damage = force * force * FixedPoint(100);

		int int_damage = damage.getInteger();
		if(int_damage > 0)
		{
			Unit& u = iter->second;
			u.takeDamage(int_damage, DamageType::PURE);
			u.last_damage_dealt_by = owner;
			u("DAMAGED_BY") = name;
		}

		iter->second.velocity += velocity_vector / FixedPoint(iter->second["MASS"], 1000);
	}

	// create some effect or something
	Location zero; zero.y = FixedPoint(1, 2);
	visualworld->explosion(pos, zero);
}

void World::atDeath(MovableObject& object, HasProperties& properties)
{
	if(properties.strVals.find("AT_DEATH") == properties.strVals.end())
		return;

	if(properties.strVals["AT_DEATH"] == "")
		properties.strVals["AT_DEATH"] = "NOTHING";

	// this way can only store one event to be executed at death :( should maybe reconcider that.
	if(properties("AT_DEATH") == "EXPLODE")
	{
		FixedPoint explode_power(properties["EXPLODE_POWER"]);
		const Location& pos = object.position;

		instantForceOutwards(explode_power, pos, properties("NAME"), properties["OWNER"]);
	}
	else if(properties("AT_DEATH") == "PUFF")
	{
		const Location& pos = object.position - object.velocity * 2;
		Location velocity(0, FixedPoint(1, 5), 0);
		visualworld->genParticleEmitter(pos, velocity, 4, 300, 1000, "GREY", "BLACK", "GREY", "BLACK", 200, 1, 50);
	}
}


void World::createLevelObjects() //fazias
{
    cerr << "reading level objects" << endl;
    vector<LevelObject> objects = lvl.level_objects.getObjects();

    for(vector<LevelObject>::iterator it = objects.begin();it != objects.end();++it)
    {
        cerr << "+";
        Location pos = Location(it->coord_x, 0, it->coord_z);

        if(it->object_name == "box") {
            int id = unitIDgenerator.nextID();
            addBoxUnit(id, pos);
        }
        else if(it->object_name == "smallitem") {
            /*
            RandomMachine random; random.setSeed(itemCreationNums);
            WorldItem item = itemCreator.createSmallItem(1 + depth / 3, random); item.position = pos;
            this->addItem(item, VisualWorld::ModelType::ITEM_MODEL, unitIDgenerator.nextID());
            */
        }
        else if(it->object_name == "item") {
            /*
            WorldItem item = itemCreator.makeItem(depth, ++itemCreationNums, this->currentWorldFrame);
            item.position = pos;
            this->addItem(item, VisualWorld::ModelType::ITEM_MODEL, unitIDgenerator.nextID());
            */
        }
        else if(it->object_name == "monster") {
            int id = this->unitIDgenerator.nextID();
            this->addAIUnit(id, pos);
        }
        else {
            string errorstr = "Unrecognized object name: ";
            errorstr.append(it->object_name);
            throw std::logic_error(errorstr);
        }
    }

    cerr << "\n" << "finished reading level objects" << endl;
}

void World::resetGame()
{
	cerr << "Reseting world game to a feasible start" << endl;
    this->intVals["START"] = this->currentWorldFrame;
    this->add_message("^GGame reset! You can now try again.");

    for(map<int, WorldItem>::iterator it = items.begin(); it != items.end(); ++it)
        it->second.dead = 1;

    for(map<int, Unit>::iterator it = units.begin(); it != units.end(); ++it)
	{
		if(!it->second.human())
		{
			it->second["HEALTH"] = -1;
			it->second("DAMAGED_BY") = "Game\\sreset";
			it->second["DELETED"] = 1;
		}
		else
		{
			// reset player characters
            int id = it->first;
            it->second = Unit();
            it->second.getInventoryEditor().clear();
            it->second.init();
			it->second["HEALTH"] = it->second.getMaxHP();
            it->second.id = id;
		}
	}

	createLevelObjects();

	teams[0].reset();
	teams[1].reset();

	const Location& startPos = lvl.getStartLocation();

	for(auto it = units.begin(); it != units.end(); ++it)
	{
		if(it->second.human())
		{
            it->second.position = startPos;
            it->second.velocity = Location();
            it->second.setHumanStart(*this);
		}
	}
}

void World::unitHasDied(int id) {
    this->deadUnits.push_back(id);
}

std::string World::getTeamColour(Unit& u)
{
	if(localPlayerID == -1)
		return "";

	auto it = units.find(localPlayerID);
	if(it == units.end())
		return "";

	if(it->second["TEAM"] == u["TEAM"])
		return "^G";
	if((it->second["TEAM"] ^ u["TEAM"]) == 1)
		return "^R";
	return "^W";
}

int World::getLocalTeam()
{
	if(localPlayerID == -1)
		return -1;

	auto it = units.find(localPlayerID);
	if(it == units.end())
		return -1;

	return it->second["TEAM"];
}


World::World(VisualWorld* vw)
{
	localPlayerID = -1; // default value.

	assert(vw);
	visualworld = vw;
	init();
}

void World::buildTerrain(int n, float& percentage_done)
{
	// TODO, post-passes should not be constant in code
	lvl.generate(*this, n, 1, percentage_done);
	intVals["GENERATOR"] = n;
}

string World::generatorMessage()
{
	stringstream ss;
	ss << "-2 WORLD_GEN_PARAM " << intVals["GENERATOR"] << " " << strVals["AREA_NAME"] << "#";
	return ss.str();
}

void World::init()
{
	cerr << "World::init()" << endl;
	unitIDgenerator.setNextID(10000);
	visualworld->init();
	show_errors = 0;
}

void World::terminate()
{
	cerr << "World::terminate()" << endl;

	cerr << "  clearing units" << endl;
	units.clear();

	cerr << "  clearing projectiles" << endl;
	projectiles.clear();

	cerr << "  clearing items" << endl;
	items.clear();

	cerr << "  clearing deadUnits" << endl;
    deadUnits.clear(); // redundant?
	visualworld->terminate();
}


void World::worldTick(int tickCount)
{

	currentWorldFrame = tickCount;

    // if no players are alive, reset game and time
    int players = 0;
    int deadPlayers = 0;
    for(map<int, Unit>::iterator it = units.begin(); it->first < 10000 && it != units.end(); ++it) {
        ++players;
        if(it->second.intVals["HEALTH"] <= 0)
            ++deadPlayers;
    }

    if(players > 0 && (players == deadPlayers)) {
        this->resetGame();
        return;
    }

	// when to spawn monsters? Note that this is not actually safe.
    int depthCounter = currentWorldFrame - this->intVals["START"];
    if(depthCounter % (400 - depthCounter / 100) == 12) {
        // spawn monster.
        for(int i=0; i<1000; ++i) {
            Location spawnPoint = this->lvl.getRandomLocation(depthCounter + ((i * 1731) % 10000) );
            if(lvl.getHeight(spawnPoint.x, spawnPoint.z) < 6) {

                int depth = depthCounter / 400;
                stringstream spawnMessage; spawnMessage << "^RSpawning monster with depth: " << depth;
                this->add_message(spawnMessage.str());

                int id = this->unitIDgenerator.nextID();
                this->addAIUnit(id, spawnPoint, depth);
                break;
            }
        }
    }


	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Build octree + do collisions    \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/

	octree.reset(new Octree(Location(0, 0, 0), Location(lvl.max_x(), FixedPoint(400), lvl.max_z())));

	for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter) {
		octree->insertObject(&(iter->second));
	}
	octree->doCollisions();

    for(auto iter = units.begin(); iter != units.end(); ++iter) {
        iter->second.preTick();
    }

	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Tick units and projectiles      \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/

    for(auto iter = items.begin(); iter != items.end(); ++iter)
	{
		Model* model = visualworld->getModel(iter->first);
		itemTicker.tickItem(*this, iter->second, model);
	}

	for(auto iter = units.begin(); iter != units.end(); ++iter)
	{
		Model* model = visualworld->getModel(iter->first);
		unitTicker.tickUnit(*this, iter->second, model);
	}

	for(map<int, Projectile>::iterator iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		Model* model = visualworld->getModel(iter->first);
		projectileTicker.tickProjectile(*this, iter->second, model);
	}

	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Find dead units                 \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/

	for(auto iter = projectiles.begin(); iter != projectiles.end(); ++iter)
	{
		Projectile& projectile = iter->second;
		if(projectile.destroyAfterFrame)
		{
			atDeath(projectile, projectile); // NICE!! :D
			deadUnits.push_back(iter->first);
		}
	}

	for(auto iter = units.begin(); iter != units.end(); ++iter)
	{
		Unit& unit = iter->second;
		if(unit["HEALTH"] < 1) {

            if(unit.intVals["GOD_MODE"]) {
				unit["HEALTH"] = 1000;
				continue;
			}

			unitDeathHandler.doDeathFor(*this, unit);
			atDeath(unit, unit);
		}
	}

	for(auto iter = items.begin(); iter != items.end(); ++iter)
	{
		WorldItem& item = iter->second;
		if(item.dead) {
			deadUnits.push_back(iter->first);
		}
	}

	for(size_t i = 0; i < deadUnits.size(); ++i) {
		removeUnit(deadUnits[i]);
	}
	deadUnits.clear();


	// Graphical things.
	visualworld->tickParticles();
	visualworld->tickLights(units);


	/*  /"\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\
	*     \  Debug stuff                     \
	*     \_/""""""""""""""""""""""""""""""""""/
	*/


	if(show_errors && (currentWorldFrame % 100) == 0)
	{
		for(map<int, Unit>::iterator iter = units.begin(); iter != units.end(); ++iter)
		{
			if(iter->second.human())
			{
				stringstream msg;
				msg << currentWorldFrame << ": " << iter->second.name << " (" << iter->first << "): " << iter->second.position.x.getFloat() << ", " << iter->second.position.z.getFloat();
				add_message(msg.str());
			}
		}
	}
}



void World::addBoxUnit(int id, const Location& location) {
	if(units.find(id) != units.end())
		throw std::logic_error("Trying to create a unit, but the unitID is already reserved.");

    units[id].init();
    units[id].name = "Box";
    units[id].id = id;
    units[id].controllerTypeID = Unit::INANIMATE_OBJECT;
	units[id].scale = 1;
	units[id].position = location;
    units[id].intVals["TEAM"] = -1;

    // should get rid of these still.
	units[id].model_type = VisualWorld::ModelType::BOX_MODEL;
	visualworld->createModel(id, units[id].position, VisualWorld::ModelType::BOX_MODEL, 1.0f);
}

void World::addAIUnit(int id, const Location& pos, int depth)
{
	if(units.find(id) != units.end())
		throw std::logic_error("Trying to create a unit, but the unitID is already reserved.");

    MonsterCreator creator;
	units[id] = creator.createMonster(1, this->currentWorldFrame, id);
	units[id].scale = 1;
	units[id].position = pos;

    RandomMachine random;
    random.setSeed(currentWorldFrame);
    this->itemCreator.fillWithItems(*this, units[id], depth, units[id].getInventoryEditor(), random);

    // should get rid of these still.
    VisualWorld::ModelType model_type = VisualWorld::ModelType::ZOMBIE_MODEL;
	units[id].model_type = model_type;
	visualworld->createModel(id, units[id].position, model_type, 1.0f);
}

void World::addUnit(int id, bool playerCharacter, int team)
{
    assert(playerCharacter);

	if(units.find(id) != units.end())
		throw std::logic_error("Trying to create a unit, but the unitID is already reserved.");

	units[id] = Unit();
	units[id].init();
    units[id].setHumanStart(*this);

	int r_seed = team + 7;
	if(playerCharacter)
	{
		r_seed += 17;
	}

	units[id].position = lvl.getRandomLocation(currentWorldFrame + r_seed);
	units[id].id = id;
	units[id].birthTime = currentWorldFrame;


    units[id].model_type = VisualWorld::ModelType::PLAYER_MODEL;
    visualworld->createModel(id, units[id].position, VisualWorld::ModelType::PLAYER_MODEL, 1.0f);

    units[id].name = "Unknown\\sPlayer";
    units[id].controllerTypeID = Unit::HUMAN_INPUT;
    units[id]["HEALTH"] = 1000;
    units[id]["TEAM"] = 1;
}

void World::addProjectile(Location& location, int id, size_t model_prototype)
{
	vec3<float> position;
	position.x = location.x.getFloat();
	position.y = location.y.getFloat();
	position.z = location.z.getFloat();


	visualworld->createModel(id, location, VisualWorld::ModelType(model_prototype), 0.4f);
	projectiles[id].position = location;
	projectiles[id].prototype_model = model_prototype;
}

void World::addItem(WorldItem& item, VisualWorld::ModelType modelType, int id)
{
    items[id] = item;
    items[id].id = id;
    items[id].intVals["MODEL_TYPE"] = (int)modelType;
    visualworld->createModel(id, items[id].position, modelType, 1.0f);
}

int World::nextUnitID()
{
	return unitIDgenerator.nextID();
}

int World::currentUnitID() const
{
	return unitIDgenerator.currentID();
}

void World::removeUnit(int id)
{
	units.erase(id);
	projectiles.erase(id);
	items.erase(id);
	visualworld->removeUnit(id);
}

int World::getUnitCount()
{
	return units.size();
}

void World::setNextUnitID(int id)
{
	unitIDgenerator.setNextID(id);
}

void World::handle(const GotPlayerID& event)
{
	localPlayerID = event.myID;
}

void World::add_message(const std::string& message) const
{
    ChatMessage msg;
    msg.line = message;
    sendMsg(msg);
}
