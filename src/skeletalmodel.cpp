#include "skeletalmodel.h"
#include <fstream>
#include <queue>
#include <iomanip>

using namespace std;

void SkeletalModel::tick(int current_frame)
{
	// Smoothed out model movement. Doesnt correspond 100% to actual unit coordinates.
	currentModelPos += (realUnitPos - currentModelPos) * 0.2;
	
	animation_time = current_frame;
}

void SkeletalModel::updatePosition(float x, float y, float z)
{
	realUnitPos.x = x;
	realUnitPos.y = y;
	realUnitPos.z = z;
}


void SkeletalModel::setAction(const string& name)
{
	if(name == animation_name)
		return;
	
	animation_name = name;
	animation_time = 0;
}


bool SkeletalModel::load(const string& filename)
{
	ifstream in(filename.c_str());
	if(!in)
	{
		return false;
	}

	string cmd;
	while(in >> cmd)
	{
		if(cmd == "MODEL")
		{
			bones.clear();
			triangles.clear();
		}
		else if(cmd == "BONE")
		{
			Bone bone;
			in >> bone.name;
			in >> bone.start_x >> bone.start_y >> bone.start_z;
			in >> bone.end_x >> bone.end_y >> bone.end_z;
			in >> bone.rotation_x >> bone.rotation_y >> bone.rotation_z;
			size_t count = 0;
			in >> count;
			for(size_t i = 0; i < count; ++i)
			{
				size_t child = 0;
				in >> child;
				bone.children.push_back(child);
			}
			bones.push_back(bone);
		}
		else if(cmd == "TRIANGLE")
		{
			WeightedTriangle wtriangle;
			ObjectTri& triangle = wtriangle.triangle;
			for(size_t i = 0; i < 3; ++i)
			{
				in >> triangle.x[i] >> triangle.y[i] >> triangle.z[i];
			}
			triangles.push_back(wtriangle);
		}
		else if(cmd == "TRIANGLE_WEIGHTS")
		{
			WeightedTriangle wtriangle = triangles.back();
			for(size_t i = 0; i < 3; ++i)
			{
				in >> wtriangle.bone1[i] >> wtriangle.weight1[i];
				in >> wtriangle.bone2[i] >> wtriangle.weight2[i];
			}
		}
	}

	return true;
}

bool SkeletalModel::save(const string& filename) const
{
	ofstream out(filename.c_str());
	if(!out)
	{
		return false;
	}

	out << "MODEL" << endl;
	out << endl;
	for(size_t i = 0; i < bones.size(); ++i)
	{
		const Bone& bone = bones[i];
		out << "BONE " << bone.name << endl;
		out << "    " << bone.start_x << " " << bone.start_y << " " << bone.start_z << endl;
		out << "    " << bone.end_x << " " << bone.end_y << " " << bone.end_z << endl;
		out << "    " << bone.rotation_x << " " << bone.rotation_y << " " << bone.rotation_z << endl;
		out << "    " << bone.children.size();
		for(size_t j = 0; j < bone.children.size(); ++j)
		{
			out << " " << bone.children[j];
		}
		out << endl;
	}
	out << endl;

	for(size_t i = 0; i < triangles.size(); ++i)
	{
		const WeightedTriangle& wtriangle = triangles[i];
		const ObjectTri& triangle = wtriangle.triangle;
		out << "TRIANGLE" << endl;
		for(size_t j = 0; j < 3; ++j)
		{
			out << "    " << triangle.x[j] << " " << triangle.y[j] << " " << triangle.z[j] << endl;
		}
		out << "TRIANGLE_WEIGHTS" << endl;
		for(size_t j = 0; j < 3; ++j)
		{
			out << "    " << wtriangle.bone1[j] << " " << wtriangle.weight1[j] << endl;
			out << "    " << wtriangle.bone2[j] << " " << wtriangle.weight2[j] << endl;
		}
	}
	return bool(out);
}

float SkeletalModel::height() const
{
	return -2.f;
}

