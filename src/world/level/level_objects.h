#ifndef LEVEL_OBJECTS_H
#define	LEVEL_OBJECTS_H
#include <vector>
#include <cstdlib>
#include "world/level/level_object.h"

class LevelObjects
{
public:
    std::vector<LevelObject> objects;

    LevelObjects();
    bool addObject(char levelFormatMark, int x, int z);
    std::vector<LevelObject> getObjects();
};

#endif	/* LEVEL_OBJECTS_H */