

#include "level.h"
#include <cstdlib>
#include <iostream>

using namespace std;

Location Level::getRandomLocation(int seed)
{
	Location result;
	result.x = ((3  * seed) % 100) * 8;
	result.z = ((17 * seed) % 100) * 8;
	result.y = FixedPoint(100);
	return result;
}


void Level::firstPassNormals()
{
	Location result;
	result.y = 1;
	
	normals.resize(pointheight_info.size());
	for(int i=0; i<normals.size(); i++)
		normals[i].resize(pointheight_info[i].size(), Location());
	
	for(int x=0; x<pointheight_info.size(); x++)
	{
		for(int z=0; z<pointheight_info[x].size(); z++)
		{
			if(x < 0 || x > static_cast<int>(pointheight_info.size()) -2)
				continue;
			if(z < 0 || z > static_cast<int>(pointheight_info[x].size()) -2)
				continue;
			
			normals[x][z] = estimateNormal(x, z) + estimateNormal(x-1, z) + estimateNormal(x+1, z) + estimateNormal(x, z-1) + estimateNormal(x, z+1) + estimateNormal(x+1, z+1) + estimateNormal(x-1, z+1) + estimateNormal(x+1, z-1) + estimateNormal(x-1, z-1);
			normals[x][z].normalize();
		}
	}
}

Location Level::estimateNormal(int x, int z)
{
	Location result;
	result.y = 1;
	if(x < 0 || x > static_cast<int>(pointheight_info.size()) -2)
		return result;
	if(z < 0 || z > static_cast<int>(pointheight_info[x].size()) -2)
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

float Level::estimateHeightDifference(int x, int y)
{
	float min = 10000000;
	float max = 0;
	
	if(pointheight_info[x][y].number < min)
		min = pointheight_info[x][y].number;
	if(pointheight_info[x][y].number > max)
		max = pointheight_info[x][y].number;
	
	x++;
	if(pointheight_info[x][y].number < min)
		min = pointheight_info[x][y].number;
	if(pointheight_info[x][y].number > max)
		max = pointheight_info[x][y].number;
	
	y++;
	if(pointheight_info[x][y].number < min)
		min = pointheight_info[x][y].number;
	if(pointheight_info[x][y].number > max)
		max = pointheight_info[x][y].number;
	
	x--;
	if(pointheight_info[x][y].number < min)
		min = pointheight_info[x][y].number;
	if(pointheight_info[x][y].number > max)
		max = pointheight_info[x][y].number;
	
	return max - min;
}

FixedPoint Level::getHeight(const FixedPoint& x, const FixedPoint& z)
{
	int x_index = x.getInteger() / 8;
	int z_index = z.getInteger() / 8;
	
	int x_desimal = x.getDesimal() + (x.getInteger() & 7) * 1000;
	int z_desimal = z.getDesimal() + (z.getInteger() & 7) * 1000;
	
	if(x_index < 0 || x_index > static_cast<int>(pointheight_info.size()) -2)
		return FixedPoint(0);
	if(z_index < 0 || z_index > static_cast<int>(pointheight_info.size()) -2)
		return FixedPoint(0);
	
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
	
	
	int top_val = C.number + x_desimal * (D.number - C.number) / 8000;
	int bot_val = A.number + x_desimal * (B.number - A.number) / 8000;
	
	FixedPoint height_value;
	height_value.number = bot_val + z_desimal * (top_val - bot_val) / 8000;
	return height_value;
	
}









void Level::generate(int seed)
{
	srand(seed);
	pointheight_info.resize(101);
	for(size_t i = 0; i < pointheight_info.size(); ++i)
	{
		pointheight_info[i].resize( 101 );
		for(size_t k = 0; k < pointheight_info[i].size(); ++k)
		{
			pointheight_info[i][k].number = 3000;
		}
	}
	
	// create long walls
	for(int i=0; i<150; i++)
	{
		
		int x_p = rand() % pointheight_info.size();
		int y_p = rand() % pointheight_info[x_p].size();
		
		for(int k=0; k<15; k++)
		{
			pointheight_info[x_p][y_p].number = 28000;
			x_p += (rand() % 3) - 1;
			y_p += (rand() % 3) - 1;
			
			if(x_p < 0 || x_p >= pointheight_info.size())
				break;
			if(y_p < 0 || y_p >= pointheight_info[x_p].size())
				break;
		}
	}
	
	// create some accessible higher ground
	for(int i=0; i<150; i++)
	{
		
		int x_p = rand() % pointheight_info.size();
		int y_p = rand() % pointheight_info[x_p].size();
		int height = 3000;
		
		for(int k=0; k<10; k++)
		{
			height += 1000;
			pointheight_info[x_p][y_p].number = height;
			x_p += (rand() % 3) - 1;
			y_p += (rand() % 3) - 1;
			
			if(x_p < 0 || x_p >= pointheight_info.size())
				break;
			if(y_p < 0 || y_p >= pointheight_info[x_p].size())
				break;
		}
	}
	
	firstPassNormals(); // calculate level normals
	
}





int Level::max_x() const
{
	return pointheight_info.size() * 8 - 8;
}

int Level::max_z() const
{
	return pointheight_info.front().size() * 8 - 8;
}

