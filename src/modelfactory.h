#ifndef MODELFACTORY_H
#define MODELFACTORY_H

#include "model.h"

#include <string>

class ModelFactory
{
public:
	enum { PLAYER_MODEL, BULLET_MODEL };

	static bool load(size_t prototype, const std::string& filename);
	static Model* create(size_t prototype);
	static void destroy(Model* model);
};

#endif

