
#include "../btt.h"

int main()
{
	BinaryTriangleTree btt(128, 128);
	
	std::vector<BTT_Triangle> tris;
	
	for(int i=0; i<100000; i++)
	{
		int x = (i * 214763) & 127;
		int y = (i * 124573) & 127;
		
		btt.setViewPoint(x, y);
		btt.getTriangles(tris);
		tris.clear();
	}
}

