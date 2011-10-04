#ifndef H_OCTREE_OBJECT
#define H_OCTREE_OBJECT

#include "misc/vec3.h"
#include "physics/movable_object.h"

#include <algorithm>

class OctreeObject : public MovableObject
{
	public:
		enum
		{
			UNDEFINED = 1,
			UNIT = 2,
			WORLD_ITEM = 4
		};

		enum CollisionRule
		{
			NO_COLLISION = 0,
			STRING_SYSTEM = 1,
			HARD_OBJECT = 2
		};

		OctreeObject();

		int type;
		int id;

		int collision_rule;
		int staticObject;

		Location posCorrection; // this does not need to be sent over network

		virtual const Location& bb_top() const = 0;
		virtual const Location& bb_bot() const = 0;
		virtual void collides(OctreeObject&) = 0;
};

struct OctreeObjectLess
{
	bool operator()(const OctreeObject* o1, const OctreeObject* o2) const
	{
		return o1->id < o2->id;
	}
};

struct OctreeObjectPairLess
{
	bool operator()(const std::pair<OctreeObject*, OctreeObject*> pair1, const std::pair<OctreeObject*, OctreeObject*> pair2) const
	{
#ifdef _WIN32
#undef min
#endif
		FixedPoint a_min_top = std::min(pair1.first->bb_top().y, pair1.second->bb_top().y);
		FixedPoint b_min_top = std::min(pair2.first->bb_top().y, pair2.second->bb_top().y);

		bool b1 = a_min_top < b_min_top;
		bool b2 = a_min_top == b_min_top && pair1.first->id < pair2.first->id;
		bool b3 = a_min_top == b_min_top && pair1.first->id == pair2.first->id && pair1.second->id < pair2.second->id;

		return b1 || b2 || b3;
	}
};

#endif

