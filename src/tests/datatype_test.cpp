#include <cassert>
#include <iostream>
#include <cmath>

using namespace std;

int main()
{
	static_assert(sizeof(int) == 4, "sizeof(int) == 4");
	static_assert(sizeof(long) == 8, "sizeof(long) == 8");
	static_assert(sizeof(long long) == 8, "sizeof(long long) == 8");
}


