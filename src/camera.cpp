#include "camera.h"

const double head_level = 6.0;

Camera::Camera():
	position(-30.0, 0.0, 0.0),
	unit(0),
	mode(RELATIVE)
{
}

Vec3& Camera::getCurrentTarget()
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

Location Camera::getUnitPosition() const
{
	if(unit)
	{
		return unit->position;
	}
	return Location();
}

Vec3 Camera::getPosition() const
{
	if(unit)
	{
		if(mode == RELATIVE)
		{
			return currentPosition + currentRelative;
		}
		
		if(mode == FIRST_PERSON)
		{
			return currentPosition;
		}
		
		return currentPosition;
	}
	
	return position;
}

void Camera::tick()
{
	if(unit)
	{
		if (mode == RELATIVE)
			relativeTick();
		else if (mode == FIRST_PERSON)
			fpsTick();
	}
}

void Camera::bind(Unit* unit, FollowMode mode)
{
	this->unit = unit;
	this->mode = mode;
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
	// f11? dont fuck up everything permanently, when the user makes the mistake of trying this button out..
	/*
	if(keystate & 1 << 11)
	{
		position = Vec3();
	}
	*/
	
	if(keystate & 1 << 12)
	{
		position.x += 1;
	}
	if(keystate & 1 << 13)
	{
		position.x -= 1;
	}
	if(keystate & 1 << 14)
	{
		position.y += 1;
	}
	if(keystate & 1 << 15)
	{
		position.y -= 1;
	}
	if(keystate & 1 << 16)
	{
		position.z += 1;
	}
	if(keystate & 1 << 17)
	{
		position.z -= 1;
	}
	if(keystate & 1 << 18)
	{
		setMode(Camera::RELATIVE);
	}
	if(keystate & 1 << 19)
	{
		setMode(Camera::FIRST_PERSON);
	}
}

void Camera::fpsTick()
{
	ApoMath math;
	
	double cos = math.getCos(unit->angle).getFloat();
	double sin = math.getSin(unit->angle).getFloat();

	double upsin = math.getSin(unit->upangle).getFloat();
	double upcos = math.getCos(unit->upangle).getFloat();

	double x = position.x;
	double y = position.y;
	double z = position.z;

	Vec3 relative_position;
	relative_position.x = cos * upcos * x - sin * z + cos * upsin * y;
	relative_position.z = sin * upcos * x + cos * z + sin * upsin * y;
	relative_position.y =      -upsin * x + 0.0 * z +       upcos * y;

	Vec3 camTarget;
	camTarget.x = unit->position.x.getFloat();
	camTarget.y = unit->position.y.getFloat() + head_level;
	camTarget.z = unit->position.z.getFloat();
	
	currentPosition += (camTarget - currentPosition) * 0.2;
	
	fps_direction = currentPosition;
	fps_direction.x -= relative_position.x;
	fps_direction.y -= relative_position.y;
	fps_direction.z -= relative_position.z;
}

void Camera::relativeTick()
{
	ApoMath math;
	
	double cos = math.getCos(unit->angle).getFloat();
	double sin = math.getSin(unit->angle).getFloat();

	double upsin = math.getSin(unit->upangle).getFloat();
	double upcos = math.getCos(unit->upangle).getFloat();

	double x = position.x;
	double y = position.y;
	double z = position.z;

	Vec3 relative_position;
	relative_position.x = cos * upcos * x - sin * z + cos * upsin * y;
	relative_position.z = sin * upcos * x + cos * z + sin * upsin * y;
	relative_position.y =      -upsin * x + 0.0 * z +       upcos * y;

	Vec3 camTarget;
	camTarget.x = unit->position.x.getFloat();
	camTarget.y = unit->position.y.getFloat() + head_level;
	camTarget.z = unit->position.z.getFloat();
	
	float multiplier = 0.04;
	
	currentRelative += (relative_position - currentRelative) * multiplier;
	currentPosition += (camTarget - currentPosition) * multiplier;
	currentTarget   += (camTarget - currentTarget) * multiplier;
}

void Camera::zoomIn()
{
	if(mode == RELATIVE)
	{
		if(position.length() > 1.0)
		{
			position *= 2.0/3.0;
		}
	}
}

void Camera::zoomOut()
{
	if(mode == RELATIVE)
	{
		if(position.length() < 100.0)
		{
			position *= 3.0/2.0;
		}
	}
}

float Camera::getXrot()
{
	if(unit)
	{
		return ApoMath().getDegrees(unit->angle);
	}
	
	return 0.f;
}

float Camera::getYrot()
{
	if(unit)
	{
		return ApoMath().getDegrees(unit->upangle);
	}
	
	return 0.f;
}

