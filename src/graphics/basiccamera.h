#ifndef BASICCAMERA_H
#define BASICCAMERA_H

#include "misc/vec3.h"
#include "graphics/frustum/matrix4.h"

class BasicCamera
{
public:
	BasicCamera();
	virtual ~BasicCamera();

	void zoomIn();
	void zoomOut();
	void zoomDefault();

	void change_yaw(float dx);
	void change_pitch(float dy);

	float fov;
	float aspect_ratio;
	static const float min_fov;
	static const float max_fov;
	static const float nearP;
	static const float farP;

	void setTarget(const vec3<float>& target);
	virtual void setPosition(const vec3<float>& position);

	virtual vec3<float> getTarget() const;
	virtual vec3<float> getPosition() const;

	Matrix4 modelview() const;
	Matrix4 perspective() const;

protected:
	vec3<float> currentPosition;
	
private:
	float static_angle;
	float static_upangle;
};

#endif

