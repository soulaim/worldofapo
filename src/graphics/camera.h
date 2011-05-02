#ifndef CAMERA_H
#define CAMERA_H

#include "graphics/basiccamera.h"
#include "graphics/frustum/matrix4.h"
#include "misc/vec3.h"

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

	virtual void setPosition(const vec3<float>& position);
	virtual vec3<float> getTarget() const;
	virtual vec3<float> getPosition() const;

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
	void getRelativePos(vec3<float>&) const;
	
	vec3<float> default_direction;
	vec3<float> fps_direction;
	
	// Lagging dudes
	vec3<float> currentRelative;
	vec3<float> currentTarget;
	
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

