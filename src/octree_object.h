#ifndef H_OCTREE_OBJECT
#define H_OCTREE_OBJECT

#include "location.h"
#include "movable_object.h"

class OctreeObject
{
	public:
		enum {
			UNDEFINED = 1,
			UNIT = 2,
			MEDIKIT = 4
		};

		OctreeObject():
			type(UNDEFINED),
			id(-1)
			{
			};

		int type;
		int id;

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
