
#include "../apomath.h"
#include <iostream>

using namespace std;

int main()
{
	ApoMath apomath;
	apomath.init(3000);
	
	for(int i=0; i<3000; i++)
	{
		int k=i;
		cerr << apomath.getSin(k) << " " << apomath.getCos(k) << " " << apomath.getDegrees(k) << "\n";
	}
	
	return 1;
}

