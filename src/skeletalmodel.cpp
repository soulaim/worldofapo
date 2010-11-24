#include "skeletalmodel.h"
#include "frustum/matrix4.h"
#include "texturehandler.h"

#include <fstream>
#include <queue>
#include <iomanip>
#include <iostream>
#include <cassert>

#include "shaders.h"

using namespace std;

extern int TRIANGLES_DRAWN_THIS_FRAME;

SkeletalModel::SkeletalModel():
	Model(),
	buffers_loaded(false),
	triangles_size(0)
{
	for(size_t i = 0; i < BUFFERS; ++i)
	{
		locations[i] = -1;
	}
}

SkeletalModel::SkeletalModel(const SkeletalModel& model):
	Model(model)
{
	buffers_loaded = model.buffers_loaded;
	for(size_t i = 0; i < BUFFERS; ++i)
	{
		locations[i] = model.locations[i];
	}
	triangles_size = model.triangles_size;

	// Always copy bones.
	bones = model.bones;

	// Copy rest of the things if they are not yet preloaded (which shouldn't happen)
	if(!buffers_loaded)
	{
		cerr << "Warning, copying not preloaded skeletalmodels." << endl;
		vertices = model.vertices;
		texture_coordinates = model.texture_coordinates;
		triangles = model.triangles;
		
		weighted_vertices = model.weighted_vertices;
	}
}

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

			const unsigned MAX_VERTICES = (1 << (sizeof(unsigned short)*8));
			if(vertices.size() < MAX_VERTICES)
			{
				vertices.push_back(v);
			}
			else
			{
				cerr << "WARNING: model '" << filename << "' has too many vertices (" << vertices.size() << " >= " << MAX_VERTICES << "), ignoring rest!" << endl;
			}
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
					cerr << "Malformed skeletal model '" << filename << "', vertex index: " << triangle.vertices[i] << endl;
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
	cerr << "Loaded model '" << filename << "' with " << vertices.size() << " vertices, " << weighted_vertices.size()
		<< " weighted vertices, " << texture_coordinates.size() << " texture coordinates, " << triangles.size()
		<< " triangles, and " << bones.size() << " bones." << endl;

	if(bones.empty())
	{
		cerr << "Warning: no bones on skeleton model: " << filename << endl;
		Bone bone;
		bone.name = "default";
		bones.push_back(bone);
	}

	size_t n = vertices.size();
	if(texture_coordinates.size() != n || weighted_vertices.size() != n)
	{
		cerr << "Warning: incomplete skeleton model: " << filename << endl;
		weighted_vertices.resize(n);
		texture_coordinates.resize(n);
	}

	triangles_size = triangles.size();

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

void SkeletalModel::calcMatrices(size_t current_bone, vector<Matrix4>& rotations, Matrix4 offset, const string& animation_name, int animation_state) const
{
	const Bone& bone = bones[current_bone];

	Animation& animation = Animation::getAnimation(bone.name, animation_name);
	// left and right sides of the body are in polarized animation_name states
	int ani_addition = 0;
	if(bone.name.substr(0, 4) == "LEFT")
	{
		ani_addition = animation.totalTime() / 2;
	}

	float rot_x = 0;
	float rot_y = 0;
	float rot_z = 0;
	animation.getAnimationState(animation_state + ani_addition, rot_x, rot_y, rot_z);

	// If models with more persistent rotation values are needed in future,
	// then don't update the rotation values when drawing, instead have a separate updating function...
	rot_y += bones[current_bone].rotation_y;

//	cerr << animation_state+ani_addition << " " << node.rotation_x << " " << node.rotation_y << " " << node.rotation_z << "\n";

	// TODO: pass only angles and bone locations to shader and do matrix calculation there.

	offset *= Matrix4(0,0,0, bone.start_x, bone.start_y, bone.start_z);
	offset *= Matrix4(rot_x, 0, 0, 0,0,0);
	offset *= Matrix4(0, rot_y, 0, 0,0,0);
	offset *= Matrix4(0, 0, rot_z, 0,0,0);
	offset *= Matrix4(0,0,0, -bone.start_x, -bone.start_y, -bone.start_z);

	rotations[current_bone] = offset;

	for(size_t i = 0; i < bone.children.size(); ++i)
	{
		calcMatrices(bone.children[i], rotations, offset, animation_name, animation_state);
	}
}

void SkeletalModel::draw() const
{
	draw(false, -1);
}

void SkeletalModel::draw_skeleton(const vector<Matrix4>& rotations, size_t hilight) const
{
	glBegin(GL_LINES);
	for(size_t i = 0; i < bones.size(); ++i)
	{
		const Bone& bone = bones[i];
		if(i == hilight)
		{
			glColor3f(1.0,0.0,0.0);
		}
		else
		{
			glColor3f(0.0,0.0,1.0);
		}
		Vec3 start(bone.start_x, bone.start_y, bone.start_z);
		Vec3 end(bone.end_x, bone.end_y, bone.end_z);
		Vec3 line_start = rotations[i] * start;
		Vec3 line_end = rotations[i] * end;
		
		glVertex3f(line_start.x, line_start.y, line_start.z);
		glVertex3f(line_end.x, line_end.y, line_end.z);
	}
	glEnd();
}

void SkeletalModel::old_draw(size_t hilight) const
{
	// This draw function is not used anymore because it is so slow!
	// Might be helpful for some debugging still.

	glBegin(GL_TRIANGLES);
	for(size_t i = 0; i < triangles.size(); ++i)
	{
		++TRIANGLES_DRAWN_THIS_FRAME;

//		glUniform4f(unit_color_location, 0.7, 0.7, 0.7, 1.0);

		for(int j = 0; j < 3; ++j)
		{
			size_t vi = triangles[i].vertices[j];
			const WeightedVertex& wv = weighted_vertices[vi];

			size_t bone1i = wv.bone1;
			size_t bone2i = wv.bone2;

			if(bone1i == hilight)
			{
				glColor3f(0, 1, 1);
			}
			else
			{
				glColor3f(0.5*j, 0.5*j,0.5*j);
			}

//			const Matrix4& offset1 = rotations[bone1i];
//			const Matrix4& offset2 = rotations[bone2i];

			float weight1 = wv.weight1;
			float weight2 = wv.weight2;

//			Bone& bone1 = bones[bone1i];
//			Bone& bone2 = bones[bone2i];

			Vec3 v = vertices[vi];
//			v = offset1 * v * weight1 + offset2 * v * weight2; // This is already done in the vertex shader.

			glVertexAttrib2f(bone_index_location, bone1i, bone2i);
			glVertexAttrib2f(bone_weight_location, weight1, weight2);
			glVertex3f(v.x, v.y, v.z);
			glTexCoord2f(texture_coordinates[vi].x, texture_coordinates[vi].y);
		}
	}
	glEnd();
}

void SkeletalModel::preload()
{
	std::cerr << "Preloading skeletalmodel buffers." << std::endl;

	glGenBuffers(BUFFERS, locations);

	glBindBuffer(GL_ARRAY_BUFFER, locations[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, locations[1]);
	glBufferData(GL_ARRAY_BUFFER, texture_coordinates.size() * sizeof(TextureCoordinate), &texture_coordinates[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(Triangle), &triangles[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, locations[3]);
	glBufferData(GL_ARRAY_BUFFER, weighted_vertices.size() * sizeof(WeightedVertex), &weighted_vertices[0], GL_STATIC_DRAW);

	buffers_loaded = true;
}

void SkeletalModel::draw(bool draw_only_skeleton, size_t hilight) const
{
	assert(weighted_vertices.size() == vertices.size());

	// does this need to be set each time before rendering a model?
	glUniform1i(active_location, true);

	vector<Matrix4> rotations;
	rotations.resize(bones.size());
	calcMatrices(0, rotations, Matrix4(), animation_name, animation_time);

	if(draw_only_skeleton)
	{
		draw_skeleton(rotations, hilight);
		return;
	}
	
	if(TextureHandler::getSingleton().getCurrentTexture(0) != texture_name)
	{
		TextureHandler::getSingleton().bindTexture(0, texture_name);
	}

	assert(rotations.size() <= 23);
	glUniformMatrix4fv(bones_location, rotations.size(), true, rotations[0].T);

//	old_draw(hilight);
//	return;

	assert(buffers_loaded);

	glBindBuffer(GL_ARRAY_BUFFER, locations[0]);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, locations[1]);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[2]);

	glBindBuffer(GL_ARRAY_BUFFER, locations[3]);
	glVertexAttribPointer(bone_index_location,  2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(WeightedVertex), 0);
	glVertexAttribPointer(bone_weight_location, 2, GL_FLOAT,        GL_FALSE, sizeof(WeightedVertex), (char*)(0) + 2*sizeof(unsigned));

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableVertexAttribArray(bone_weight_location);
	glEnableVertexAttribArray(bone_index_location);
	glDrawElements(GL_TRIANGLES, triangles_size * 3, GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(bone_index_location);
	glDisableVertexAttribArray(bone_weight_location);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	TRIANGLES_DRAWN_THIS_FRAME += triangles_size;
}

