#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <string>

#include "fixed_point.h"
#include "location.h"

struct Level
{
	Level();
	
	std::vector<std::vector<FixedPoint> > pointheight_info;
	std::vector<std::vector<Location> > normals;
	
	void generate(int);
	FixedPoint getHeight(const FixedPoint& x, const FixedPoint& y) const;
	Location getRandomLocation(int);
	
	FixedPoint getJumpPower(FixedPoint& x, FixedPoint& z);
	float estimateHeightDifference(int x, int y) const;

	int max_x() const;
	int max_z() const;

private:
	// dont use these!! ffs!
	Location unitVectorUp;
	Location estimateNormal(int x, int y);
	void firstPassNormals();
};

#endif

