
#include "../fixed_point.h"
#include "../location.h"
#include <iostream>

using namespace std;

FixedPoint pointInLinesegment(const Location& a, const Location& b, const Location& p);

int main()
{
	cerr << "square root of 2 = " << FixedPoint(2).squareRoot().getFloat() << endl;
	cerr << "square root of 3 = " << FixedPoint(3).squareRoot().getFloat() << endl;
	cerr << "square root of 4 = " << FixedPoint(4).squareRoot().getFloat() << endl;
	
	Location a;
	a.x = 2;
	a.z = 2;
	a.y = 2;
	
	Location b;
	b.x = 10;
	b.z = 10;
	b.y = 10;
	
	Location p;
	p.x = 5;
	p.z = 5;
	p.y = 5;
	
	cerr << "dot product = " <<  (a-p).dotProduct(b-p).getFloat() << endl;
	cerr << "MegaValue: " << pointInLinesegment(a, b, p).getFloat() << endl;
	p.x = -1;
	p.z = -1;
	p.y = -1;
	cerr << "dot product = " <<  (a-p).dotProduct(b-p).getFloat() << endl;
	cerr << "MegaValue: " << pointInLinesegment(a, b, p).getFloat() << endl;
	p.x = 0;
	p.z = 0;
	p.y = 0;
	cerr << "dot product = " <<  (a-p).dotProduct(b-p).getFloat() << endl;
	cerr << "MegaValue: " << pointInLinesegment(a, b, p).getFloat() << endl;
	p.x = 15;
	p.z = 15;
	p.y = 15;
	cerr << "dot product = " <<  (a-p).dotProduct(b-p).getFloat() << endl;
	cerr << "MegaValue: " << pointInLinesegment(a, b, p).getFloat() << endl;
	p.x = 11;
	p.z = 11;
	p.y = 11;
	cerr << "dot product = " <<  (a-p).dotProduct(b-p).getFloat() << endl;
	cerr << "MegaValue: " << pointInLinesegment(a, b, p).getFloat() << endl;
	p.x = 10;
	p.z = 0;
	p.y = 10;
	cerr << "dot product = " <<  (a-p).dotProduct(b-p).getFloat() << endl;
	cerr << "MegaValue: " << pointInLinesegment(a, b, p).getFloat() << endl;
	
	return 1;
}

