#include "graphics/models/skeletalmodel.h"
#include "graphics/frustum/matrix4.h"
#include "graphics/texturehandler.h"

#include <fstream>
#include <queue>
#include <iomanip>
#include <iostream>
#include <cassert>
#include <stdexcept>

#include "graphics/shaders.h"

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

	myScale = 1.0f; // default size
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

void SkeletalModel::viewTick()
{
	for(size_t i=0; i<bones.size(); i++)
	{
		if(bones[i].root)
		{
			bones[i].rotation_y = current_y_rot;
		}
	}
}

void SkeletalModel::calculate_weights()
{
	for(size_t i=0; i<vertices.size(); ++i)
	{
		const vec3<float>& v = vertices[i];

		map<int, float> weights;

		for(size_t k=0; k<bones.size(); ++k)
		{
			const Bone& bone = bones[k];

			vec3<float> start(bone.start_x, bone.start_y, bone.start_z);
			vec3<float> end(bone.end_x, bone.end_y, bone.end_z);

			vec3<float> segment = end - start;

			weights[k] = .0f;

			for(int z = 0; z < 100; z++)
			{
				vec3<float> tmp_point = start + segment * (z / 99.f);

				float length = (v - tmp_point).lengthSquared();
				float tmp_w = 1.0f / (0.01f + length);
				if(tmp_w > weights[k])
					weights[k] = tmp_w;
			}
		}

		// we want 2 bones
		WeightedVertex vw;

		for(int z = 0; z < 2; z++)
		{
			int best_index = 0;
			float best_weight = 0.f;

			for(auto it = weights.begin(); it != weights.end(); it++)
			{
				if(it->second > best_weight)
				{
					best_index = it->first;
					best_weight = it->second;
				}
			}

			if(z == 0)
			{
				vw.bone1 = best_index;
				vw.weight1 = best_weight;
			}

			if(z == 1)
			{
				vw.bone2 = best_index;
				vw.weight2 = best_weight;
			}

			weights.erase(best_index); // don't select same bone twice.
		}

		// normalise weights
		float sum = vw.weight1 + vw.weight2;
		vw.weight1 /= sum;
		vw.weight2 /= sum;

		weighted_vertices.push_back(vw);
	}
}


bool SkeletalModel::load(const string& filename)
{
	ifstream in(filename.c_str());
	if(!in)
	{
		return false;
	}

	// NOTE: Since we don't have per triangle texture/color properties,
	// we have to gather all the info from FACE fields and sort it before placing to containers.
	map<int, TextureCoordinate> tex_coords_map;
	map<int, ColorStruct> colors_map;

	// NOTE2: In order to have per vertex attributes we can make duplicate copies of the vertex coordinates and normals.
	vector<vec3<float> > final_vertices;
	vector<vec3<float> > final_normals;

	// TODO: Not all vertices need to be duplicated for all triangles. When adding a new vertex, check if some old vertex has the same coordinates and UV coordinates etc.
	// TODO: Make this dirty duplicating already in the blender exporter?

	string cmd;
	while(in >> cmd)
	{

		if(cmd == "MODEL")
		{
			vertices.clear();
			weighted_vertices.clear();
			texture_coordinates.clear();
			normals.clear();
			colors.clear();

			bones.clear();
			triangles.clear();
			triangle_normals.clear();
			triangle_areas.clear();
		}

		else if(cmd == "MESH" || cmd == "END_MESH")
		{
			// ...
		}
		else if(cmd == "BONE")
		{
			Bone bone;
			in >> bone.name;

			string sub_cmd;

			while(in >> sub_cmd)
			{
				if(sub_cmd == "START_POS")
					in >> bone.start_x >> bone.start_y >> bone.start_z;
				else if(sub_cmd == "END_POS")
					in >> bone.end_x >> bone.end_y >> bone.end_z;
				else if(sub_cmd == "HEAD_RADIUS")
					in >> bone.start_r;
				else if(sub_cmd == "TAIL_RADIUS")
					in >> bone.end_r;
				else if(sub_cmd == "BONE_WEIGHT")
					in >> bone.w;
				else if(sub_cmd == "ROOT")
					bone.root = true;
				else if(sub_cmd == "CHILDREN")
				{
					int count;
					in >> count;
					for(int i=0; i<count; i++)
					{
						in >> sub_cmd;
						bone.children_names.push_back(sub_cmd);
					}
				}
				else if(sub_cmd == "END_BONE")
				{
					break;
				}
				else
				{
					throw std::logic_error(string("Skeletal Model could not be loaded, bone parameter unrecognized: ") + sub_cmd);
				}
			}

			bones.push_back(bone);
		}
		else if(cmd == "VERTEX")
		{
			string sub_cmd;
			vec3<float> v;
			vec3<float> n;

			while(in >> sub_cmd)
			{
				if(sub_cmd == "COORDINATE")
				{
					in >> v.x >> v.y >> v.z;
					if(v.length() > 10)
						cerr << "DUBIOUS: vertex length " << v.length() << endl;
				}
				else if(sub_cmd == "NORMAL")
				{
					in >> n.x >> n.y >> n.z;
				}
				else if(sub_cmd == "END_VERTEX")
				{
					break;
				}
				else
				{
					throw std::logic_error(string("Skeletal Model could not be loaded, vertex parameter unrecognized: ") + sub_cmd);
				}
			}

			if(v.length() > 10)
				cerr << "DUBIOUS: post-check: vertex length " << v.length() << endl;

			const unsigned MAX_VERTICES = (1 << (sizeof(unsigned short)*8));
			if(vertices.size() < MAX_VERTICES)
			{
				vertices.push_back(v);
				normals.push_back(n);
			}
			else
			{
				cerr << "WARNING: model '" << filename << "' has too many vertices (" << vertices.size() << " >= " << MAX_VERTICES << "), ignoring rest!" << endl;
			}
		}
		else if(cmd == "FACE")
		{
			string sub_cmd;

			Triangle triangle;
			vec3<float> triangle_n;
			float triangle_area = 0.0f;

			while(in >> sub_cmd)
			{
				if(sub_cmd == "TRIANGLE")
				{
					for(size_t i = 0; i < 3; ++i)
					{
						in >> triangle.vertices[i];

						if(triangle.vertices[i] >= vertices.size())
						{
							cerr << triangle.vertices[i] << " >= " << vertices.size() << endl;
							throw std::logic_error(string("SkeletalModel triangle's vertex index out of bounds."));
						}

						final_vertices.push_back(vertices.at(triangle.vertices[i]));
						final_normals.push_back(normals.at(triangle.vertices[i]));
						triangle.vertices[i] = final_vertices.size() - 1;
					}
				}
				else if(sub_cmd == "NORMAL")
				{
					in >> triangle_n.x >> triangle_n.y >> triangle_n.z;
				}
				else if(sub_cmd == "UV_COORDINATES")
				{
					for(int i = 0; i < 3; ++i)
					{
						in >> tex_coords_map[triangle.vertices[i]].x >> tex_coords_map[triangle.vertices[i]].y;
					}
				}
				else if(sub_cmd == "COLORS")
				{
					for(int i = 0; i < 3; ++i)
					{
						in >>
							colors_map[triangle.vertices[i]].r >>
							colors_map[triangle.vertices[i]].g >>
							colors_map[triangle.vertices[i]].b >>
							colors_map[triangle.vertices[i]].a;
					}
				}
				else if(sub_cmd == "AREA")
				{
					in >> triangle_area;
				}
				else if(sub_cmd == "END_FACE")
				{
					break;
				}
				else
				{
					throw std::logic_error(string("Skeletal Model could not be loaded, face parameter unrecognized: ") + sub_cmd);
				}

			}

			for(size_t i = 0; i < 3; ++i)
			{
//				if(triangle.vertices[i] >= vertices.size())
				if(triangle.vertices[i] >= final_vertices.size())
				{
					cerr << triangle.vertices[i] << " >= " << vertices.size() << endl;
					throw std::logic_error(string("SkeletalModel at post-check: triangle's vertex index out of bounds."));
				}
			}

			triangles.push_back(triangle);
			triangle_normals.push_back(triangle_n);
			triangle_areas.push_back(triangle_area);
		}
		else
		{
			throw std::logic_error(string("SkeletalModel could not be loaded, unknown field: ") + cmd);
		}
	}

	assert(final_normals.size() == final_vertices.size());
//	assert(tex_coords_map.size() == final_vertices.size());
//	assert(colors_map.size() == final_vertices.size());
	vertices.swap(final_vertices);
	normals.swap(final_normals);

	// get uv coordinates
	for(auto it = tex_coords_map.begin(); it != tex_coords_map.end(); it++)
	{
		texture_coordinates.push_back(it->second);
	}

	// get colors
	for(auto it = colors_map.begin(); it != colors_map.end(); it++)
	{
		colors.push_back(it->second);
	}

	if(bones.size() < 2)
	{
		throw std::logic_error(string("SkeletalModel has less than two bones: ") + filename);
	}


	// resolve bone dependencies
	for(size_t i=0; i<bones.size(); i++)
	{
		for(size_t b=0; b<bones[i].children_names.size(); b++)
		{
			for(size_t k=0; k<bones.size(); k++)
			{
				if(bones[k].name == bones[i].children_names[b])
				{
					bones[i].children.push_back(k);
				}
			}
		}

		// free the memory required by storing bone children names.
		bones[i].children_names.clear();
	}

	calculate_weights();

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
	// TODO: this doesnt do it.
	return false;

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

	Animation* p_animation;
	Animation& try_animation = Animation::getAnimation(bone.name, animation_name);

	if(try_animation.totalTime() == 0)
	{
		// lalala...
		if(animation_name.substr(0, 3) == "run")
		{
			Animation& try_animation2 = Animation::getAnimation(bone.name, "run");
			p_animation = &try_animation2;
		}
		else
			p_animation = &try_animation;
	}
	else
		p_animation = &try_animation;

	Animation& animation = *p_animation;

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
	offset *= Matrix4(0, rot_y, 0, 0,0,0);
	offset *= Matrix4(0, 0, rot_z, 0,0,0);
	offset *= Matrix4(rot_x, 0, 0, 0,0,0);
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
		vec3<float> start(bone.start_x, bone.start_y, bone.start_z);
		vec3<float> end(bone.end_x, bone.end_y, bone.end_z);
		vec3<float> line_start = rotations[i] * start;
		vec3<float> line_end = rotations[i] * end;

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

			vec3<float> v = vertices[vi];
//			v = offset1 * v * weight1 + offset2 * v * weight2; // This is already done in the vertex shader.

			glVertexAttrib2f(bone_index_location, bone1i, bone2i);
			glVertexAttrib2f(bone_weight_location, weight1, weight2);
			glVertex3f(v.x, v.y, v.z);
			glTexCoord2f(texture_coordinates[vi].x, texture_coordinates[vi].y);
		}
	}
	glEnd();
}


// TODO: This should not be a member of the Model, but of the renderer who takes the SkeletalModel as a parameter.
//       The renderer could also check whether a model with the name of the given model has already been preloaded or not.
void SkeletalModel::preload()
{
	std::cerr << "Sending model data to VideoMemory.. " << std::flush;

	assert(vertices.size() == texture_coordinates.size());
	assert(vertices.size() == weighted_vertices.size());
	assert(vertices.size() == normals.size());

	glGenBuffers(BUFFERS, locations);

	size_t buffer = 0;
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3<float>), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, texture_coordinates.size() * sizeof(TextureCoordinate), &texture_coordinates[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(Triangle), &triangles[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, weighted_vertices.size() * sizeof(WeightedVertex), &weighted_vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3<float>), &normals[0], GL_STATIC_DRAW);

	assert(buffer == BUFFERS);

	buffers_loaded = true;

	std::cerr << "OK" << std::endl;
}

void SkeletalModel::draw_normals() const
{
	assert(vertices.size() == normals.size());

	glColor3f(1, 1, 0);
	glBegin(GL_LINES);
	for(size_t i = 0; i < vertices.size(); ++i)
	{
		vec3<float> v = vertices[i] * myScale - currentModelPos;
		vec3<float> n = normals[i];

		glVertex3f(v.x, v.y, v.z);
		glVertex3f(v.x + n.x, v.y + n.y, v.z + n.z);
	}
	glEnd();
}

void SkeletalModel::draw(bool draw_only_skeleton, size_t hilight) const
{
	assert(weighted_vertices.size() == vertices.size());

	draw_normals();

	glUniform1f(model_scale_location, myScale);

	vector<Matrix4> rotations;
	rotations.resize(bones.size());

	for(size_t i=0; i<bones.size(); i++)
	{
		if(bones[i].root)
			calcMatrices(i, rotations, Matrix4(), animation_name, animation_time);
	}

	if(draw_only_skeleton)
	{
		draw_skeleton(rotations, hilight);
		return;
	}

	if(TextureHandler::getSingleton().getCurrentTexture(0) != texture_name)
	{
		if(texture_name.empty())
		{
			TextureHandler::getSingleton().unbindTexture(0);
		}
		else
		{
			TextureHandler::getSingleton().bindTexture(0, texture_name);
		}
	}

	assert(rotations.size() < 23);
	glUniformMatrix4fv(bones_location, rotations.size(), true, rotations[0].T);

	//old_draw(hilight);
	//return;

	draw_buffers();
}

void SkeletalModel::draw_buffers() const
{
	assert(buffers_loaded);

	size_t buffer = 0;
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[buffer++]);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glVertexAttribPointer(bone_index_location,  2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(WeightedVertex), 0);
	glVertexAttribPointer(bone_weight_location, 2, GL_FLOAT,        GL_FALSE, sizeof(WeightedVertex), (char*)(0) + 2*sizeof(unsigned));

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glNormalPointer(GL_FLOAT, 0, 0);

	assert(buffer == BUFFERS);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableVertexAttribArray(bone_weight_location);
	glEnableVertexAttribArray(bone_index_location);
	glDrawElements(GL_TRIANGLES, triangles_size * 3, GL_UNSIGNED_SHORT, 0);
	glDisableVertexAttribArray(bone_index_location);
	glDisableVertexAttribArray(bone_weight_location);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	TRIANGLES_DRAWN_THIS_FRAME += triangles_size;
}

