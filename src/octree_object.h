#ifndef H_OCTREE_OBJECT
#define H_OCTREE_OBJECT

#include "location.h"
#include "movable_object.h"

class OctreeObject : public MovableObject
{
	public:
		enum {
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
		
		OctreeObject():
			type(UNDEFINED),
			id(-1),
			//collision_rule(CollisionRule::STRING_SYSTEM)
			collision_rule(CollisionRule::HARD_OBJECT)
			{
			};

		int type;
		int id;
		
		int collision_rule;
		
		
		virtual Location bb_top() const = 0;
		virtual Location bb_bot() const = 0;
		virtual void collides(OctreeObject&) = 0;
};

struct OctreeObjectLess {
	bool operator( )(const OctreeObject* o1, const OctreeObject* o2) {
		return(o1->id < o2->id);
	}
};


#endif
