

#include "level.h"
#include <cstdlib>
#include <iostream>

using namespace std;

// level size should be ((2^n) + 1) because binary triangle trees work best then.
#define LEVEL_LVLSIZE 129

void interpolate(const Location& A, const Location& B, const Location& C, Location& p)
{
	Location direction1 = B - A;
	Location direction2 = C - A;

	// A + t*(direction1) + u*(direction2) = p;
	//
	// or in other words:
	//
	// t*(direction11.x) + u*(direction2.x) = p.x - A.x
	// t*(direction11.z) + u*(direction2.z) = p.z - A.z
	//
	// Solution for t and u is:
	FixedPoint t = ( direction2.x * (p.z - A.z) - direction2.z * (p.x - A.x) ) / ( direction2.x * direction1.z - direction1.x * direction2.z  );
	FixedPoint u = ( direction1.x * (p.z - A.z) - direction1.z * (p.x - A.x) ) / ( direction1.x * direction2.z - direction2.x * direction1.z  );

	p = A + direction1 * t + direction2 * u;
}


Level::Level(): btt(LEVEL_LVLSIZE-1, LEVEL_LVLSIZE-1)
{
	fpZero = FixedPoint(0);
	
	unitVectorUp.x = FixedPoint(0);
	unitVectorUp.y = FixedPoint(1);
	unitVectorUp.z = FixedPoint(0);
	
	
	pointheight_info.resize(LEVEL_LVLSIZE);
	for(size_t i = 0; i < pointheight_info.size(); ++i)
	{
		pointheight_info[i].resize(LEVEL_LVLSIZE);
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


void Level::buildVarianceTree()
{
	variance_tree.resize(2048, FixedPoint(0));
	FixedPoint max_error = btt.buildVarianceTree(pointheight_info, variance_tree);
	cerr << "Maximum encountered variance error: " << max_error << endl;
	
	for(int i=1; i<256; i++)
	{
//		cerr << variance_tree[i] << endl;
	}
	
	btt.doSplit(pointheight_info, variance_tree);
	btt.draw(5, 5);
	
	vector<BTT_Triangle> tris;
	
	btt.getTriangles(tris);
	cerr << "There is " << tris.size() << " triangles in BTT" << endl;
}




Location Level::getRandomLocation(int seed)
{
	Location result;
	result.x = ((173  * seed) % LEVEL_LVLSIZE) * 8;
	result.z = ((833 * seed) % LEVEL_LVLSIZE) * 8;
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
	
	normals[x][z] = estimateNormal(x, z) + estimateNormal(x-1, z) + estimateNormal(x+1, z) + estimateNormal(x, z-1) + estimateNormal(x, z+1); // + estimateNormal(x+1, z+1) + estimateNormal(x-1, z+1) + estimateNormal(x+1, z-1) + estimateNormal(x-1, z-1);
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
	
	result = (b-a).crossProduct(c-a);
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


const FixedPoint& Level::getVertexHeight(const int& x, const int& z) const
{
	return pointheight_info[x][z];
}

FixedPoint Level::getHeight(const FixedPoint& x, const FixedPoint& z) const
{
	int x_index = x.getInteger() / 8;
	int z_index = z.getInteger() / 8;

	if(x_index < 1 || x_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint(1);
	if(z_index < 1 || z_index > static_cast<int>(pointheight_info.size()) - 2)
		return FixedPoint(1);
	
	int x_desimal = ((x.getInteger() % 8) * FIXED_POINT_ONE + x.getDesimal()) / 8;
	int z_desimal = ((z.getInteger() % 8) * FIXED_POINT_ONE + z.getDesimal()) / 8;
	
	const FixedPoint& A = pointheight_info[x_index][z_index];
	const FixedPoint& B = pointheight_info[x_index+1][z_index];
	
	const FixedPoint& C = pointheight_info[x_index][z_index+1];
	const FixedPoint& D = pointheight_info[x_index+1][z_index+1];

	Location pA(8*x_index,     A, 8*z_index);
	Location pB(8*(x_index+1), B, 8*z_index);
	Location pC(8*x_index,     C, 8*(z_index+1));
	Location pD(8*(x_index+1), D, 8*(z_index+1));
	const Location* p1 = 0;
	const Location* p2 = 0;
	const Location* p3 = 0;
	
	if((z_index + x_index) % 2)
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
		if(FIXED_POINT_ONE - x_desimal < z_desimal)
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




void Level::generate(int seed)
{
	srand(seed);
	
	for(size_t i = 0; i < pointheight_info.size(); ++i)
		for(size_t k = 0; k < pointheight_info[i].size(); ++k)
			updateHeight(i, k, FixedPoint(0));
	
	
	// create long walls
	for(int i=0; i<350; i++)
	{
		
		int x_p = rand() % pointheight_info.size();
		int y_p = rand() % pointheight_info[x_p].size();
		FixedPoint height = FixedPoint(15);
		
		for(int k=0; k<15; k++)
		{
			height += FixedPoint(1);
			updateHeight(x_p, y_p, height);
			
			int x_size = pointheight_info.size() - 2;
			int y_size = pointheight_info[0].size() - 2;
			
			if(x_p > 0 && x_p < x_size)
			if(y_p > 0 && y_p < y_size)
			{
				updateHeight(x_p-1, y_p, (height + getHeight(x_p-1, y_p)) / FixedPoint(2));
				updateHeight(x_p+1, y_p, (height + getHeight(x_p+1, y_p)) / FixedPoint(2));
				updateHeight(x_p, y_p+1, (height + getHeight(x_p, y_p+1)) / FixedPoint(2));
				updateHeight(x_p, y_p-1, (height + getHeight(x_p, y_p-1)) / FixedPoint(2));
			}
			
			x_p += (rand() % 3) - 1;
			y_p += (rand() % 3) - 1;
		}
	}
	
	// create some valleys
	for(int i=0; i<150; i++)
	{
		int x_p = rand() % pointheight_info.size();
		int y_p = rand() % pointheight_info[x_p].size();
		FixedPoint height = FixedPoint(-2);
		
		for(int k=0; k<20; k++)
		{
			height -= FixedPoint(1, 2);
			
			updateHeight(x_p, y_p, height);
			
			int x_size = pointheight_info.size() - 2;
			int y_size = pointheight_info[0].size() - 2;
			
			if(x_p > 0 && x_p < x_size)
			if(y_p > 0 && y_p < y_size)
			{
				updateHeight(x_p-1, y_p, (height + getHeight(x_p-1, y_p)) / FixedPoint(2));
				updateHeight(x_p+1, y_p, (height + getHeight(x_p+1, y_p)) / FixedPoint(2));
				updateHeight(x_p, y_p+1, (height + getHeight(x_p, y_p+1)) / FixedPoint(2));
				updateHeight(x_p, y_p-1, (height + getHeight(x_p, y_p-1)) / FixedPoint(2));
			}
			
			x_p += (rand() % 3) - 1;
			y_p += (rand() % 3) - 1;
		}
	}
	
	
	// create some semihigh-ground
	for(int i=0; i<150; i++)
	{
		
		int x_p = rand() % pointheight_info.size();
		int y_p = rand() % pointheight_info[x_p].size();
		FixedPoint height = FixedPoint(2);
		
		for(int k=0; k<20; k++)
		{
			height += FixedPoint(2, 5);
			
			updateHeight(x_p, y_p, height);
			
			int x_size = pointheight_info.size() - 2;
			int y_size = pointheight_info[0].size() - 2;
			
			if(x_p > 0 && x_p < x_size)
				if(y_p > 0 && y_p < y_size)
				{
					updateHeight(x_p-1, y_p, (height + getHeight(x_p-1, y_p)) / FixedPoint(2));
					updateHeight(x_p+1, y_p, (height + getHeight(x_p+1, y_p)) / FixedPoint(2));
					updateHeight(x_p, y_p+1, (height + getHeight(x_p, y_p+1)) / FixedPoint(2));
					updateHeight(x_p, y_p-1, (height + getHeight(x_p, y_p-1)) / FixedPoint(2));
				}
				
				x_p += (rand() % 3) - 1;
			y_p += (rand() % 3) - 1;
		}
	}
	
	// after level has been fully defined, build the corresponding variance tree.
	buildVarianceTree();
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
	return pointheight_info.size() * 8 - 9;
}

int Level::max_z() const
{
	return pointheight_info.front().size() * 8 - 9;
}

