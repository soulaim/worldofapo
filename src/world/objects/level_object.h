#ifndef LEVEL_OBJECT_H
#define	LEVEL_OBJECT_H
class LevelObject
{
public:
    int model_type;
    int coord_x;
    int coord_z;
    LevelObject(int, int, int);
    void setModelType(int type);
    void setCoordinates(int x, int z);
};

#endif	/* LEVEL_OBJECT_H */