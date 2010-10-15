#include "model.h"
#include <fstream>
#include <queue>
#include <iomanip>

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
			for(size_t i = 0; i < parts.size(); ++i)
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

bool Model::save(const string& filename) const
{
	ofstream out(filename.c_str());
	if(!out)
	{
		return false;
	}

	out << "ROOT " << parts[root].name << " " << parts[root].wireframe << "\n";

	queue<size_t> to_be_printed;
	to_be_printed.push(root);
	while(!to_be_printed.empty())
	{
		size_t current = to_be_printed.front();
		to_be_printed.pop();

		for(size_t i = 0; i < parts[current].children.size(); ++i)
		{
			size_t child = parts[current].children[i];
			to_be_printed.push(child);
			out << "CHILD " << parts[current].name << " " << parts[child].name << " " << parts[child].wireframe << " "
				<< fixed << setprecision(3) << parts[child].offset_x << " " << parts[child].offset_y << " " << parts[child].offset_z << "\n";
		}
	}

	return bool(out);
}

float Model::height() const
{
	return -2.f;
}
