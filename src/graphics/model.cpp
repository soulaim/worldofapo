#include "model.h"

#include <fstream>
#include <queue>
#include <iomanip>
#include <vector>

using namespace std;

Model::Model():
	animation_time(0)
{
	target_y_rot = 0.f;
	current_y_rot = 0.f;
}

Model::~Model()
{
}

void Model::tick(int current_frame)
{
	// Smoothed out model movement. Doesnt correspond 100% to actual unit coordinates.
	currentModelPos += (realUnitPos - currentModelPos) * 0.2f;
	animation_time = current_frame;
	
	current_y_rot += (target_y_rot - current_y_rot) * 0.03f;
	viewTick();
}

void Model::increase_rot_y(float angle)
{
	target_y_rot += angle;
	if(target_y_rot > 360.f)
	{
		target_y_rot -= 360.f;
		current_y_rot -= 360.f;
	}
	else if(target_y_rot < 0.f)
	{
		target_y_rot += 360.f;
		current_y_rot += 360.f;
	}
}

void Model::rotate_y(float angle)
{
	float direct_distance = (target_y_rot - angle)*(target_y_rot - angle);
	float indirect_distance = 129600.f - direct_distance;
	if( direct_distance < indirect_distance )
	{
		target_y_rot = angle;
	}
	else
	{
		float diff = angle - target_y_rot;
		target_y_rot += diff;
		current_y_rot += diff;
	}
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

