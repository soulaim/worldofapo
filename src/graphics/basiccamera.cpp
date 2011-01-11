#include "basiccamera.h"
#include "algorithms.h"

#include <cmath>

using namespace std;

BasicCamera::BasicCamera():
	static_angle(0.0f),
	static_upangle(0.0f)
{
}

BasicCamera::~BasicCamera()
{
}

Vec3 BasicCamera::getPosition() const
{
	return currentPosition;
}

Vec3 BasicCamera::getTarget() const
{
	float angle1 = static_angle;
	float angle2 = static_upangle;
	Matrix4 rotation1(0, angle1, 0, 0,0,0);
	Matrix4 rotation2(0, 0, angle2, 0,0,0);
	
	return currentPosition + rotation1 * rotation2 * Vec3(-30.0f, 0.0f, 0.0f);
}

void BasicCamera::setPosition(const Vec3& position)
{
	currentPosition = position;
}

void BasicCamera::setTarget(const Vec3& target)
{
	Vec3 direction = target - currentPosition;
	static_angle = -90.0f + getXangle(direction);
	static_upangle = 90.0f - getYangle(direction);
}

void BasicCamera::change_yaw(float dx)
{
	static_angle += dx;
}

void BasicCamera::change_pitch(float dy)
{
	static_upangle += dy;
	if(static_upangle > 80.0f)
		static_upangle = 80.0f;
	if(static_upangle < -80.0f)
		static_upangle = -80.0f;
}

void BasicCamera::zoomIn()
{
	fov /= 1.2f;
	if(fov < min_fov)
	{
		fov = min_fov;
	}
}

void BasicCamera::zoomOut()
{
	fov *= 1.2f;
	if(fov > max_fov)
	{
		fov = max_fov;
	}
}

Matrix4 BasicCamera::modelview() const
{
	Vec3 position = getPosition();
	Vec3 target = getTarget();
	Vec3 up(0.0, 1.0, 0.0);

	Vec3 forward = target - position;
	forward.normalize();

	Vec3 side = forward * up;
	side.normalize();

	up = side * forward;

	Matrix4 m(side, up, -forward);
	Matrix4 t(0,0,0, -position.x, -position.y, -position.z);

	return m * t;
}

Matrix4 BasicCamera::perspective() const
{
	float ymax = nearP * tan(fov / 2.0 / 180.0 * 3.14159265);
	float xmax = ymax * aspect_ratio;

	return Matrix4::projectionFrustum(-xmax, xmax, -ymax, ymax, nearP, farP);
}

