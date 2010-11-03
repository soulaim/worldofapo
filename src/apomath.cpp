
#include "apomath.h"

#include <iostream>

using namespace std;


std::vector<FixedPoint> ApoMath::sin_vals;
std::vector<FixedPoint> ApoMath::cos_vals;
std::vector<float> ApoMath::degree_vals;

int ApoMath::DEGREES_90;
int ApoMath::DEGREES_180;
int ApoMath::DEGREES_360;

ApoMath::ApoMath()
{
	init(3000);
}

void ApoMath::init(int size)
{
	if(ready())
	{
		return;
	}

	cerr << "Creating a NEW APOMATH :DD" << endl;
	DEGREES_90 = size / 4;
	DEGREES_180 = size / 2;
	DEGREES_360 = size;

	for(int i=0; i<size; i++)
	{
		int sin_val = 1000 * sin(2 * 3.14159265 * i / size);
		int cos_val = 1000 * cos(2 * 3.14159265 * i / size);
		float degree = 360. * i / size;
		
		//cerr << sin_val << " " << cos_val << " " << degree << endl;
		
		sin_vals.push_back(FixedPoint(sin_val, 1000));
		cos_vals.push_back(FixedPoint(cos_val, 1000));
		degree_vals.push_back(degree);
	}

	cerr << "ApoMath initialized with size " << size << "\n";
}

void ApoMath::clamp(int& angle) const
{
	while(angle < 0)
		angle += sin_vals.size();
	while(unsigned(angle) >= sin_vals.size())
		angle -= sin_vals.size();
}

bool ApoMath::ready() const
{
	return !sin_vals.empty();
}

FixedPoint& ApoMath::getCos(int& angle) const
{
	clamp(angle);
	return sin_vals[angle];
}

FixedPoint& ApoMath::getSin(int& angle) const
{
	clamp(angle);
	return cos_vals[angle];
}

float ApoMath::getDegrees(int& angle) const
{
	clamp(angle);
	return degree_vals[angle];
}

FixedPoint ApoMath::sqrt(const FixedPoint& x)
{
	return x.squareRoot();
}

