#ifndef CAMERA_H
#define CAMERA_H

#include "basiccamera.h"
#include "location.h"
#include "frustum/vec3.h"
#include "frustum/matrix4.h"

class Location;
class Unit;

class Camera: public BasicCamera
{
public:
	enum FollowMode
	{
		STATIC, // Camera acts like the BasicCamera.
		THIRD_PERSON, // Camera looks the bound unit.
		FIRST_PERSON // Camera is like the eyes of the bound unit.
	};
	
	Camera();

	virtual void setPosition(const Vec3& position);
	virtual Vec3 getTarget() const;
	virtual Vec3 getPosition() const;

	void tick();

	void bind(Unit* unit, FollowMode mode);
	void setMode(FollowMode mode);
	FollowMode mode() const;

	void updateInput(int keystate, int x, int y);
	void zoomIn();
	void zoomOut();
	void setAboveGround(float min_cam_y);

	const Location& getUnitLocation() const;
	float getXrot() const;
	float getYrot() const;

	bool unitDie(int id);
	Unit* getUnitPointer() const;
	int unit_id;
private:
	void getRelativePos(Vec3&) const;
	
	Vec3 default_direction;
	Vec3 fps_direction;
	
	// Lagging dudes
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

