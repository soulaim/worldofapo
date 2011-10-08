#include "level_object.h"
//#include "misc/idgenerator.h"
#include "graphics/visualworld.h"

using namespace std;

LevelObject::LevelObject(int type, int x, int z)
{
    model_type = type;
    coord_x = x;
    coord_z = z;
}

void LevelObject::setModelType(int type)
{

    model_type = type;
}
void LevelObject::setCoordinates(int x, int z)
{
    coord_x = x;
    coord_z = z;
}