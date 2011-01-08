#ifndef H_MODEL_APODUS
#define H_MODEL_APODUS

#include <string>
#include <vector>
#include <map>

#include "animation.h"
#include "frustum/vec3.h"
#include "model.h"
#include "primitives.h"

struct ModelNode
{
	ModelNode():
		offset_x(0),
		offset_y(0),
		offset_z(0),
		rotation_x(0),
		rotation_y(0),
		rotation_z(0),
		hilight(false)
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

	bool hilight; // For editor.

	std::vector<size_t> children;
};


struct ApoModel: public Model
{
	std::vector<ModelNode> parts;
	int root;

	ApoModel():
		root(-1)
	{
	}
	
	~ApoModel();

	virtual bool load(const std::string& filename);
	virtual bool save(const std::string& filename) const;
	virtual void draw() const;
	virtual void viewTick();
	
	static std::map<std::string, ObjectPart> objects;
	static bool loadObjects(const std::string&);
	static bool saveObjects(const std::string&);

private:
	void drawPartsRecursive(int current_node) const;
};

#endif

