
#include "apomath.h"

#include <iostream>
#include <fstream>

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
	if(!sin_vals.empty())
	{
		return;
	}
	
	cerr << "Creating a NEW APOMATH :DD" << endl;
	DEGREES_90 = size / 4;
	DEGREES_180 = size / 2;
	DEGREES_360 = size;
	
	ifstream infile("data/math.dat");
	
	if(infile)
	{
		cerr << "Reading math data from file..";
		
		int sinval, cosval;
		float degreeval;
		
		while(infile >> sinval >> cosval >> degreeval)
		{
			sin_vals.push_back(FixedPoint(sinval, 1000));
			cos_vals.push_back(FixedPoint(cosval, 1000));
			degree_vals.push_back(degreeval);
		}
		assert(sin_vals.size()    == unsigned(size));
		assert(cos_vals.size()    == unsigned(size));
		assert(degree_vals.size() == unsigned(size));
		cerr << " SUCCESS!" << endl;
	}
	else
	{
		cerr << "ERROR: math data file was not found!" << endl;
		exit(0);
		
		// this code would generate a new math file, but may give different values than original
		/*
		ofstream outfile("data/math.dat");
		
		for(int i=0; i<size; i++)
		{
			int sin_val = 1000 * sin(2 * 3.14159265f * i / size);
			int cos_val = 1000 * cos(2 * 3.14159265f * i / size);
			float degree = 360. * i / size;
			
			outfile << sin_val << " " << cos_val << " " << degree << endl;
			
			sin_vals.push_back(FixedPoint(sin_val, 1000));
			cos_vals.push_back(FixedPoint(cos_val, 1000));
			degree_vals.push_back(degree);
		}
		*/
	}
	
	
	cerr << "ApoMath initialized with size " << size << "\n";
}

void ApoMath::clamp(int& angle)
{
	while(angle < 0)
		angle += sin_vals.size();
	while(unsigned(angle) >= sin_vals.size())
		angle -= sin_vals.size();
}


FixedPoint& ApoMath::getCos(int& angle)
{
	clamp(angle);
	return sin_vals[angle];
}

FixedPoint& ApoMath::getSin(int& angle)
{
	clamp(angle);
	return cos_vals[angle];
}

float ApoMath::getDegrees(int& angle)
{
	clamp(angle);
	return degree_vals[angle];
}

FixedPoint ApoMath::sqrt(const FixedPoint& x)
{
	return x.squareRoot();
}

