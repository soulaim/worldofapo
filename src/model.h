
#ifndef H_MODEL_APODUS
#define H_MODEL_APODUS

#include <string>
#include <vector>

#include "frustum/Vec3.h"

struct ModelNode
{
	ModelNode(): offset_x(0), offset_y(0), offset_z(0), rotation_x(0), rotation_y(0), rotation_z(0)
	{
	}
	
	std::string name;
	std::string wireframe;
	
	float offset_x;
	float offset_y;
	float offset_z;
	
	float rotation_x;
	float rotation_y;
	float rotation_z;
	
	std::vector<int> children;
};


struct Model
{
	std::string animation_name;
	int animation_time;
	
	Model():root(-1) {}
	std::vector<ModelNode> parts;
	int root;
	
	Vec3 realUnitPos;
	Vec3 currentModelPos;
	
	void updatePosition(float x, float y, float z);
	
	void tick();
	void load(const std::string& );
	void setAction(const std::string&);
	
};

#endif

