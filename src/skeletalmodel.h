#ifndef SKELETALMODEL_H
#define SKELETALMODEL_H

#include <string>
#include <vector>
#include <map>

#include "glew/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "animation.h"
#include "frustum/vec3.h"
#include "frustum/matrix4.h"
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
	unsigned bone1;
	unsigned bone2;
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

struct Triangle
{
	unsigned vertices[3];
};

struct SkeletalModel: public Model
{
	SkeletalModel();
	SkeletalModel(const SkeletalModel&);
	float height() const;
	bool load(const std::string& filename);
	bool save(const std::string& filename) const;
	void draw(bool draw_only_skeleton = 0, size_t hilight = -1);
	void draw();
	void rotate_y(float angle);

	void preload();
private:
	void old_draw(size_t hilight);
	void draw_skeleton(const std::vector<Matrix4>& rotations, size_t hilight);
	void calcMatrices(size_t current_bone, std::vector<Matrix4>& rotations, Matrix4 offset, const std::string& animation_name, int animation_state);

	static const size_t BUFFERS = 4;
	GLuint locations[BUFFERS];
	bool buffers_loaded;
	size_t triangles_size;

	// These stay constant over different model instances.
	std::vector<Vec3> vertices;
	std::vector<TextureCoordinate> texture_coordinates;
	std::vector<Triangle> triangles;
	std::vector<WeightedVertex> weighted_vertices;

	// These change with animations.
	std::vector<Bone> bones;

	friend class Editor;
};

#endif

