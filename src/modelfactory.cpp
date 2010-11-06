#include "modelfactory.h"
#include "logger.h"
#include "apomodel.h"
#include "skeletalmodel.h"

#include <iostream>
#include <memory>
#include <cassert>

using namespace std;


enum Modeltype { NO_MODEL, SKELETALMODEL, APOMODEL };
std::vector<std::shared_ptr<Model> > prototypes;
std::vector<Modeltype> types;

bool ModelFactory::load(size_t prototype, const std::string& filename)
{
	Logger log;

	if(prototypes.size() <= prototype)
	{
		prototypes.resize(prototype + 1);
		types.resize(prototype + 1);
	}
	if(prototypes[prototype])
	{
		cerr << "WARNING: model with prototype " << prototype << " already loaded, reloading with file '" << filename << "'\n";
	}

	log << "Loading model '" << filename << "' with prototype " << prototype << "\n";

	prototypes[prototype].reset();
	types[prototype] = NO_MODEL;
	if(filename.find(".skeleton") != string::npos)
	{
		prototypes[prototype].reset(new SkeletalModel);
		types[prototype] = SKELETALMODEL;
	}
	else if(filename.find(".bones") != string::npos)
	{
		prototypes[prototype].reset(new ApoModel);
		types[prototype] = APOMODEL;
	}
	else
	{
		cerr << "Bad filename when loading prototype " << prototype << " model from file '" << filename << "'\n";
		return false;
	}

	return prototypes[prototype]->load(filename);
}

Model* ModelFactory::create(size_t prototype)
{
	if(prototypes.size() <= prototype || !prototypes[prototype])
	{
		cerr << "ERROR: model with prototype " << prototype << " not loaded!\n";
		throw std::string("Error creating model");
	}
	assert(types.size() == prototypes.size());
	Model* model = prototypes[prototype].get();
	switch(types[prototype])
	{
		case APOMODEL:
			return new ApoModel( *(ApoModel*)model );
		case SKELETALMODEL:
			return new SkeletalModel( *(SkeletalModel*)model );
		case NO_MODEL:
			return 0;
	}
	return 0;
}

void ModelFactory::destroy(Model* model)
{
	delete model;
}

