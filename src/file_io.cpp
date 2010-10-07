
#include <fstream>
#include <iostream>
#include <string>

#include "graphics.h"

using namespace std;


bool Graphics::loadObjects(const string& object_filename)
{
	ifstream object_file(object_filename.c_str());
	if(!object_file)
	{
		return false;
	}
	
	string current_part = "default";
	while(object_file.good() && !object_file.eof())
	{
		string cmd;
		object_file >> cmd;
		
		if(cmd == "NEXT")
		{
			// A new bodypart is to be defined
			object_file >> current_part;
		}
		
		if(cmd == "TRIANGLE")
		{
			objects[current_part].triangles.push_back(ObjectTri());
			float x, y, z;
			object_file >> x >> y >> z;
			objects[current_part].triangles.back().x[0] = x;
			objects[current_part].triangles.back().y[0] = y;
			objects[current_part].triangles.back().z[0] = z;
			
			object_file >> x >> y >> z;
			objects[current_part].triangles.back().x[1] = x;
			objects[current_part].triangles.back().y[1] = y;
			objects[current_part].triangles.back().z[1] = z;
			
			object_file >> x >> y >> z;
			objects[current_part].triangles.back().x[2] = x;
			objects[current_part].triangles.back().y[2] = y;
			objects[current_part].triangles.back().z[2] = z;
		}
		
		if(cmd == "ENDPOINT")
		{
			float x, y, z;
			object_file >> x >> y >> z;
			objects[current_part].end_x = x;
			objects[current_part].end_y = y;
			objects[current_part].end_z = z;
		}
	}
	return true;
}

