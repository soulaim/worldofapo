#include "basiccamera.h"
#include "algorithms.h"

#include <cmath>

using namespace std;

namespace
{
	const float default_fov = 100.0f;
	const float default_aspect_ratio = 800.0f / 600.0f;
}
const float BasicCamera::min_fov = 15.0f;
const float BasicCamera::max_fov = 100.0f;
const float BasicCamera::nearP = 0.1f;
const float BasicCamera::farP = 200.0f;

BasicCamera::BasicCamera():
	fov(default_fov),
	aspect_ratio(default_aspect_ratio),
	static_angle(0.0f),
	static_upangle(0.0f)
{
}

BasicCamera::~BasicCamera()
{
}

vec3<float> BasicCamera::getPosition() const
{
	return currentPosition;
}

vec3<float> BasicCamera::getTarget() const
{
	float angle1 = static_angle;
	float angle2 = static_upangle;
	Matrix4 rotation1(0.0f, angle1,   0.0f, 0.0f,0.0f,0.0f);
	Matrix4 rotation2(0.0f,   0.0f, angle2, 0.0f,0.0f,0.0f);
	
	return currentPosition + rotation1 * rotation2 * vec3<float>(-30.0f, 0.0f, 0.0f);
}

void BasicCamera::setPosition(const vec3<float>& position)
{
	currentPosition = position;
}

void BasicCamera::setTarget(const vec3<float>& target)
{
	vec3<float> direction = target - currentPosition;
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

void BasicCamera::zoomDefault()
{
	fov = default_fov;
}

Matrix4 BasicCamera::modelview() const
{
	vec3<float> position = getPosition();
	vec3<float> target = getTarget();
	vec3<float> up(0.0, 1.0, 0.0);

	vec3<float> forward = target - position;
	forward.normalize();

	vec3<float> side = forward * up;
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

