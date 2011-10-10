#ifndef LEVEL_OBJECT_H
#define	LEVEL_OBJECT_H

#include <string>

class LevelObject
{
public:
    int coord_x;
    int coord_z;
    std::string object_name;

    LevelObject(int, int, const std::string& str);
    void setCoordinates(int x, int z);
};

#endif	/* LEVEL_OBJECT_H */