#ifndef H_OCTREE_OBJECT
#define H_OCTREE_OBJECT

#include "location.h"
#include "movable_object.h"

class OctreeObject : public MovableObject
{
	public:
		enum
		{
			UNDEFINED = 1,
			UNIT = 2,
			MEDIKIT = 4
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
		
		virtual Location bb_top() const = 0;
		virtual Location bb_bot() const = 0;
		virtual void collides(OctreeObject&) = 0;
};

struct OctreeObjectLess
{
	bool operator( )(const OctreeObject* o1, const OctreeObject* o2)
	{
		return(o1->id < o2->id);
	}
};

struct OctreeObjectPairLess
{
	bool operator( )(const std::pair<OctreeObject*, OctreeObject*> pair1, const std::pair<OctreeObject*, OctreeObject*> pair2)
	{
		FixedPoint a_min_top = min(pair1.first->bb_top().y, pair1.second->bb_top().y);
		FixedPoint b_min_top = min(pair2.first->bb_top().y, pair2.second->bb_top().y);
		
		return a_min_top < b_min_top;
		// TODO ALERT: Should handle the case where min_tops are equal (danger of de-sync).
	}
};


#endif
