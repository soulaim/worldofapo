#ifndef INVISIBLEMODEL_H
#define INVISIBLEMODEL_H

#include "model.h"

#include <string>

struct InvisibleModel: public Model
{
	virtual bool load(const std::string& filename);
	virtual bool save(const std::string& filename) const;
	virtual void draw() const;
	virtual void viewTick();
	
	InvisibleModel();
	InvisibleModel(const InvisibleModel&);
};

#endif

