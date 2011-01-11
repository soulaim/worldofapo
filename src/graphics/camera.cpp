#include "camera.h"
#include "unit.h"
#include "frustum/matrix4.h"
#include "apomath.h"
#include "algorithms.h"

const double head_level = 4.5;

Camera::Camera():
	BasicCamera(),
	default_direction(-30.0f, 0.0f, 0.0f),
	unit(0),
	mode_(THIRD_PERSON)
{
	cur_sin = 0.0f;
	cur_cos = 0.0f;
	cur_upsin = 0.0f;
	cur_upcos = 0.0f;
	
	fov = 100.f;
	aspect_ratio = 800.f / 600.f;
	nearP = 0.1f;
	farP  = 200.f;
	
	min_fov = 15.0f;
	max_fov = 100.0f;

	unit_id = -2;
}

Unit* Camera::getUnitPointer() const
{
	return unit;
}

Vec3 Camera::getTarget() const
{
	if(mode_ == STATIC)
	{
		return BasicCamera::getTarget();
	}
	else if(mode_ == FIRST_PERSON)
	{
		return fps_direction;
	}
	return currentTarget;
}

void Camera::setAboveGround(float min_cam_y)
{
	if(mode_ == THIRD_PERSON)
	{
		if(currentPosition.y + currentRelative.y < min_cam_y)
		{
			currentRelative.y = min_cam_y - currentPosition.y;
		}
	}
}

bool Camera::unitDie(int id)
{
	if(id == unit_id)
	{
		unit = 0;
		return true;
	}
	
	return false;
}

const Location& Camera::getUnitLocation() const
{
	static Location lastPos;
	if(unit)
		lastPos = unit->getPosition();
	return lastPos;
}

Vec3 Camera::getPosition() const
{
	if(mode_ == THIRD_PERSON)
	{
		return currentPosition + currentRelative;
	}
	else if(mode_ == FIRST_PERSON)
	{
		return currentPosition;
	}
	
	return BasicCamera::getPosition();
}

void Camera::setPosition(const Vec3& position)
{
	if(mode_ == THIRD_PERSON)
	{
		currentRelative = position;
	}
	else
	{
		BasicCamera::setPosition(position);
	}
}

void Camera::tick()
{
	if(unit)
	{
		if(mode_ == THIRD_PERSON)
		{
			relativeTick();
		}
		else if(mode_ == FIRST_PERSON)
		{
			fpsTick();
		}
	}
}

void Camera::bind(Unit* unit, FollowMode mode)
{
	this->mode_ = mode;
	this->unit = unit;
	this->unit_id = unit->id;
}

void Camera::setMode(FollowMode mode)
{
	this->mode_ = mode;
}

Camera::FollowMode Camera::mode() const
{
	return mode_;
}

void Camera::updateInput(int keystate, int x, int y)
{
	if(mode_ == STATIC)
	{
		Vec3 delta = (getTarget() - currentPosition);
		delta.normalize();
		
		Vec3 delta_sides = delta * Vec3(0.0f, 1.0f, 0.0f);
		delta_sides.normalize();
		
		float speed = default_direction.length() / 30.0f;
		delta *=  speed;
		delta_sides *= speed;
		
		if(keystate & 2)
		{
			currentPosition += delta_sides;
		}
		if(keystate & 1)
		{
			currentPosition -= delta_sides;
		}
		if(keystate & 4)
		{
			currentPosition += delta;
		}
		if(keystate & 8)
		{
			currentPosition -= delta;
		}

		change_yaw(-x / 10.0f);
		change_pitch(y / 10.0f);
	}
}

void Camera::fpsTick()
{
	ApoMath math;
	
	double cos = math.getCos(unit->angle).getFloat();
	double sin = math.getSin(unit->angle).getFloat();

	double upsin = math.getSin(unit->upangle).getFloat();
	double upcos = math.getCos(unit->upangle).getFloat();
	
	cur_sin += (sin - cur_sin) * 0.2f;
	cur_cos += (cos - cur_cos) * 0.2f;
	cur_upsin += (upsin - cur_upsin) * 0.2f;;
	cur_upcos += (upcos - cur_upcos) * 0.2f;
	
	Vec3 relative_position;
	getRelativePos(relative_position);

	Vec3 camTarget;
	const Location& unitPos = unit->getPosition();
	camTarget.x = unitPos.x.getFloat();
	camTarget.y = unitPos.y.getFloat() + head_level;
	camTarget.z = unitPos.z.getFloat();
	
	currentPosition += (camTarget - currentPosition) * 0.2f;
	
	fps_direction = currentPosition;
	fps_direction.x -= relative_position.x;
	fps_direction.y -= relative_position.y;
	fps_direction.z -= relative_position.z;
}

void Camera::getRelativePos(Vec3& result) const
{
	float x = default_direction.x;
	float y = default_direction.y;
	float z = default_direction.z;
	
	result.x = cur_cos * cur_upcos * x - cur_sin * z + cur_cos * cur_upsin * y;
	result.z = cur_sin * cur_upcos * x + cur_cos * z + cur_sin * cur_upsin * y;
	result.y =          -cur_upsin * x +     0.0 * z +           cur_upcos * y;
}

void Camera::relativeTick()
{
	ApoMath math;
	
	double cos = math.getCos(unit->angle).getFloat();
	double sin = math.getSin(unit->angle).getFloat();
	
	double upsin = math.getSin(unit->upangle).getFloat();
	double upcos = math.getCos(unit->upangle).getFloat();
	
	cur_sin += (sin - cur_sin) * 0.2f;
	cur_cos += (cos - cur_cos) * 0.2f;
	cur_upsin += (upsin - cur_upsin) * 0.2f;;
	cur_upcos += (upcos - cur_upcos) * 0.2f;

	Vec3 relative_position;
	getRelativePos(relative_position);
	
	Vec3 camTarget;
	const Location& unitPos = unit->getPosition();
	camTarget.x = unitPos.x.getFloat();
	camTarget.y = unitPos.y.getFloat() + head_level;
	camTarget.z = unitPos.z.getFloat();
	
	float multiplier = 0.04f;
	
	currentRelative += (relative_position - currentRelative) * multiplier;
	currentPosition += (camTarget - currentPosition) * multiplier;
	currentTarget   += (camTarget - currentTarget) * multiplier;
}

void Camera::zoomIn()
{
	if(mode_ == THIRD_PERSON || mode_ == STATIC)
	{
		if(default_direction.length() > 1.0f)
		{
			default_direction *= 2.0f/3.0f;
		}
	}
	else if(mode_ == FIRST_PERSON)
	{
		BasicCamera::zoomIn();
	}
}

void Camera::zoomOut()
{
	if(mode_ == THIRD_PERSON || mode_ == STATIC)
	{
		if(default_direction.length() < 100.0f)
		{
			default_direction *= 3.0f/2.0f;
		}
	}
	else if(mode_ == FIRST_PERSON)
	{
		BasicCamera::zoomOut();
	}
}

float Camera::getXrot() const
{
	if(mode_ == STATIC)
	{
		return getXangle(currentPosition - currentTarget);
	}

	static float x_rot = 0.f;
	if(unit)
	{
		x_rot = ApoMath().getDegrees(unit->angle);
	}
	return x_rot;
}

float Camera::getYrot() const
{
	if(mode_ == STATIC)
	{
		return getYangle(currentTarget - currentPosition);
	}

	static float y_rot = 0.f;
	if(unit)
	{
		y_rot = ApoMath().getDegrees(unit->upangle);
	}
	return y_rot;
}

