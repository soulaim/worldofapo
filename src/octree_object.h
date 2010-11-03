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
			type(UNDEFINED)
			{
			};

		int type;

		virtual Location bb_top() const = 0;
		virtual Location bb_bot() const = 0;
		virtual void collides(OctreeObject&) = 0;
};


#endif
