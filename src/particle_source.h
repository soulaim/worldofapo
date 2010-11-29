
#ifndef PARTICLE_SOURCE_H
#define PARTICLE_SOURCE_H

#include "movable_object.h"
#include "item.h"

#include "particle.h"

class ParticleSource: public MovableObject, public HasProperties
{
	friend class World;
	
	int esred, esgreen, esblue;
	int ssred, ssgreen, ssblue;
	
	int sered, segreen, seblue;
	int eered, eegreen, eeblue;
	
public:
	std::string& getStrProperty(const std::string&);
	int& getIntProperty(const std::string&);
	
	void setColors(const std::string&, const std::string&, const std::string&, const std::string&);
	
	const std::string& getStrProperty(const std::string&) const;
	const int& getIntProperty(const std::string&) const;
	
	void tick(std::vector<Particle>&); // generate particles for current frame
	bool alive() const;
};

#endif
