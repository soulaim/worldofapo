#ifndef SKELETALMODEL_H
#define SKELETALMODEL_H

#include "graphics/opengl.h"
#include "graphics/models/animation.h"
#include "graphics/models/model.h"
#include "misc/vec3.h"
#include "graphics/frustum/matrix4.h"
#include "graphics/texturecoordinate.h"


#include <string>
#include <vector>
#include <map>
#include <cassert>

struct Bone
{
	Bone():
		root(false),
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

	bool root;
	std::string name;

	float start_x;
	float start_y;
	float start_z;
	float start_r;

	float end_x;
	float end_y;
	float end_z;
	float end_r;

	float w;

	float rotation_x;
	float rotation_y;
	float rotation_z;

	std::vector<std::string> children_names;
	std::vector<size_t> children;
};

struct WeightedVertex
{
	// TODO: make bone datatypes unsigned chars (i.e. fix passing to opengl and the use in shader).
	unsigned bone1;
	unsigned bone2;

//	unsigned char padding1;
//	unsigned char padding2;

	float weight1;
	float weight2;

	WeightedVertex()
	{
		assert(sizeof(WeightedVertex) == sizeof(float)*2 + sizeof(unsigned)*2);

		bone1 = 0;
		bone2 = 0;
		weight1 = 1.0f;
		weight2 = 0.0f;
	}
};

struct ColorStruct
{
	float r, g, b, a;
};

struct Triangle
{
	unsigned short vertices[3];

	Triangle()
	{
		assert(sizeof(Triangle) == sizeof(unsigned short)*3);
	}
};

struct SkeletalModel: public Model
{
	virtual bool load(const std::string& filename);
	virtual bool save(const std::string& filename) const;
	virtual void draw() const;
	virtual void viewTick();

	SkeletalModel();
	SkeletalModel(const SkeletalModel&);
	void draw(bool draw_only_skeleton = 0, size_t hilight = -1) const;
	void preload();

private:
	void old_draw(size_t hilight) const;
	void draw_skeleton(const std::vector<Matrix4>& rotations, size_t hilight) const;
	void draw_normals() const;
	void calcMatrices(size_t current_bone, std::vector<Matrix4>& rotations, Matrix4 offset, const std::string& animation_name, int animation_state) const;
	void draw_buffers() const;

	static const size_t BUFFERS = 5;
	GLuint locations[BUFFERS];
	bool buffers_loaded;
	size_t triangles_size;

	void calculate_weights();

	// These stay constant over different model instances.
	std::vector<vec3<float> > vertices;
	std::vector<vec3<float> > normals;
	std::vector<ColorStruct> colors;
	std::vector<TextureCoordinate> texture_coordinates;

	std::vector<Triangle> triangles;
	std::vector<vec3<float> > triangle_normals;
	std::vector<float> triangle_areas;

	std::vector<WeightedVertex> weighted_vertices;

	// These change with animations.
	std::vector<Bone> bones;

	friend class Editor;
};

#endif

