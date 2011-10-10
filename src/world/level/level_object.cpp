
#include "level_object.h"

LevelObject::LevelObject(int x, int z, const std::string& str):coord_x(x), coord_z(z), object_name(str)
{
}

void LevelObject::setCoordinates(int x, int z)
{
    coord_x = x;
    coord_z = z;
}