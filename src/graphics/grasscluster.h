#ifndef GRASSCLUSTER_H
#define GRASSCLUSTER_H

#include <string>
#include <vector>
#include <map>
#include <cassert>

#include "opengl.h"
#include "frustum/vec3.h"
#include "frustum/matrix4.h"

class Shader;

struct GrassCluster
{
	GrassCluster();
	~GrassCluster();
	
	void draw(Shader& shader) const;
	void preload();
	void unload();
	
	Vec3 center;
	float radius;
	std::vector<Vec3> bushes;
	
private:
	void draw_old(Shader& shader) const;
	void draw_fbo(Shader& shader) const;

	static const size_t BUFFERS = 1;
	GLuint locations[BUFFERS];
	bool buffers_loaded;
};

#endif

