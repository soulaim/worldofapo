#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <string>

#include "fixed_point.h"
#include "location.h"
#include "btt.h"

struct Level
{
	Level();
	
	friend class Graphics;
	
	std::vector<std::vector<FixedPoint> > pointheight_info;
	
	void generate(int);
	FixedPoint getHeight(const FixedPoint& x, const FixedPoint& z) const;
	const FixedPoint& getVertexHeight(const int& x, const int& z) const;
	
	void splitBTT(const Location& pos, const FrustumR& frustum);
	
	// random position from the map. (spawn)
	Location getRandomLocation(int);
	
	// wtf physics ffs this is bullshit..
	FixedPoint getJumpPower(FixedPoint& x, FixedPoint& z);

	// getter functions for precalculated data.
	const Location& getNormal(FixedPoint& x, FixedPoint& z) const;
	const Location& getNormal(int x, int z) const;
	const FixedPoint& getHeightDifference(FixedPoint& x, FixedPoint& z) const;
	const FixedPoint& getHeightDifference(int x_index, int z_index) const;
	
	// Dynamic environment changes can be done with this function
	void updateHeight(int x, int z, FixedPoint h);
	
	int max_x() const;
	int max_z() const;

private:
	
	BinaryTriangleTree btt;
	std::vector<FixedPoint> variance_tree;
	
	FixedPoint fpZero;
	
	// dont use these!! ffs!
	std::vector<std::vector<FixedPoint> > h_diff;
	std::vector<std::vector<Location> > normals;
	
	FixedPoint estimateHeightDifference(int x, int z) const;
	void updateHeightDifference(int x, int z);
	void updateNormal(int x, int z);
	
	void buildVarianceTree();
	
	Location unitVectorUp;
	Location estimateNormal(int x, int z);
};

#endif

