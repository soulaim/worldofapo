#include "level_objects.h"
#include "graphics/visualworld.h"
#include <cassert>
using namespace std;

LevelObjects::LevelObjects()
{
    assert(objects.empty());
}

//add support for more objects!!
void LevelObjects::addObject(char levelFormatMark, int coordinate_x, int coordinate_y)
{
    if (levelFormatMark == 'B')
    {
        objects.push_back(LevelObject(VisualWorld::ModelType::BOX_MODEL, coordinate_x, coordinate_y));
    }
    
    if (levelFormatMark == 'T')
    {
        // you get the idea from line 15
    }
    
    
        
}

std::vector<LevelObject> LevelObjects::getObjects()
{
    return objects;
}