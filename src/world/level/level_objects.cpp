#include "level_objects.h"
#include "graphics/visualworld.h"
#include <cassert>
#include <iostream>

using namespace std;

LevelObjects::LevelObjects()
{
    assert(objects.empty());
}

//add support for more objects!!
bool LevelObjects::addObject(char levelFormatMark, int coordinate_x, int coordinate_y)
{
    if (levelFormatMark == 'B')
    {
        objects.push_back(LevelObject(coordinate_x, coordinate_y, "box"));
        return true;
    }

    if (levelFormatMark == 'S')
    {
        objects.push_back(LevelObject(coordinate_x, coordinate_y, "smallitem"));
        return true;
    }

    if (levelFormatMark == 'T')
    {
        objects.push_back(LevelObject(coordinate_x, coordinate_y, "item"));
        return true;
    }

    if (levelFormatMark == 'M')
    {
        objects.push_back(LevelObject(coordinate_x, coordinate_y, "monster"));
        return true;
    }

    return false;
}

std::vector<LevelObject> LevelObjects::getObjects()
{
    return objects;
}