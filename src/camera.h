
#ifndef CAMERA_H
#define CAMERA_H

#include "fixed_point.h"
#include "apomath.h"
#include "location.h"
#include "unit.h"
#include "frustum/Vec3.h"

#include <iostream>

class Camera
{
public:
	enum FollowMode
	{
		STATIC, // Camera doesn't care about unit's direction.
		RELATIVE, // Camera rotates if unit rotates.
		FIRST_PERSON // Camera moves at unit's location
	};


	Camera();
	Vec3& getCurrentTarget();
	Vec3 getPosition() const;
	void tick();

	void bind(Unit* unit, FollowMode mode);
	void setMode(FollowMode mode);
	bool isFirstPerson() const;
	void updateInput(int keystate, int mousex, int mousey);

private:
	Vec3 position;
	Vec3 fps_direction;

	// Lagging dudes
	Vec3 currentPosition;
	Vec3 currentRelative;
	Vec3 currentTarget;

	Unit* unit;
	FollowMode mode;
};

#endif

