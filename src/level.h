#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <string>

#include "fixed_point.h"
#include "location.h"

struct Level
{
	std::vector<std::vector<FixedPoint> > pointheight_info;
	std::vector<std::vector<Location> > normals;
	
	void generate(int);
	FixedPoint getHeight(const FixedPoint& x, const FixedPoint& y);
	
	float estimateHeightDifference(int x, int y);
	
private: 
	// dont use these!! ffs!
	Location estimateNormal(int x, int y);
	void firstPassNormals();
};

#endif
