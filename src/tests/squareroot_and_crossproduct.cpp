
#include "../fixed_point.h"
#include "../location.h"
#include <iostream>

using namespace std;

int main()
{
	
	cerr << "square root of 2 = " << FixedPoint(2).squareRoot() << endl;
	cerr << "square root of 3 = " << FixedPoint(3).squareRoot() << endl;
	cerr << "square root of 4 = " << FixedPoint(4).squareRoot() << endl;
	
	Location a;
	a.x = 0;
	a.y = 1;
	a.z = 2;
	
	Location b;
	b.x = 1;
	b.y = 5;
	b.z = 9;
	
	Location cp = a.crossProduct(b);
	cerr << "cross product = (" << cp.x.getFloat() << ", " << cp.y.getFloat() << ", " << cp.z.getFloat() << ")" << endl;
	cp = b.crossProduct(a);
	cerr << "cross product = (" << cp.x.getFloat() << ", " << cp.y.getFloat() << ", " << cp.z.getFloat() << ")" << endl;
	cerr << "dot product = " << a.dotProduct(b).getFloat() << endl;
	
	return 1;
}

