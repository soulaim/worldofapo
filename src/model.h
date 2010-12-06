#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <map>

#include "animation.h"
#include "frustum/vec3.h"

class Model
{
public:
	std::string texture_name;
	std::string animation_name;
	int animation_time;

	Vec3 realUnitPos;
	Vec3 currentModelPos;

	Model();
	virtual ~Model();

	void updatePosition(float x, float y, float z);
	void tick(int current_frame);
	void setAction(const std::string&);

	virtual bool load(const std::string& filename) = 0;
	virtual bool save(const std::string& filename) const = 0;
	virtual void draw() const = 0;
	virtual void rotate_y(float angle) = 0;
};

#endif

