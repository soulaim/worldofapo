#include "graphics/models/modelfactory.h"
#include "graphics/models/skeletalmodel.h"
#include "graphics/models/invisiblemodel.h"

#include "misc/logger.h"

#include <iostream>
#include <memory>
#include <cassert>
#include <stdexcept>

using namespace std;


enum Modeltype { NOMODEL, SKELETALMODEL, INVISIBLEMODEL };
std::vector<std::shared_ptr<Model> > prototypes;
std::vector<Modeltype> types;

bool ModelFactory::load(size_t prototype, const std::string& filename, const std::string& texture_name)
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
	types[prototype] = NOMODEL;
	if(filename.empty())
	{
		prototypes[prototype].reset(new InvisibleModel);
		types[prototype] = INVISIBLEMODEL;
	}
	else if(filename.find(".sm2") != string::npos)
	{
		prototypes[prototype].reset(new SkeletalModel);
		types[prototype] = SKELETALMODEL;
	}
	else
	{
		cerr << "Bad filename when loading prototype " << prototype << " model from file '" << filename << "'\n";
		return false;
	}

	bool ok = prototypes[prototype]->load(filename);
	if(ok)
	{
		prototypes[prototype]->texture_name = texture_name;
	}
	if(ok && types[prototype] == SKELETALMODEL)
	{
		SkeletalModel* model = static_cast<SkeletalModel*>(prototypes[prototype].get());
		model->preload();
	}
	return ok;
}

Model* ModelFactory::create(size_t prototype)
{
	if(prototypes.size() <= prototype || !prototypes[prototype])
	{
		cerr << "ERROR: model with prototype " << prototype << " not loaded!\n";
		throw std::logic_error("Error creating model");
	}
	assert(types.size() == prototypes.size());
	Model* model = prototypes[prototype].get();
	Model* ret = 0;
	switch(types[prototype])
	{
		case SKELETALMODEL:
			ret = new SkeletalModel( *(SkeletalModel*)model );
			break;
		case INVISIBLEMODEL:
			ret = new InvisibleModel(*(InvisibleModel*)model );
			break;
		case NOMODEL:
			ret = 0;
	}
	if(!ret)
	{
		cerr << "Request to create model from prototype " << prototype << " returned 0." << endl;
	}
	return ret;
}

void ModelFactory::destroy(Model* model)
{
	delete model;
}

