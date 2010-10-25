#include "../fixed_point.h"
#include "../location.h"
#include "test.h"

void interpolate(const Location& A, const Location& B, const Location& C, Location& p);

int main()
{
	{
		Location p1(0,0,0);
		Location p2(10,0,0);
		Location p3(0,0,10);

		Location p(10,0,0);

		interpolate(p1, p2, p3, p);

		assertEqual(p, Location(10,0,0));
	}
	{
		Location p1(0,0,0);
		Location p2(10,0,0);
		Location p3(0,0,10);

		Location p(1,0,1);

		interpolate(p1, p2, p3, p);

		assertEqual(p, Location(1,0,1));
	}
	{
		Location p1(0,2,0);
		Location p2(2,0,0);
		Location p3(0,0,2);

		Location p(1,0,0);

		interpolate(p1, p2, p3, p);

		assertEqual(p, Location(1,1,0));
	}
	{
		Location p1(0,2,0);
		Location p2(2,0,0);
		Location p3(0,0,2);

		Location p(0,0,1);

		interpolate(p1, p2, p3, p);

		assertEqual(p, Location(0,1,1));
	}
	{
		Location p1(0,2,0);
		Location p2(2,0,0);
		Location p3(0,0,2);

		Location p(0,0,1);

		interpolate(p1, p2, p3, p);

		assertEqual(p, Location(0,1,1));
	}
	{
		Location p1(0,2,0);
		Location p2(2,0,0);
		Location p3(0,0,2);

		Location p(FixedPoint(1,2),0,FixedPoint(1,2));

		interpolate(p1, p2, p3, p);

		assertEqual(p, Location(FixedPoint(1,2),1,FixedPoint(1,2)));
	}
}

