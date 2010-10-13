

#include "level.h"
#include <cstdlib>
#include <iostream>

using namespace std;

Level::Level()
{
	fpZero = FixedPoint(0);
	
	unitVectorUp.x = FixedPoint(0);
	unitVectorUp.y = FixedPoint(1);
	unitVectorUp.z = FixedPoint(0);
	
	
	pointheight_info.resize(101);
	for(size_t i = 0; i < pointheight_info.size(); ++i)
	{
		pointheight_info[i].resize( 101 );
		for(size_t k = 0; k < pointheight_info[i].size(); ++k)
		{
			pointheight_info[i][k] = FixedPoint(3);
		}
	}
	
	normals.resize(pointheight_info.size());
	for(size_t i = 0; i < normals.size(); ++i)
		normals[i].resize(pointheight_info[i].size(), Location());
	
	h_diff.resize(pointheight_info.size());
	for(size_t i = 0; i < normals.size(); ++i)
		h_diff[i].resize(pointheight_info[i].size(), FixedPoint(0));
}

Location Level::getRandomLocation(int seed)
{
	Location result;
	result.x = ((43  * seed) % 100) * 8;
	result.z = ((73 * seed) % 100) * 8;
	result.y = FixedPoint(100);
	return result;
}

void Level::updateHeight(int x, int z, FixedPoint h)
{
	
	if(x < 0 || x > static_cast<int>(pointheight_info.size()) - 1)
	{
		return;
	}
	if(z < 0 || z > static_cast<int>(pointheight_info[x].size()) - 1)
	{
		return;
	}
	
	pointheight_info[x][z] = h;
	
	updateNormal(x, z);
	updateNormal(x+1, z);
	updateNormal(x-1, z);
	updateNormal(x, z+1);
	updateNormal(x, z-1);
	updateNormal(x+1, z+1);
	updateNormal(x-1, z+1);
	updateNormal(x+1, z-1);
	updateNormal(x-1, z-1);
	
	updateHeightDifference(x, z);
	updateHeightDifference(x+1, z);
	updateHeightDifference(x-1, z);
	updateHeightDifference(x, z+1);
	updateHeightDifference(x, z-1);
	updateHeightDifference(x+1, z+1);
	updateHeightDifference(x-1, z+1);
	updateHeightDifference(x+1, z-1);
	updateHeightDifference(x-1, z-1);
}

void Level::updateNormal(int x, int z)
{
	Location result;
	result.y = 1;
	
	if(x < 0 || x > static_cast<int>(pointheight_info.size()) - 1)
		return;
	if(z < 0 || z > static_cast<int>(pointheight_info[x].size()) - 1)
		return;
	
	normals[x][z] = estimateNormal(x, z) + estimateNormal(x-1, z) + estimateNormal(x+1, z) + estimateNormal(x, z-1) + estimateNormal(x, z+1) + estimateNormal(x+1, z+1) + estimateNormal(x-1, z+1) + estimateNormal(x+1, z-1) + estimateNormal(x-1, z-1);
	normals[x][z].normalize();
}

Location Level::estimateNormal(int x, int z)
{
	Location result;
	if(x < 0 || x > static_cast<int>(pointheight_info.size()) - 2)
		return result;
	if(z < 0 || z > static_cast<int>(pointheight_info[x].size()) - 2)
		return result;
	
	Location a;
	a.x = x*8;
	a.z = z*8;
	a.y = pointheight_info[x][z];
	
	Location b;
	b.x = x*8;
	b.z = z*8+8;
	b.y = pointheight_info[x][z+1];
	
	Location c;
	c.x = x*8+8;
	c.z = z*8;
	c.y = pointheight_info[x+1][z];
	
	result = (c-a).crossProduct(b-a);
	result.normalize();
	
	return result;
}

void Level::updateHeightDifference(int x, int z)
{
	if(x < 0 || x > static_cast<int>(pointheight_info.size()) - 2)
		return;
	if(z < 0 || z > static_cast<int>(pointheight_info[x].size()) - 2)
		return;
	h_diff[x][z] = estimateHeightDifference(x, z);
}

FixedPoint Level::estimateHeightDifference(int x, int y) const
{
	FixedPoint min = 10000000;
	FixedPoint max = 0;
	
	if(pointheight_info[x][y] < min)
		min = pointheight_info[x][y];
	if(pointheight_info[x][y] > max)
		max = pointheight_info[x][y];
	
	x++;
	if(pointheight_info[x][y] < min)
		min = pointheight_info[x][y];
	if(pointheight_info[x][y] > max)
		max = pointheight_info[x][y];
	
	y++;
	if(pointheight_info[x][y] < min)
		min = pointheight_info[x][y];
	if(pointheight_info[x][y] > max)
		max = pointheight_info[x][y];
	
	x--;
	if(pointheight_info[x][y] < min)
		min = pointheight_info[x][y];
	if(pointheight_info[x][y] > max)
		max = pointheight_info[x][y];
	
	return max - min;
}



const FixedPoint& Level::getHeightDifference(FixedPoint& x, FixedPoint& z) const
{
	int x_index = x.getInteger() / 8;
	int z_index = z.getInteger() / 8;
	
	// These checks should not have to be necessary
	if(x_index < 0 || x_index > static_cast<int>(pointheight_info.size()) - 2)
		return fpZero;
	if(z_index < 0 || z_index > static_cast<int>(pointheight_info.size()) - 2)
		return fpZero;
	
	return h_diff[x_index][z_index];
}

const FixedPoint& Level::getHeightDifference(int x_index, int z_index) const
{
	// These checks should not have to be necessary
	if(x_index < 0 || x_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint::ZERO;
	if(z_index < 0 || z_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint::ZERO;
	return h_diff[x_index][z_index];
}

const Location& Level::getNormal(FixedPoint& x, FixedPoint& z) const
{
	int x_index = x.getInteger() / 8;
	int z_index = z.getInteger() / 8;
	return normals[x_index][z_index];
}

const Location& Level::getNormal(int x_index, int z_index) const
{
	return normals[x_index][z_index];
}




FixedPoint Level::getHeight(const FixedPoint& x, const FixedPoint& z) const
{
	int x_index = x.getInteger() / 8;
	int z_index = z.getInteger() / 8;

	if(x_index < 0 || x_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint(1);
	if(z_index < 0 || z_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint(1);
	
	FixedPoint A = pointheight_info[x_index][z_index];
	FixedPoint B = pointheight_info[x_index+1][z_index];
	
	FixedPoint C = pointheight_info[x_index][z_index+1];
	FixedPoint D = pointheight_info[x_index+1][z_index+1];
	
	/*
	if(x_desimal + z_desimal < 8000)
	{
		// use bottom triangle
		FixedPoint y_from_x = A + (B - A) * FixedPoint(x_desimal, true) / FixedPoint(8);
		FixedPoint y_from_z = A + (C - A) * FixedPoint(z_desimal, true) / FixedPoint(8);
		return y_from_x + y_from_z;
	}
	else
	{
		// use top triangle
		FixedPoint y_from_x; y_from_x.number = D.number + (C - D).number * (8000 - x_desimal) / 8000;
		FixedPoint y_from_z; y_from_z.number = D.number + (B - D).number * (8000 - z_desimal) / 8000;
		return y_from_x + y_from_z;
	}
	*/

	FixedPoint remainder_x = x - FixedPoint(x_index * 8);
	FixedPoint remainder_z = z - FixedPoint(z_index * 8);

	FixedPoint top_val = C + (D - C) * remainder_x / FixedPoint(8);
	FixedPoint bot_val = A + (B - A) * remainder_x / FixedPoint(8);
	
	FixedPoint height_value;
	height_value = bot_val + (top_val - bot_val) * remainder_z / FixedPoint(8);
	return height_value;
	
}




void Level::generate(int seed)
{
	srand(seed);
	
	for(size_t i = 0; i < pointheight_info.size(); ++i)
		for(size_t k = 0; k < pointheight_info[i].size(); ++k)
			updateHeight(i, k, FixedPoint(0));
	
	
	// create long walls
	for(int i=0; i<150; i++)
	{
		
		int x_p = rand() % pointheight_info.size();
		int y_p = rand() % pointheight_info[x_p].size();
		
		for(int k=0; k<15; k++)
		{
			updateHeight(x_p, y_p, FixedPoint(28));
			x_p += (rand() % 3) - 1;
			y_p += (rand() % 3) - 1;
		}
	}
	
	// create some accessible higher ground
	for(int i=0; i<150; i++)
	{
		
		int x_p = rand() % pointheight_info.size();
		int y_p = rand() % pointheight_info[x_p].size();
		FixedPoint height = FixedPoint(3);
		
		for(int k=0; k<10; k++)
		{
			height += FixedPoint(1);
			updateHeight(x_p, y_p, height);
			
			x_p += (rand() % 3) - 1;
			y_p += (rand() % 3) - 1;
		}
	}
	
	
	// create bounding mountains
	for(size_t i = 0; i < pointheight_info.size(); ++i)
	{
		updateHeight(i, 0, FixedPoint(80));
		updateHeight(0, i, FixedPoint(80));
		updateHeight(i, pointheight_info[i].size()-1, FixedPoint(80));
		updateHeight(pointheight_info.size()-1, i, FixedPoint(80));
	}

}


FixedPoint Level::getJumpPower(FixedPoint& x, FixedPoint& z)
{
	int x_index = x.getInteger() / 8;
	int z_index = z.getInteger() / 8;
	
	if(x_index < 0 || x_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint(1, 2);
	if(z_index < 0 || z_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint(1, 2);
	
	return normals[x_index][z_index].dotProduct(unitVectorUp);
}


int Level::max_x() const
{
	return pointheight_info.size() * 8 - 8;
}

int Level::max_z() const
{
	return pointheight_info.front().size() * 8 - 8;
}

