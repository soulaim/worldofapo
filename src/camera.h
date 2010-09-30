
#ifndef CAMERA_H
#define CAMERA_H

#include "fixed_point.h"
#include "apomath.h"
#include "location.h"
#include "unit.h"
#include "frustum/Vec3.h"

#include <iostream>

struct Camera
{
	enum FollowMode
	{
		STATIC, // Camera doesn't care about unit's direction.
		RELATIVE // Camera rotates if unit rotates.
	};

	Camera():
		position(-30.0, 0.0, 0.0),
		yaw(0.0),
		pitch(0.0),
		roll(0.0),
		unit(0),
		mode(STATIC)
	{
	}

	Vec3 getPosition() const
	{
		if(unit)
		{
			Vec3 relative_position = position;

			if(mode == RELATIVE)
			{
				// TODO: Fix to use some common ApoMath.
				ApoMath dorka;
				dorka.init(300);
				double cos = dorka.getCos(unit->angle).getFloat();
				double sin = dorka.getSin(unit->angle).getFloat();

				relative_position.x = cos * position.x - sin * position.z;
				relative_position.z = sin * position.x + cos * position.z;
				relative_position.y = position.y;
			}

			return Vec3(getTargetX(), getTargetY(), getTargetZ()) + relative_position;
		}
		return position;
	}


	double getTargetX() const
	{
		if(unit)
			return unit->position.x.getFloat();
		return position.x + -50.0;
	}

	double getTargetY() const
	{
		if(unit)
			return unit->position.h.getFloat() + head_level;
		return position.y;
	}

	double getTargetZ() const
	{
		if(unit)
			return unit->position.y.getFloat();
		return position.z;
	}

	void setYaw(double y)
	{
		yaw = y;
	}
	void setPitch(double y)
	{
		pitch = y;
	}
	void setRoll(double y)
	{
		roll = y;
	}

	// TODO: Yaw, pitch and roll axes are wrong?
	double getYaw() const
	{
		return yaw;
	}
	double getPitch() const
	{
		return pitch;
	}
	double getRoll() const
	{
		return roll;
	}

	void bind(Unit* unit, FollowMode mode)
	{
		this->unit = unit;
		this->mode = mode;
	}

	void setMode(FollowMode mode)
	{
		this->mode = mode;
	}

	Vec3 position;

	double yaw;
	double pitch;
	double roll;

	static const double head_level = 7.0;

private:
	Unit* unit;
	FollowMode mode;
};

#endif

