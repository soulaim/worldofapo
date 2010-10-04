
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
		RELATIVE, // Camera rotates if unit rotates.
		FIRST_PERSON // Camera moves at unit's location
	};
	
	Camera():
		position(-30.0, 0.0, 0.0),
		unit(0),
		mode(RELATIVE)
	{
	}
	
	Vec3 getPosition() const
	{
		if(unit)
		{
			
			// Vec3 relative_position = position;
			// Vec3 relative_position = currentPosition;
			
			if(mode == RELATIVE)
			{
				/*
				relative_position.x = cos * position.x - sin * position.z;
				relative_position.z = sin * position.x + cos * position.z;
				relative_position.y = position.y;
				*/
				
				/*
				relative_position.x = cos * currentRelative.x - sin * currentRelative.z;
				relative_position.z = sin * currentRelative.x + cos * currentRelative.z;
				relative_position.y = currentRelative.y;
				*/
				
				return currentPosition + currentRelative;
				
			}
			
			if(mode == FIRST_PERSON)
				return currentPosition;
			
			// return Vec3(getTargetX(), getTargetY(), getTargetZ()) + relative_position;
			return currentPosition;
		}
		
		return position;
	}
	
	void tick()
	{
		if(unit)
		{
			// TODO: Fix to use some common ApoMath.
			static ApoMath dorka;
			if(!dorka.ready())
				dorka.init(300);
			
			double cos = dorka.getCos(unit->angle).getFloat();
			double sin = dorka.getSin(unit->angle).getFloat();
			
			Vec3 relative_position;
			relative_position.x = cos * position.x - sin * position.z;
			relative_position.z = sin * position.x + cos * position.z;

			relative_position.y = position.y + unit->upangle/2; // TODO: this is dirty hack :)


			Vec3 camTarget;
			camTarget.x = getTargetX();
			camTarget.y = getTargetY();
			camTarget.z = getTargetZ();
			
			float multiplier = 0.04;
			if(mode == FIRST_PERSON)
				multiplier = 0.2;
			
			currentRelative += (relative_position - currentRelative) * multiplier;
			currentPosition += (Vec3(getTargetX(), getTargetY(), getTargetZ()) - currentPosition) * multiplier;
			currentTarget   += (camTarget - currentTarget) * multiplier;
			
			fps_direction = currentPosition;
			fps_direction.x -= relative_position.x;
			fps_direction.y  = relative_position.y;
			fps_direction.z -= relative_position.z;
		}
	}
	
	Vec3& getCurrentTarget()
	{
		if(mode == FIRST_PERSON)
			return fps_direction;
		return currentTarget;
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
	Vec3 fps_direction;
	
	// Lagging dudes
	Vec3 currentPosition;
	Vec3 currentRelative;
	Vec3 currentTarget;
	
	
	static const double head_level = 7.0;
	
	private:
		Unit* unit;
		FollowMode mode;
};

#endif

