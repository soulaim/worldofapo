#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <string>

#include "fixed_point.h"

struct Level
{
	std::vector<std::vector<FixedPoint> > pointheight_info;
	//std::vector<std::vector<int> > walls_info;
	
	void generate(int);
	FixedPoint getHeight(const FixedPoint& x, const FixedPoint& y);
	
	float estimateHeightDifference(int x, int y);
};

#endif
