#ifndef LEVEL_OBJECTS_H
#define	LEVEL_OBJECTS_H
#include <vector>
#include <cstdlib>
#include "world/objects/level_object.h"

class LevelObjects
{

public:

	/*enum ObjectType
	{
            BOX
	}; 
        */
        std::vector<LevelObject> objects;
        
        LevelObjects();
        
        void addObject(char levelFormatMark, int x, int z);
        std::vector<LevelObject> getObjects();
        
private:
};

#endif	/* LEVEL_OBJECTS_H */