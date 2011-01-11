#ifndef BASICCAMERA_H
#define BASICCAMERA_H

#include "frustum/vec3.h"
#include "frustum/matrix4.h"

class BasicCamera
{
public:
	BasicCamera();
	virtual ~BasicCamera();

	void zoomIn();
	void zoomOut();

	void change_yaw(float dx);
	void change_pitch(float dy);

	float fov, min_fov, max_fov;
	float aspect_ratio;
	float nearP;
	float farP;

	void setTarget(const Vec3& target);
	virtual void setPosition(const Vec3& position);

	virtual Vec3 getTarget() const;
	virtual Vec3 getPosition() const;

	Matrix4 modelview() const;
	Matrix4 perspective() const;

protected:
	Vec3 currentPosition;
private:
	float static_angle;
	float static_upangle;
};

#endif

