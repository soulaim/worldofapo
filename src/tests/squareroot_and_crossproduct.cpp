
#include "../fixed_point.h"
#include "../location.h"
#include <iostream>

using namespace std;

int main()
{
	cerr << "square root of 2 = " << FixedPoint(2).squareRoot().getFloat() << endl;
	cerr << "square root of 3 = " << FixedPoint(3).squareRoot().getFloat() << endl;
	cerr << "square root of 4 = " << FixedPoint(4).squareRoot().getFloat() << endl;
	
	Location a;
	a.x = 0;
	a.h = 1;
	a.y = 2;
	
	Location b;
	b.x = 1;
	b.h = 5;
	b.y = 9;
	
	Location cp = a.crossProduct(b);
	cerr << "cross product = (" << cp.x.getFloat() << ", " << cp.h.getFloat() << ", " << cp.y.getFloat() << ")" << endl;
	cp = b.crossProduct(a);
	cerr << "cross product = (" << cp.x.getFloat() << ", " << cp.h.getFloat() << ", " << cp.y.getFloat() << ")" << endl;
	cerr << "dot product = " << a.dotProduct(b).getFloat() << endl;
	
	return 1;
}

