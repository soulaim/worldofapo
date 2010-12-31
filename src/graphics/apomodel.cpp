#include "apomodel.h"
#include "primitives.h"

#include "shaders.h"

#include <fstream>
#include <queue>
#include <iostream>
#include <iomanip>

using namespace std;

extern int TRIANGLES_DRAWN_THIS_FRAME;

std::map<std::string, ObjectPart> ApoModel::objects;

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

	size_t sum = 0;
	for(size_t i = 0; i < parts.size(); ++i)
	{
		sum += ApoModel::objects[parts[i].wireframe].triangles.size();
	}
	cerr << "Loaded model '" << filename << "' with " << parts.size() << " parts and " << sum << " triangles" << endl;

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

void ApoModel::drawPartsRecursive(int current_node) const
{
	if(current_node < 0 || size_t(current_node) >= parts.size())
	{
		return;
	}
	const ModelNode& node = parts[current_node];
	ObjectPart& obj_part = ApoModel::objects[node.wireframe];
	
	Animation& animation = Animation::getAnimation(node.name, animation_name);
	// left and right sides of the body are in polarized animation_name states
	int ani_addition = 0;
	if(node.name.substr(0, 4) == "LEFT")
	{
		ani_addition = animation.totalTime() / 2;
	}
	float rot_x = 0;
	float rot_y = 0;
	float rot_z = 0;
	animation.getAnimationState(animation_time + ani_addition, rot_x, rot_y, rot_z);

	// If models with more persistent rotation values are needed in future,
	// then don't update the rotation values when drawing, instead have a separate updating function...
	rot_y += parts[root].rotation_y;
	
	if(node.hilight)
	{
		glUniform4f(unit_color_location, 1.0, 0.0, 0.0, 1.0);
	}
	glTranslatef(node.offset_x, node.offset_y, node.offset_z);
	
	glRotatef(rot_x, 1, 0, 0);
	glRotatef(rot_y, 0, 1, 0);
	glRotatef(rot_z, 0, 0, 1);
	
	glBegin(GL_TRIANGLES);
	for(size_t i=0; i<obj_part.triangles.size(); i++)
	{
		++TRIANGLES_DRAWN_THIS_FRAME;
		// how to choose textures??
		ObjectTri& tri = obj_part.triangles[i];
		glVertex3f(tri.x[0] * myScale, tri.y[0] * myScale, tri.z[0] * myScale);

		glVertex3f(tri.x[1] * myScale, tri.y[1] * myScale, tri.z[1] * myScale);

		glVertex3f(tri.x[2] * myScale, tri.y[2] * myScale, tri.z[2] * myScale);
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
		glUniform4f(unit_color_location, 0.7f, 0.7f, 0.7f, 0.5f);
	}
	
	// restore rotations
	glRotatef(-node.rotation_z, 0, 0, 1);
	glRotatef(-node.rotation_y, 0, 1, 0);
	glRotatef(-node.rotation_x, 1, 0, 0);
	
	glTranslatef(-node.offset_x, -node.offset_y, -node.offset_z);
}

void ApoModel::draw() const
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

