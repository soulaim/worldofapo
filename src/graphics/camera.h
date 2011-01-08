
#ifndef CAMERA_H
#define CAMERA_H

#include "fixed_point.h"
#include "apomath.h"
#include "location.h"
#include "frustum/vec3.h"
#include "frustum/matrix4.h"
#include "level.h"
#include "unit.h"

#include <iostream>

// NOTE: This could be just implemented as a movable object. Would be a bit more simple.
class Camera
{
public:
	enum FollowMode
	{
		STATIC, // Camera doesn't care about unit's direction.
		THIRD_PERSON, // Camera rotates if unit rotates.
		FIRST_PERSON // Camera moves at unit's location
	};
	
	Camera();
	const Vec3& getTarget() const;
	Vec3 getPosition() const;
	const Location& getUnitLocation() const;
	
	void setPosition(const Vec3& position);

	bool unitDie(int id);
	
	void tick();

	float getXrot() const;
	float getYrot() const;
	
	void bind(Unit* unit, FollowMode mode);
	void setMode(FollowMode mode);
	FollowMode mode() const;

	void updateInput(int keystate);
	void fixHeight(float h);
	void zoomIn();
	void zoomOut();
	void setAboveGround(float min_cam_y);
	
	float fov, min_fov, max_fov;
	float aspect_ratio;
	float nearP;
	float farP;
	
	int unit_id;
	
	Matrix4 modelview() const;
	Matrix4 perspective() const;
	Unit* getUnitPointer() const;

private:
	void getRelativePos(Vec3&) const;
	
	Vec3 position;
	Vec3 fps_direction;
	
	// Lagging dudes
	Vec3 currentPosition;
	Vec3 currentRelative;
	Vec3 currentTarget;
	
	float cur_sin;
	float cur_cos;
	float cur_upsin;
	float cur_upcos;
	
	Unit* unit;
	FollowMode mode_;
	
	void fpsTick();
	void relativeTick();
	void staticTick();
};

#endif

