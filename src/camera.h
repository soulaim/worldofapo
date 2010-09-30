
#ifndef CAMERA_H
#define CAMERA_H

#include "fixed_point.h"
#include "apomath.h"
#include "location.h"

#include <iostream>

struct Camera
{
	Camera():
		yaw(0.0),
		pitch(0.0),
		roll(0.0),
		location(0)
	{
	}

	double getX() const
	{
		if(location)
			return x + location->x.getFloat();
		return x;
	}
	double getY() const
	{
		if(location)
			return y + location->h.getFloat();
		return y;
	}
	double getZ() const
	{
		if(location)
			return z + location->y.getFloat();
		return z;
	}

	// Maybe one these to a class called "Camera" or something...
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

	void bind(Location* loc)
	{
		location = loc;
	}

	double x;
	double y;
	double z;

	double yaw;
	double pitch;
	double roll;

private:
	Location* location;
};

#endif

