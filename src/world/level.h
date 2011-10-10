#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <string>

#include "misc/fixed_point.h"
#include "misc/vec3.h"
#include "world/random.h"
#include "world/level_objects.h"

class MovableObject;

struct Level
{
	Level();

	friend class LevelDescriptor;
	friend class BinaryTriangleTree; // TODO: is this really necessary?

	void generate(int, int, float&);

	// random position from the map. (spawn)
	Location getRandomLocation(int) const;
    const Location& getStartLocation() const;

	// getter functions for precalculated data.
	const Location& getNormal(FixedPoint& x, FixedPoint& z) const;
	const Location& getNormal(int x, int z) const;
	const FixedPoint& getHeightDifference(FixedPoint& x, FixedPoint& z) const;
	const FixedPoint& getHeightDifference(int x_index, int z_index) const;

	// Dynamic environment changes can be done with this function
	void updateHeight(int x, int z, FixedPoint h);

    void clampToLevelArea(MovableObject&);

	FixedPoint max_x() const;
	FixedPoint max_z() const;
	FixedPoint getHeight(const FixedPoint& x, const FixedPoint& z) const;

	int max_block_x() const;
	int max_block_z() const;
	const FixedPoint& getVertexHeight(int x, int z) const;
        LevelObjects level_objects;
	enum { BLOCK_SIZE = 8 };

private:
	std::vector<std::vector<FixedPoint> > pointheight_info;

	// dont use these!! ffs!
	std::vector<std::vector<FixedPoint> > h_diff;
	std::vector<std::vector<Location> > normals;

	FixedPoint estimateHeightDifference(int x, int z) const;
	void updateHeightDifference(int x, int z);
	void updateNormal(int x, int z);

    Location startPosition;
	Location unitVectorUp;
	RandomMachine randomer;

	Location estimateNormal(int x, int z);
};

#endif

