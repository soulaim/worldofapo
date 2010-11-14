#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>

#include "graphics.h"
#include "apomodel.h"

using namespace std;


bool ApoModel::loadObjects(const string& object_filename)
{
	ifstream in(object_filename.c_str());
	if(!in)
	{
		return false;
	}
	
	string current_part = "default";
	string cmd;
	while(in >> cmd)
	{
		if(cmd == "NEXT")
		{
			// A new bodypart is to be defined
			in >> current_part;
		}
		else if(cmd == "TRIANGLE")
		{
			auto& object = ApoModel::objects[current_part];
			object.triangles.push_back(ObjectTri());
			float x, y, z;
			in >> x >> y >> z;
			object.triangles.back().x[0] = x;
			object.triangles.back().y[0] = y;
			object.triangles.back().z[0] = z;
			
			in >> x >> y >> z;
			object.triangles.back().x[1] = x;
			object.triangles.back().y[1] = y;
			object.triangles.back().z[1] = z;
			
			in >> x >> y >> z;
			object.triangles.back().x[2] = x;
			object.triangles.back().y[2] = y;
			object.triangles.back().z[2] = z;
		}
		else if(cmd == "ENDPOINT")
		{
			float x, y, z;
			in >> x >> y >> z;
			auto& object = ApoModel::objects[current_part];
			object.end_x = x;
			object.end_y = y;
			object.end_z = z;
		}
	}
	return in.eof();
}

bool ApoModel::saveObjects(const string& object_filename)
{
	ofstream out(object_filename.c_str());
	if(!out)
	{
		return false;
	}

	for(std::map<std::string, ObjectPart>::const_iterator it = ApoModel::objects.begin(), et = ApoModel::objects.end(); it != et; ++it)
	{
		const string& part_name = it->first;
		out << "NEXT " << part_name << "\n";
		const ObjectPart& part = it->second;

		for(size_t i = 0; i < part.triangles.size(); ++i)
		{
			const ObjectTri& triangle = part.triangles[i];
			out << "TRIANGLE " << fixed << setprecision(3) << triangle.x[0] << " " << triangle.y[0] << " " << triangle.z[0] << "\n";
			out << "         " << fixed << setprecision(3) << triangle.x[1] << " " << triangle.y[1] << " " << triangle.z[1] << "\n";
			out << "         " << fixed << setprecision(3) << triangle.x[2] << " " << triangle.y[2] << " " << triangle.z[2] << "\n";
		}

		out << "ENDPOINT " << fixed << setprecision(3) << part.end_x << " " << part.end_y << " " << part.end_z << "\n";
		out << "\n";
	}

	return bool(out);
}

