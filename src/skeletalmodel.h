#ifndef SKELETALMODEL_H
#define SKELETALMODEL_H

#include <string>
#include <vector>
#include <map>

#include "animation.h"
#include "frustum/Vec3.h"
#include "primitives.h"
#include "model.h"

struct Bone
{
	Bone():
		start_x(0),
		start_y(0),
		start_z(0),
		end_x(0),
		end_y(0),
		end_z(0),
		rotation_x(0),
		rotation_y(0),
		rotation_z(0)
	{
	}

	std::string name;

	float start_x;
	float start_y;
	float start_z;

	float end_x;
	float end_y;
	float end_z;

	float rotation_x;
	float rotation_y;
	float rotation_z;

	std::vector<size_t> children;
};

struct WeightedVertex
{
	size_t bone1;
	size_t bone2;
	float weight1;
	float weight2;

	WeightedVertex()
	{
		bone1 = 0;
		bone2 = 0;
		weight1 = 1.0f;
		weight2 = 0.0f;
	}
};

struct TextureCoordinate
{
	float x;
	float y;
};

struct Triangle
{
	size_t vertices[3];
};

struct SkeletalModel: public Model
{
	// These stay constant over different model instances.
	std::vector<Vec3> vertices;
	std::vector<TextureCoordinate> texture_coordinates;
	std::vector<Triangle> triangles;
	std::vector<WeightedVertex> weighted_vertices;

	// These change with animations.
	std::vector<Bone> bones;

	float height() const;
	bool load(const std::string& filename);
	bool save(const std::string& filename) const;
	void draw(bool draw_skeleton = 0, size_t hilight = -1);
	void draw();
	void rotate_y(float angle);
};

#endif

