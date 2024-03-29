#include "world/level/level.h"
#include "algorithms.h"
#include "physics/movable_object.h"
#include "world/world.h"

#include <cstdlib>
#include <iostream>
#include <fstream>

using namespace std;

// level size should be ((2^n) + 1) because binary triangle trees work best then.
#define LEVEL_LVLSIZE 129

Level::Level()
{
	size_t level_x_size = LEVEL_LVLSIZE;
	size_t level_z_size = LEVEL_LVLSIZE;

	unitVectorUp.x = FixedPoint(0);
	unitVectorUp.y = FixedPoint(1);
	unitVectorUp.z = FixedPoint(0);


	pointheight_info.resize(level_x_size);
	for(size_t i = 0; i < pointheight_info.size(); ++i)
	{
		pointheight_info[i].resize(level_z_size);
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

Location Level::getRandomLocation(int seed) const
{
	Location result;
	result.x = ((173  * seed) % max_block_x()) * BLOCK_SIZE;
	result.z = ((833 * seed) % max_block_z()) * BLOCK_SIZE;
	result.y = getHeight(result.x, result.z) + FixedPoint(5);
	return result;
}

const Location& Level::getStartLocation() const
{
    return startPosition;
}

void Level::clampToLevelArea(MovableObject& object) {
	if(object.position.x < 0)
	{
		object.position.x = 0;
		if(object.velocity.x < 0)
			object.velocity.x = 0;
	}
	if(object.position.x > max_x())
	{
		object.position.x = max_x();
		if(object.velocity.x > 0)
			object.velocity.x = 0;
	}
	if(object.position.z < 0)
	{
		object.position.z = 0;
		if(object.velocity.z < 0)
			object.velocity.z = 0;
	}
	if(object.position.z > max_z())
	{
		object.position.z = max_z();
		if(object.velocity.z > 0)
			object.velocity.z = 0;
	}
    if(object.position.y > 10) {
        object.position.y = 10;
        object.velocity.y = FixedPoint(-1, 10);
    }
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

	normals[x][z] = estimateNormal(x, z) + estimateNormal(x-1, z) + estimateNormal(x+1, z) + estimateNormal(x, z-1) + estimateNormal(x, z+1); // + estimateNormal(x+1, z+1) + estimateNormal(x-1, z+1) + estimateNormal(x+1, z-1) + estimateNormal(x-1, z-1);

	if(normals[x][z].length() == FixedPoint(0))
	{
		cerr << "update normal trying to normalize length 0 vector" << endl;
	}
	else
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
	a.x = x*BLOCK_SIZE;
	a.z = z*BLOCK_SIZE;
	a.y = pointheight_info[x][z];

	Location b;
	b.x = x*BLOCK_SIZE;
	b.z = z*BLOCK_SIZE+BLOCK_SIZE;
	b.y = pointheight_info[x][z+1];

	Location c;
	c.x = x*BLOCK_SIZE+BLOCK_SIZE;
	c.z = z*BLOCK_SIZE;
	c.y = pointheight_info[x+1][z];

	result = (b-a).crossProduct(c-a);
	if(result.length() == FixedPoint(0))
	{
		cerr << "estimate normal trying to normalize length 0 vector" << endl;
	}
	else
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
	int x_index = x.getInteger() / BLOCK_SIZE;
	int z_index = z.getInteger() / BLOCK_SIZE;

	// These checks should not have to be necessary
	if(x_index < 0 || x_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint::ZERO;
	if(z_index < 0 || z_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint::ZERO;

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
	int x_index = x.getInteger() / BLOCK_SIZE;
	int z_index = z.getInteger() / BLOCK_SIZE;
	return normals[x_index][z_index];
}

const Location& Level::getNormal(int x_index, int z_index) const
{
	return normals[x_index][z_index];
}


const FixedPoint& Level::getVertexHeight(int x, int z) const
{
	return pointheight_info[x][z];
}

FixedPoint Level::getHeight(const FixedPoint& x, const FixedPoint& z) const
{
	int x_index = x.getInteger() / BLOCK_SIZE;
	int z_index = z.getInteger() / BLOCK_SIZE;

	if(x_index < 0 || x_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint(1);
	if(z_index < 0 || z_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint(1);

	FixedPoint x_desimal = (x - x_index * BLOCK_SIZE) / BLOCK_SIZE;
	FixedPoint z_desimal = (z - z_index * BLOCK_SIZE) / BLOCK_SIZE;

	const FixedPoint& A = pointheight_info[x_index][z_index];
	const FixedPoint& B = pointheight_info[x_index+1][z_index];

	const FixedPoint& C = pointheight_info[x_index][z_index+1];
	const FixedPoint& D = pointheight_info[x_index+1][z_index+1];

	Location pA(BLOCK_SIZE*x_index,     A, BLOCK_SIZE*z_index);
	Location pB(BLOCK_SIZE*(x_index+1), B, BLOCK_SIZE*z_index);
	Location pC(BLOCK_SIZE*x_index,     C, BLOCK_SIZE*(z_index+1));
	Location pD(BLOCK_SIZE*(x_index+1), D, BLOCK_SIZE*(z_index+1));

	const Location* p1 = 0;
	const Location* p2 = 0;
	const Location* p3 = 0;

	if( ((z_index + x_index) & 1) == 0)
	{
		p1 = &pD;
		p2 = &pA;
		if(x_desimal < z_desimal)
		{
			// working in upper triangle (CDA)
			p3 = &pC;
		}
		else
		{
			// working in lower triangle (ABD)
			p3 = &pB;
		}
	}
	else
	{
		p1 = &pB;
		p2 = &pC;
		if(FixedPoint(1) - x_desimal < z_desimal)
		{
			// upper triangle (CDB)
			p3 = &pD;
		}
		else
		{
			// lower triangle (ABC)
			p3 = &pA;
		}
	}

	Location p(x, 0, z);
	interpolate(*p1, *p2, *p3, p);
	return p.y;
}




void Level::generate(World& world, int seed, int post_passes, float& percentage_done)
{
    ++post_passes;
	randomer.setSeed(seed);

	for(size_t i = 0; i < pointheight_info.size(); ++i)
		for(size_t k = 0; k < pointheight_info[i].size(); ++k)
			updateHeight(i, k, FixedPoint(20));

    ifstream inFile("data/levels/level1.dat");
    string line;
    int lineCounter = 0;
    while(inFile >> line) {
        percentage_done = lineCounter / 129.0f;
        for(unsigned i=0; i<line.size(); ++i) {
            if(line[i] == 'x')
                updateHeight(i, lineCounter, FixedPoint(20));
            if(line[i] == '.')
                updateHeight(i, lineCounter, FixedPoint(0));
            if(line[i] == ',')
                updateHeight(i, lineCounter, FixedPoint(2));
            if(line[i] == ';')
                updateHeight(i, lineCounter, FixedPoint(4));
            if(line[i] == ':')
                updateHeight(i, lineCounter, FixedPoint(6));
            if(line[i] == 'L')
            {
                updateHeight(i, lineCounter, FixedPoint(0));
                Location pos = Location(8*i, 15, 8*lineCounter);
                world.visualworld->insertLevelLight(pos);
            }
            if(line[i] == 's')
            {
                updateHeight(i, lineCounter, FixedPoint(0));
                startPosition.x = 8 * i;
                startPosition.z = 8 * lineCounter;
                startPosition.y = 4;
            }
            if(level_objects.addObject(line[i], 8*i, 8*lineCounter)) {
                updateHeight(i, lineCounter, FixedPoint(0));
            }
        }

        ++lineCounter;
    }
}



FixedPoint Level::max_x() const
{
	return pointheight_info.size() * BLOCK_SIZE - (BLOCK_SIZE+1);
}

FixedPoint Level::max_z() const
{
	return pointheight_info.front().size() * BLOCK_SIZE - (BLOCK_SIZE+1);
}

int Level::max_block_x() const
{
	return pointheight_info.size();
}

int Level::max_block_z() const
{
	return pointheight_info.front().size();
}

