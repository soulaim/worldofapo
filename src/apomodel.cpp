#include "apomodel.h"
#include "primitives.h"
#include "graphics.h"

#include "shaders.h"

#include <fstream>
#include <queue>
#include <iostream>
#include <iomanip>

using namespace std;

extern int TRIANGLES_DRAWN_THIS_FRAME;

ApoModel::~ApoModel()
{
}

bool ApoModel::load(const string& filename)
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

bool ApoModel::save(const string& filename) const
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

void ApoModel::drawPartsRecursive(int current_node)
{
	if(current_node < 0 || size_t(current_node) >= parts.size())
	{
		return;
	}
	ModelNode& node = parts[current_node];
	ObjectPart& obj_part = Graphics::objects[node.wireframe];
	
	Animation& animation = Animation::getAnimation(node.name, animation_name);
	// left and right sides of the body are in polarized animation_name states
	int ani_addition = 0;
	if(node.name.substr(0, 4) == "LEFT")
	{
		ani_addition = animation.totalTime() / 2;
	}
	animation.getAnimationState(animation_time + ani_addition, node.rotation_x, node.rotation_y, node.rotation_z);
	
	if(node.hilight)
	{
		glUniform4f(unit_color_location, 1.0, 0.0, 0.0, 1.0);
	}
	glTranslatef(node.offset_x, node.offset_y, node.offset_z);
	
	glRotatef(node.rotation_x, 1, 0, 0);
	glRotatef(node.rotation_y, 0, 1, 0);
	glRotatef(node.rotation_z, 0, 0, 1);
	
	glBegin(GL_TRIANGLES);
	for(size_t i=0; i<obj_part.triangles.size(); i++)
	{
		++TRIANGLES_DRAWN_THIS_FRAME;
		// how to choose textures??
		ObjectTri& tri = obj_part.triangles[i];
		glVertex3f(tri.x[0], tri.y[0], tri.z[0]);

		glVertex3f(tri.x[1], tri.y[1], tri.z[1]);

		glVertex3f(tri.x[2], tri.y[2], tri.z[2]);
//		cerr << current_node << "\n";
	}
	glEnd();
	
	glTranslatef(obj_part.end_x, obj_part.end_y, obj_part.end_z);
	for(size_t i=0; i<node.children.size(); i++)
	{
		drawPartsRecursive(node.children[i]);
	}
	glTranslatef(-obj_part.end_x, -obj_part.end_y, -obj_part.end_z);
	
	if(node.hilight)
	{
		glUniform4f(unit_color_location, 0.7, 0.7, 0.7, 0.5);
	}
	
	// restore rotations
	glRotatef(-node.rotation_z, 0, 0, 1);
	glRotatef(-node.rotation_y, 0, 1, 0);
	glRotatef(-node.rotation_x, 1, 0, 0);
	
	glTranslatef(-node.offset_x, -node.offset_y, -node.offset_z);
}

void ApoModel::draw()
{
	if(root < 0)
	{
		cerr << "ERROR: There exists a Model descriptor which is empty! (not drawing it)" << endl;
		return;
	}
	glUniform1i(active_location, false);
	drawPartsRecursive(root);
}

void ApoModel::rotate_y(float angle)
{
	parts[root].rotation_y = angle;
}

