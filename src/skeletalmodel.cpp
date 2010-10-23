#include "skeletalmodel.h"
#include "frustum/matrix4.h"
#include "texturehandler.h"

#include <fstream>
#include <queue>
#include <iomanip>
#include <iostream>

#include "shaders.h"

using namespace std;

extern int TRIANGLES_DRAWN_THIS_FRAME;

void SkeletalModel::rotate_y(float angle)
{
	bones[0].rotation_y = angle;
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
			vertices.clear();
			weighted_vertices.clear();
			texture_coordinates.clear();
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
		else if(cmd == "VERTEX")
		{
			Vec3 v;
			in >> v.x >> v.y >> v.z;
			vertices.push_back(v);
		}
		else if(cmd == "TEXTURE_COORDINATES")
		{
			TextureCoordinate tc;
			in >> tc.x >> tc.y;
			texture_coordinates.push_back(tc);
		}
		else if(cmd == "VERTEX_WEIGHTS")
		{
			WeightedVertex v;
			in >> v.bone1 >> v.weight1;
			in >> v.bone2 >> v.weight2;
			weighted_vertices.push_back(v);
		}
		else if(cmd == "TRIANGLE")
		{
			Triangle triangle;
			for(size_t i = 0; i < 3; ++i)
			{
				in >> triangle.vertices[i];
				if(triangle.vertices[i] >= vertices.size())
				{
					cerr << "Malformed: " << filename << endl;
				}
			}
			triangles.push_back(triangle);
		}
		else
		{
			cerr << "Error loading skeleton model " << filename << " cmd = " << cmd << endl;
			return false;
		}
	}
	cerr << "Loaded model " << filename << " with " << vertices.size() << " vertices, " << weighted_vertices.size()
		<< " weighted vertices, " << texture_coordinates.size() << " texture coordinates, " << triangles.size()
		<< " triangles, and " << bones.size() << " bones." << endl;

	size_t n = vertices.size();
	if(texture_coordinates.size() != n || weighted_vertices.size() != n)
	{
		cerr << "Warning: incomplete skeleton model: " << filename << endl;
		weighted_vertices.resize(n);
		texture_coordinates.resize(n);
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

	for(size_t i = 0; i < vertices.size(); ++i)
	{
		out << "VERTEX" << endl;
		out << fixed << setprecision(3);
		out << "    " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << endl;

		out << "TEXTURE_COORDINATES" << endl;
		out << fixed << setprecision(3);
		out << "    " << texture_coordinates[i].x << " " << texture_coordinates[i].y << endl;

		out << "VERTEX_WEIGHTS" << endl;
		out << fixed << setprecision(3);
		out << "    " << weighted_vertices[i].bone1 << " " << weighted_vertices[i].weight1 << endl;
		out << "    " << weighted_vertices[i].bone2 << " " << weighted_vertices[i].weight2 << endl;
	}

	for(size_t i = 0; i < triangles.size(); ++i)
	{
		out << "TRIANGLE" << endl;
		out << "   ";
		for(size_t j = 0; j < 3; ++j)
		{
			out << " " << triangles[i].vertices[j];
		}
		out << endl;
	}
	return bool(out);
}

float SkeletalModel::height() const
{
	return 0.f;
}

void calcMatrices(SkeletalModel& model, Vec3 prev, size_t current_bone, vector<Matrix4>& rotations, Matrix4 offset, const string& animation_name, int animation_state)
{
	Bone& bone = model.bones[current_bone];

	Animation& animation = Animation::getAnimation(bone.name, animation_name);
	// left and right sides of the body are in polarized animation_name states
	int ani_addition = 0;
	if(bone.name.substr(0, 4) == "LEFT")
	{
		ani_addition = animation.totalTime() / 2;
	}
	animation.getAnimationState(animation_state + ani_addition, bone.rotation_x, bone.rotation_y, bone.rotation_z);
//	cerr << animation_state+ani_addition << " " << node.rotation_x << " " << node.rotation_y << " " << node.rotation_z << "\n";

	offset *= Matrix4(0,0,0, bone.start_x - prev.x, bone.start_y - prev.y, bone.start_z - prev.z);

	offset *= Matrix4(bone.rotation_x, 0, 0, 0,0,0);
	offset *= Matrix4(0, bone.rotation_y, 0, 0,0,0);
	offset *= Matrix4(0, 0, bone.rotation_z, 0,0,0);

	rotations[current_bone] = offset;

	for(size_t i = 0; i < bone.children.size(); ++i)
	{
		calcMatrices(model, Vec3(bone.start_x, bone.start_y, bone.start_z), bone.children[i], rotations, offset, animation_name, animation_state);
	}
}

void SkeletalModel::draw()
{
	draw(false, -1);
}

void SkeletalModel::draw(bool draw_skeleton, size_t hilight)
{
	glEnable(GL_TEXTURE_2D);
	TextureHandler::getSingleton().bindTexture(texture_name);

	vector<Matrix4> rotations;
	rotations.resize(bones.size());
	calcMatrices(*this, Vec3(), 0, rotations, Matrix4(), animation_name, animation_time);

	for(size_t i = 0; i < triangles.size(); ++i)
	{
		++TRIANGLES_DRAWN_THIS_FRAME;

		glUniform4f(unit_color_location, 0.7, 0.7, 0.7, 1.0);

		glBegin(GL_TRIANGLES);
		for(int j = 0; j < 3; ++j)
		{
			size_t vi = triangles[i].vertices[j];
			WeightedVertex& wv = weighted_vertices[vi];

			const Matrix4& offset1 = rotations[wv.bone1];
			const Matrix4& offset2 = rotations[wv.bone2];
			float weight1 = wv.weight1;
			float weight2 = wv.weight2;
			Bone& bone1 = bones[wv.bone1];
			Bone& bone2 = bones[wv.bone2];

			Vec3 v1 = vertices[vi];
			Vec3 vc1(bone1.start_x, bone1.start_y, bone1.start_z);
			v1 -= vc1;

			Vec3 v2 = vertices[vi];
			Vec3 vc2(bone2.start_x, bone2.start_y, bone2.start_z);
			v2 -= vc2;

			Vec3 v = offset1 * v1 * weight1 + offset2 * v2 * weight2;


			if(wv.bone1 == hilight)
			{
				glColor3f(0, 1, 1);
			}
			else
			{
				glColor3f(0.5*j, 0.5*j,0.5*j);
			}
			glTexCoord2f(texture_coordinates[vi].x, texture_coordinates[vi].y);
			glVertex3f(v.x, v.y, v.z);
		}
		glEnd();
	}

	if(draw_skeleton)
	{
		glColor3f(0.0,0.0,1.0);
		for(size_t i = 0; i < bones.size(); ++i)
		{
			const Bone& bone = bones[i];
			Vec3 start(bone.start_x, bone.start_y, bone.start_z);
			Vec3 end(bone.end_x, bone.end_y, bone.end_z);
			Vec3 line_start = rotations[i] * Vec3(0,0,0);
			Vec3 line_end = rotations[i] * (end - start);
			
			glBegin(GL_LINES);
			glVertex3f(line_start.x, line_start.y, line_start.z);
			glVertex3f(line_end.x, line_end.y, line_end.z);
			glEnd();
//			cerr << i << " has start at " << line_start << "\n";
		}
	}
	glDisable(GL_TEXTURE_2D);
}


