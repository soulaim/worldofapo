#include "model.h"

#include <fstream>
#include <queue>
#include <iomanip>
#include <vector>

using namespace std;

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

void Model::setScale(float newScale)
{
	myScale = newScale;
}

void Model::changeScale(float delta)
{
	myScale *= delta;
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

