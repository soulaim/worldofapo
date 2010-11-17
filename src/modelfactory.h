#ifndef MODELFACTORY_H
#define MODELFACTORY_H

#include "model.h"

#include <string>

class ModelFactory
{
public:
	static bool load(size_t prototype, const std::string& filename, const std::string& texture_name);
	static Model* create(size_t prototype);
	static void destroy(Model* model);
};

#endif

