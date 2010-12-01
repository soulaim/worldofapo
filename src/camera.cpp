#include "camera.h"
#include "unit.h"
#include "frustum/matrix4.h"

const double head_level = 4.0;

Camera::Camera():
	position(-30.0, 0.0, 0.0),
	unit(0),
	mode(RELATIVE)
{
	cur_sin = 0.f;
	cur_cos = 0.f;
	cur_upsin = 0.f;
	cur_upcos = 0.f;
	
	fov = 100.f;
	aspect_ratio = 800.f / 600.f;
	nearP = 1.f;
	farP  = 200.f;
	
	min_fov = 15.f;
	max_fov = 100.f;
}

Vec3& Camera::getTarget()
{
	if(mode == FIRST_PERSON)
		return fps_direction;
	return currentTarget;
}

void Camera::setAboveGround(float min_cam_y)
{
	if(mode == RELATIVE)
	{
		//std::cerr << min_cam_y << "\n";
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
	//if(unit)
	//{
		if(mode == RELATIVE)
		{
			return currentPosition + currentRelative;
		}
		else if(mode == FIRST_PERSON)
		{
			return currentPosition;
		}
		
		return currentPosition;
	//}
	
	//return position;
}

void Camera::tick()
{
	if(unit)
	{
		if(mode == RELATIVE)
		{
			relativeTick();
		}
		else if(mode == FIRST_PERSON)
		{
			fpsTick();
		}
		else
		{
			staticTick();
		}
	}
}

void Camera::bind(Unit* unit, FollowMode mode)
{
	this->unit = unit;
	this->mode = mode;
	this->unit_id = unit->id;
}

void Camera::setMode(FollowMode mode)
{
	this->mode = mode;
}

bool Camera::isFirstPerson() const
{
	return mode == FIRST_PERSON;
}

void Camera::updateInput(int keystate)
{
	if(mode == STATIC)
	{
		float wtf = position.length();
		
		Vec3 delta = (currentTarget - currentPosition);
		delta.normalize();
		
		Vec3 delta_sides = delta * Vec3(0.0f, 1.0f, 0.0f);
		delta_sides.normalize();
		
		delta *=  wtf / 30.0f;
		delta_sides *= wtf / 30.0f;
		
		if(keystate & 2)
		{
			currentPosition += delta_sides;
			currentTarget   += delta_sides;
		}
		if(keystate & 1)
		{
			currentPosition -= delta_sides;
			currentTarget   -= delta_sides;
		}
		if(keystate & 4)
		{
			currentPosition += delta;
			currentTarget   += delta;
		}
		if(keystate & 8)
		{
			currentPosition -= delta;
			currentTarget   -= delta;
		}
	}

	if(keystate & 1 << 18)
	{
		setMode(Camera::RELATIVE);
	}
	if(keystate & 1 << 19)
	{
		setMode(Camera::FIRST_PERSON);
	}
	if(keystate & 1 << 11)
	{
		Vec3 position = getPosition();
		setMode(Camera::STATIC);
		currentPosition = position;
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
	
	currentPosition += (camTarget - currentPosition) * 0.2;
	
	fps_direction = currentPosition;
	fps_direction.x -= relative_position.x;
	fps_direction.y -= relative_position.y;
	fps_direction.z -= relative_position.z;
}

void Camera::staticTick()
{
	ApoMath math;
	double angle1 = math.getDegrees(unit->angle);
	double angle2 = math.getDegrees(unit->upangle) + 90;
	Matrix4 rotation1(0, angle1, 0, 0,0,0);
	Matrix4 rotation2(0, 0, angle2, 0,0,0);
	
	// Vec3 relative_position;
	// getRelativePos(relative_position);
	
	currentTarget = currentPosition + rotation1 * rotation2 * position;
}

void Camera::getRelativePos(Vec3& result)
{
	double x = position.x;
	double y = position.y;
	double z = position.z;
	
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
	
	float multiplier = 0.04;
	
	currentRelative += (relative_position - currentRelative) * multiplier;
	currentPosition += (camTarget - currentPosition) * multiplier;
	currentTarget   += (camTarget - currentTarget) * multiplier;
}

void Camera::zoomIn()
{
	if(mode == RELATIVE || mode == STATIC)
	{
		if(position.length() > 1.0)
		{
			position *= 2.0/3.0;
		}
	}
	else if(mode == FIRST_PERSON)
	{
		fov /= 1.2;
		if(fov < min_fov)
			fov = min_fov;
	}
}

void Camera::zoomOut()
{
	if(mode == RELATIVE || mode == STATIC)
	{
		if(position.length() < 100.0)
		{
			position *= 3.0/2.0;
		}
	}
	else if(mode == FIRST_PERSON)
	{
		fov *= 1.2;
		if(fov > max_fov)
			fov = max_fov;
	}
}

float Camera::getXrot()
{
	static float x_rot = 0.f;
	if(unit)
		x_rot = ApoMath().getDegrees(unit->angle);
	return x_rot;
}

float Camera::getYrot()
{
	static float y_rot = 0.f;
	if(unit)
		y_rot = ApoMath().getDegrees(unit->upangle);
	return y_rot;
}

