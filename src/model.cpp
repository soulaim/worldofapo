#include "apomodel.h"
#include "primitives.h"
#include "graphics.h"

#include <GL/gl.h>

#include <fstream>
#include <queue>
#include <iomanip>

using namespace std;

// ALERT: Why are these here..?
extern GLint unit_color_location;
extern int TRIANGLES_DRAWN_THIS_FRAME;

Model::Model():
	animation_time(0)
{
}

Model::~Model()
{
}

void Model::tick(int current_frame)
{
	// Smoothed out model movement. Doesnt correspond 100% to actual unit coordinates.
	currentModelPos += (realUnitPos - currentModelPos) * 0.2;
	animation_time = current_frame;
}

void Model::updatePosition(float x, float y, float z)
{
	realUnitPos.x = x;
	realUnitPos.y = y;
	realUnitPos.z = z;
}

void Model::setAction(const string& name)
{
	if(name == animation_name)
		return;
	
	animation_name = name;
	animation_time = 0;
}

