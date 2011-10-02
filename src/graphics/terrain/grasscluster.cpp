#include "graphics/terrain/grasscluster.h"
#include "graphics/frustum/matrix4.h"
#include "graphics/texturehandler.h"
#include "graphics/shader.h"

#include <fstream>
#include <queue>
#include <iomanip>
#include <iostream>
#include <cassert>

using namespace std;

extern int QUADS_DRAWN_THIS_FRAME;

GrassCluster::GrassCluster():
	radius(9999999.0f),
	buffers_loaded(false)
{
	for(size_t i = 0; i < BUFFERS; ++i)
	{
		locations[i] = -1;
	}
}

GrassCluster::~GrassCluster()
{
}

void GrassCluster::preload()
{
//	std::cerr << "Preloading grasscluster buffers." << std::endl;

	assert(!buffers_loaded);
	glGenBuffers(BUFFERS, locations);

	size_t buffer = 0;
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, bushes.size() * sizeof(vec3<float>), &bushes[0], GL_STATIC_DRAW);

	assert(buffer == BUFFERS);

	buffers_loaded = true;
}

void GrassCluster::unload()
{
	if(buffers_loaded)
	{
//		std::cerr << "Unloading grasscluster buffers." << std::endl;

		glDeleteBuffers(BUFFERS, locations);
	}
	buffers_loaded = false;
}

void GrassCluster::draw_fbo(Shader&) const
{
	assert(buffers_loaded);
	size_t buffer = 0;

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glVertexPointer(3, GL_FLOAT, 0, 0);

//	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
//	glVertexAttribPointer(bone_index_location,  2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(WeightedVertex), 0);

	assert(buffer == BUFFERS);

	glEnableClientState(GL_VERTEX_ARRAY);
//	glEnableVertexAttribArray(bone_weight_location);
	glDrawArrays(GL_POINTS, 0, bushes.size());
//	glDisableVertexAttribArray(bone_index_location);
	glDisableClientState(GL_VERTEX_ARRAY);

	QUADS_DRAWN_THIS_FRAME += bushes.size() * 3;
}

void GrassCluster::draw_old(Shader& shader) const
{
	glBegin(GL_POINTS);
	for(size_t i = 0; i < bushes.size(); ++i)
	{
		const vec3<float>& v = /*center +*/ bushes[i];
		// const vec3<float>& wind = winds[i];
		vec3<float> wind(0,0,0);
		float scale = 1.0;

		glVertexAttrib1f(shader.uniform("scale"), scale);
		glVertexAttrib3f(shader.uniform("wind"), wind.x, wind.y, wind.z);
		glVertex3f(v.x, v.y, v.z);

		QUADS_DRAWN_THIS_FRAME += 3;
	}
	glEnd();
}

void GrassCluster::draw(Shader& shader) const
{
	glDisable(GL_CULL_FACE);

	draw_fbo(shader);
//	draw_old(shader);

	glEnable(GL_CULL_FACE);
}

