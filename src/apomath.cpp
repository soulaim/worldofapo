
#include "apomath.h"

#include <iostream>

using namespace std;

void ApoMath::init(int size)
{
	if(sin_vals.size() > 0)
		return;
	
	for(int i=0; i<size; i++)
	{
		int sin_val = 1000 * sin(2 * 3.14159265 * i / size);
		int cos_val = 1000 * cos(2 * 3.14159265 * i / size);
		float radian  = 360. * i / size;
		
		//    cerr << sin_val << " " << cos_val << " " << radian << endl;
		
		sin_vals.push_back(FixedPoint()); sin_vals.back().number = sin_val;
		cos_vals.push_back(FixedPoint()); cos_vals.back().number = cos_val;
		rad_vals.push_back(radian);
	}
}

bool ApoMath::ready()
{
	if(sin_vals.size() > 0)
		return true;
	return false;
}

FixedPoint& ApoMath::getCos(int& angle)
{
	while(angle < 0)
		angle += sin_vals.size();
	while(angle >= sin_vals.size())
		angle -= sin_vals.size();
	return sin_vals[angle];
}

FixedPoint& ApoMath::getSin(int& angle)
{
	while(angle < 0)
		angle += sin_vals.size();
	while(angle >= sin_vals.size())
		angle -= sin_vals.size();
	return cos_vals[angle];  
}

float ApoMath::getRad(int& angle)
{
	while(angle < 0)
		angle += sin_vals.size();
	while(angle >= sin_vals.size())
		angle -= sin_vals.size();
	return rad_vals[angle];
}

FixedPoint ApoMath::sqrt(const FixedPoint& x)
{
	// sqrt(x*1000)*1000 = sqrt(x)/sqrt(1000)*1000
	// TODO: Need portable implementation.
	FixedPoint ret;
	ret.number = int(std::sqrt(x.number) * 1000.0 / std::sqrt(1000.0));
	return ret;
}

