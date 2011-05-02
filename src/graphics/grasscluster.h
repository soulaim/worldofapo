#ifndef GRASSCLUSTER_H
#define GRASSCLUSTER_H

#include "graphics/opengl.h"
#include "misc/vec3.h"
#include "graphics/frustum/matrix4.h"

#include <string>
#include <vector>
#include <map>
#include <cassert>

class Shader;

struct GrassCluster
{
	GrassCluster();
	~GrassCluster();
	
	void draw(Shader& shader) const;
	void preload();
	void unload();
	
	vec3<float> center;
	float radius;
	std::vector<vec3<float> > bushes;
	
private:
	void draw_old(Shader& shader) const;
	void draw_fbo(Shader& shader) const;

	static const size_t BUFFERS = 1;
	GLuint locations[BUFFERS];
	bool buffers_loaded;
};

#endif

