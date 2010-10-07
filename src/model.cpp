

#include "model.h"
#include <fstream>

using namespace std;


void Model::tick()
{
	// Smoothed out model movement. Doesnt correspond 100% to actual unit coordinates.
	currentModelPos += (realUnitPos - currentModelPos) * 0.2;
	
	// animation time should be increased by the actual time difference, instead of by a constant
	// FIX!!
	animation_time++;
	return;
}

void Model::updatePosition(float x, float y, float z)
{
	realUnitPos.x = x;
	realUnitPos.y = y;
	realUnitPos.z = z;
}


void Model::setAction(const string& name)
{
	if(name == animation_name)
		return;
	
	animation_name = name;
	animation_time = 0;
}


bool Model::load(const string& filename)
{
	ifstream in_file(filename.c_str());
	if(!in_file)
	{
		return false;
	}

	while(in_file.good() && !in_file.eof())
	{
		string cmd, father, child, wirename;
		float osx, osy, osz;
		in_file >> cmd;
		
		if(cmd == "CHILD")
		{
			in_file >> father >> child >> wirename >> osx >> osy >> osz;
			for(int i=0; i<parts.size(); i++)
			{
				if(parts[i].name == father)
				{
					parts[i].children.push_back(parts.size());
					break;
				}
			}
			
			ModelNode node;
			node.name = child;
			node.wireframe = wirename;
			node.offset_x = osx;
			node.offset_y = osy;
			node.offset_z = osz;
			parts.push_back(node);
			
		}
		else if(cmd == "ROOT")
		{
			in_file >> father >> wirename;
			root = parts.size();
			
			ModelNode node;
			node.name = father;
			node.wireframe = wirename;
			parts.push_back(node);
		}
	}
	return true;
}

