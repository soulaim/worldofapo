
#ifndef PARTICLE_SOURCE_H
#define PARTICLE_SOURCE_H

#include "movable_object.h"
#include "item.h"

#include "particle.h"

class ParticleSource: MovableObject, HasProperties
{
	friend class World;
public:
	std::string& getStrProperty(const std::string&);
	int& getIntProperty(const std::string&);
	
	const std::string& getStrProperty(const std::string&) const;
	const int& getIntProperty(const std::string&) const;
	
	void tick(std::vector<Particle>&); // generate particles for current frame
	bool alive() const;
	
	Location particleDirection;
};

#endif
