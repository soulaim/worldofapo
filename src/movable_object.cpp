
#include "movable_object.h"

MovableObject::MovableObject()
{
}

void MovableObject::movableEnable(int enable_flags)
{
	flags |= enable_flags;
}