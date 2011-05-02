#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>
#include <map>

#include "graphics/animation.h"
#include "misc/vec3.h"

class Model
{
public:
	std::string texture_name;
	std::string animation_name;
	int animation_time;

	vec3<float> realUnitPos;
	vec3<float> currentModelPos;
	
	float myScale;
	
	float target_y_rot;
	float current_y_rot;
	
	Model();
	virtual ~Model();

	// access to model scale
	void setScale(float);
	void changeScale(float delta);
	
	void updatePosition(float x, float y, float z);
	void tick(int current_frame);
	void setAction(const std::string&);
	
	void rotate_y(float angle);
	void increase_rot_y(float angle);
	
	virtual bool load(const std::string& filename) = 0;
	virtual bool save(const std::string& filename) const = 0;
	virtual void draw() const = 0;
	virtual void viewTick() = 0;
};

#endif

