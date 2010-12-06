#include "grasscluster.h"
#include "frustum/matrix4.h"
#include "texturehandler.h"

#include <fstream>
#include <queue>
#include <iomanip>
#include <iostream>
#include <cassert>

#include "shaders.h"

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

void GrassCluster::preload()
{
//	std::cerr << "Preloading grasscluster buffers." << std::endl;

	assert(!buffers_loaded);
	glGenBuffers(BUFFERS, locations);

	size_t buffer = 0;
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, bushes.size() * sizeof(Vec3), &bushes[0], GL_STATIC_DRAW);

	assert(buffer == BUFFERS);

	buffers_loaded = true;
}

void GrassCluster::unload()
{
	std::cerr << "Unloading grasscluster buffers." << std::endl;

	if(buffers_loaded)
	{
		glDeleteBuffers(BUFFERS, locations);
	}
	buffers_loaded = false;
}

void GrassCluster::draw_fbo(const Shaders*) const
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

void GrassCluster::draw_old(const Shaders* shaders) const
{
	glBegin(GL_POINTS);
	for(size_t i = 0; i < bushes.size(); ++i)
	{
		const Vec3& v = /*center +*/ bushes[i];
//		const Vec3& wind = winds[i];
		Vec3 wind(0,0,0);
		float scale = 1.0;

		glVertexAttrib1f(shaders->uniform("grass_scale"), scale);
		glVertexAttrib3f(shaders->uniform("grass_wind"), wind.x, wind.y, wind.z);
		glVertex3f(v.x, v.y, v.z);

		QUADS_DRAWN_THIS_FRAME += 3;
	}
	glEnd();
}

void GrassCluster::draw(const Shaders* shaders) const
{
//	glColor3f(1.0, 1.0, 1.0);
//	glPointSize(5.0f);

	glDisable(GL_CULL_FACE);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glAlphaFunc( GL_GREATER, 0.5 ) ;
	glEnable( GL_ALPHA_TEST ) ;

	draw_fbo(shaders);
//	draw_old(shaders);

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_CULL_FACE);
}

