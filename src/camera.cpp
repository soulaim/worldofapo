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
		// TODO: Fix to use some common ApoMath.
		static ApoMath dorka;
		if(!dorka.ready())
			dorka.init(3000);
		
		double cos = dorka.getCos(unit->angle).getFloat();
		double sin = dorka.getSin(unit->angle).getFloat();

		double upsin = dorka.getSin(unit->upangle).getFloat();
		double upcos = dorka.getCos(unit->upangle).getFloat();

		double x = position.x;
		double y = position.y;
		double z = position.z;

		Vec3 relative_position;
		relative_position.x = cos * upcos * x - sin * z + cos * upsin * y;
		relative_position.z = sin * upcos * x + cos * z + sin * upsin * y;
		relative_position.y =      -upsin * x + 0.0 * z +       upcos * y;

		if (mode == RELATIVE && relative_position.y < -head_level/2) {
			relative_position.y = -head_level/2;
		}

		Vec3 camTarget;
		if(unit)
		{
			camTarget.x = unit->position.x.getFloat();
			camTarget.y = unit->position.y.getFloat() + head_level;
			camTarget.z = unit->position.z.getFloat();
		}
		else
		{
			camTarget.x = position.x - 50.0;
			camTarget.y = position.y;
			camTarget.z = position.z;
		}
		
		float multiplier = 0.04;
		if(mode == FIRST_PERSON)
			multiplier = 0.2;
		
		currentRelative += (relative_position - currentRelative) * multiplier;
		currentPosition += (camTarget - currentPosition) * multiplier;
		currentTarget   += (camTarget - currentTarget) * multiplier;
		
		fps_direction = currentPosition;
		fps_direction.x -= relative_position.x;
		fps_direction.y -= relative_position.y;
		fps_direction.z -= relative_position.z;
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

void Camera::updateInput(int keystate, int mousex, int mousey)
{
	if(keystate & 1 << 11)
	{
		position = Vec3();
	}
	
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


float Camera::getXrot()
{
	if(unit)
	{
		return unit->angle;
	}
	
	return 0.f;
}

float Camera::getYrot()
{
	if(unit)
	{
		return unit->upangle;
	}
	
	return 0.f;
}
