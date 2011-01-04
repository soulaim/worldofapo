#include "editor.h"
#include "graphics/texturehandler.h"
#include "graphics/grasscluster.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <climits>
#include <locale>
#include <iomanip>
#include <set>

using namespace std;

extern vector<pair<Vec3,Vec3> > LINES;
extern vector<Vec3> DOTS;
GrassCluster meadows;
Vec3 wind;

int r = 50;
int g = 50;
int b = 160;

std::string red(const std::string& s)
{
	return "^r" + s;
}

std::string green(const std::string& s)
{
	return "^g" + s;
}

Editor::Editor():
	window(),
	view(window, hud)
{
	drawing_model = true;
	drawing_skeleton = true;
	skele = false;
	speed = 0.1f;
	rotate_speed = 45.0f/2.0f;
	current_command = 0;

	skybox = false;
	editing_single_part = false;
	edited_type = 0;
	selected_part = 0;
	selected_dot = 0;
	userio.init();
	visualworld.init();

	view.init(visualworld.camera);

	visualworld.bindCamera(&dummy);
//	view.toggleLightingStatus();

	TextureHandler::getSingleton().createTextures("data/textures.txt");
	

//	handle_command("load objects model.parts");
//	handle_command("load model model.bones");
//	handle_command("load skele model.skeleton");
//	handle_command("load animations model.animation");
	handle_command("load animations skeleani.animation");
//	handle_command("load objects bullet.parts");
//	handle_command("edit type HEAD");
	commands.push_back("load skele marine.sm2");
	current_command = commands.size();

//	view.megaFuck();

	LINES.push_back(make_pair(Vec3(100,0,0),Vec3(-100,0,0)));
	LINES.push_back(make_pair(Vec3(0,100,0),Vec3(0,-100,0)));
	LINES.push_back(make_pair(Vec3(0,0,100),Vec3(0,0,-100)));

	meadows.preload();
}

void Editor::release_swarm()
{
	for(auto it = models.begin(); it != models.end(); ++it)
	{
		delete it->second;
	}
	models.clear();
}

void Editor::release_particles()
{
	visualworld.terminate();
}

Editor::~Editor()
{
	cout << "~Editor()\n";
	release_swarm();
}

bool Editor::do_tick()
{
	if(!handle_input())
	{
		return false;
	}

	string message;

	if(editing_single_part)
	{
		LINES.resize(3);
		// TODO: print dx,dy,dz
		stringstream ss;
		ss << current_dot;
		message = "Editing '" + edited_type_name + "': " + ss.str();

		for(size_t i = 0; i < new_dots.size(); ++i)
		{
			// Line i -> current.
			LINES.push_back(make_pair(new_dots[i],current_dot));
			for(size_t j = 0; j < i; ++j)
			{
				// Line j -> i.
				LINES.push_back(make_pair(new_dots[j], new_dots[i]));
			}
		}

		DOTS.clear();
		DOTS.push_back(current_dot);
	}
	else
	{
		stringstream ss;
		if(skele)
		{
			if(skeletal_model.bones.size() > selected_part)
			{
				const Bone& bone = skeletal_model.bones[selected_part];
				ss << "'" << bone.name << "' at ("
					<< fixed << setprecision(2) << bone.start_x << ", " << bone.start_y << ", " << bone.start_z << "), ["
					<< fixed << setprecision(2) << bone.rotation_x << ", " << bone.rotation_y << ", " << bone.rotation_z << "]";
			}
		}
		else
		{
			if(edited_model.parts.size() > selected_part)
			{
				const ModelNode& node = edited_model.parts[selected_part];
				ss << "'" << node.name << "' is " << node.wireframe << " at ("
					<< fixed << setprecision(2) << node.offset_x << ", " << node.offset_y << ", " << node.offset_z << "), ["
					<< fixed << setprecision(2) << node.rotation_x << ", " << node.rotation_y << ", " << node.rotation_z << "]";
			}
		}

		message = ss.str();
	}

	view.startDrawing();
	if(skybox)
	{
		view.drawSkybox();
	}
	else
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	if(skele)
	{
		if(drawing_skeleton)
		{
			skeletal_model.draw(true, selected_part);
		}
		if(drawing_model)
		{
//			skeletal_model.draw(false, selected_part);
			models[0] = &skeletal_model;
			view.drawModels(models);
		}
	}
	else
	{
		if(drawing_model && edited_model.root >= 0)
		{
//			edited_model.draw();
			models[0] = &edited_model;
			view.drawModels(models);
		}
	}

	view.drawDebugLines();
	
	/*
	static int counter = 0;
	++counter;
	winds.resize(meadows.size());
	for(size_t i = 0; i < winds.size(); ++i)
	{
		Vec3& w = winds[i];
		w.x = wind.x * sin((counter + 20 * meadows[i].z) / 100.0);
		w.y = wind.y * sin((counter + 20 * (meadows[i].x + meadows[i].y)) / 100.0) * 0.2;
		w.z = wind.z * sin((counter + 20 * meadows[i].x) / 100.0);
	}
	*/
	view.drawGrass(vector<GrassCluster>(1, meadows));

	if(view.intVals["DEFERRED_RENDERING"])
	{
		view.applyDeferredLights(visualworld.lights);
	}

	hud.drawFPS();
	hud.drawMessages();
	hud.drawString(message, -0.9, 0.9, 1.5, true);
	hud.clearDebugStrings();

	view.drawParticles(visualworld.particles);

	view.finishDrawing();

	models.erase(0);
	return true;
}

bool Editor::tick()
{
	int ticks = SDL_GetTicks();
	static int last_tick = -999999;
	static int world_ticks = 0;

	int time_since_last = ticks - last_tick;
	int time_between_ticks = 1000/50;
	if(time_since_last >= time_between_ticks)
	{
		++world_ticks;
		last_tick = ticks;

		hud.setTime( ticks );
		view.tick();
		edited_model.tick(world_ticks);
		skeletal_model.tick(world_ticks);
		for(auto it = models.begin(); it != models.end(); ++it)
		{
			it->second->tick(it->second->animation_time + 1);
		}
	
		visualworld.camera.tick();
		visualworld.tickParticles();

		return do_tick();
	}
	else
	{
		SDL_Delay(time_between_ticks - time_since_last);
		return true;
	}
}

bool Editor::type_exists(const std::string& type)
{
	return ApoModel::objects.count(type) > 0;
}

void Editor::saveModel(const std::string& file)
{
	string pathed_file = "models/" + file;
	hud.pushMessage("Saving model to '" + pathed_file + "'");
	if(edited_model.save(pathed_file))
	{
		hud.pushMessage(green("Success"));
		modelFile = file;
		skele = false;
	}
	else
	{
		hud.pushMessage(red("Fail"));
	}
}

void Editor::saveSkeletalModel(const std::string& file)
{
	string pathed_file = "models/" + file;
	hud.pushMessage("Saving skeletal model to '" + pathed_file + "'");
	if(skeletal_model.save(pathed_file))
	{
		hud.pushMessage(green("Success"));
		modelFile = file;
		skele = true;
	}
	else
	{
		hud.pushMessage(red("Fail"));
	}
}

void Editor::saveObjects(const std::string& file)
{
	string pathed_file = "models/" + file;
	hud.pushMessage("Saving objects to '" + pathed_file + "'");
	if(ApoModel::saveObjects(pathed_file))
	{
		hud.pushMessage(green("Success"));
		objectsFile = file;
	}
	else
	{
		hud.pushMessage(red("Fail"));
	}
}

void Editor::saveAnimations(const std::string& file)
{
	string pathed_file = "models/" + file;
	hud.pushMessage("Saving animations to '" + pathed_file + "'");
	if(Animation::save(pathed_file))
	{
		hud.pushMessage(green("Success"));
		animationsFile = file;
	}
	else
	{
		hud.pushMessage(red("Fail"));
	}
}

void Editor::loadObjects(const string& file)
{
	if(file.empty())
		return;

	string pathed_file = "models/" + file;
	hud.pushMessage("Loading objects from '" + pathed_file + "'");
	if(ApoModel::loadObjects(pathed_file))
	{
		hud.pushMessage(green("Success"));
		objectsFile = file;
		selected_part = 0;
		editing_single_part = false;
	}
	else
	{
		hud.pushMessage(red("Fail"));
	}
}

void Editor::loadModel(const string& file)
{
	if(file.empty())
		return;

	string pathed_file = "models/" + file;
	hud.pushMessage("Loading model from '" + pathed_file + "'");

	ApoModel model;
	bool ok = model.load(pathed_file);
	if(ok)
	{
		release_swarm();

		selected_part = 0;
		editing_single_part = false;
		edited_model = model;
		hud.pushMessage(green("Success"));
		modelFile = file;
		skele = false;
	}
	else
	{
		hud.pushMessage(red("Fail"));
	}
}

void Editor::loadSkeletalModel(const string& file)
{
	if(file.empty())
		return;

	string pathed_file = "models/" + file;
	hud.pushMessage("Loading skeletal model from '" + pathed_file + "'");

	SkeletalModel model;
	bool ok = model.load(pathed_file);
	if(ok)
	{
		release_swarm();

		model.preload();
		selected_part = 0;
		editing_single_part = false;
		skeletal_model = model;
		skeletal_model.texture_name = "marine";
		hud.pushMessage(green("Success"));
		modelFile = file;
		skele = true;
	}
	else
	{
		hud.pushMessage(red("Fail"));
	}
}

void Editor::loadAnimations(const string& file)
{
	if(file.empty())
		return;

	string pathed_file = "models/" + file;
	hud.pushMessage("Loading animations from '" + pathed_file + "'");

	bool ok = Animation::load(pathed_file);
	if(ok)
	{
		hud.pushMessage(green("Success"));
		animationsFile = file;
	}
	else
	{
		hud.pushMessage(red("Fail"));
	}
}

void Editor::move_dot(double dx, double dy, double dz)
{
	current_dot.x += dx;
	current_dot.y += dy;
	current_dot.z += dz;
}

void Editor::move(double dx, double dy, double dz)
{
	if(editing_single_part)
	{
		move_dot(dx, dy, dz);
	}
	else
	{
		move_part(dx, dy, dz);
	}
}

void Editor::move_part(double dx, double dy, double dz)
{
	if(edited_model.parts.size() <= selected_part)
	{
		hud.pushMessage("Failed to move part, no selected part");
		return;
	}

	ModelNode& modelnode = edited_model.parts[selected_part];
	modelnode.offset_x += dx;
	modelnode.offset_y += dy;
	modelnode.offset_z += dz;

	stringstream ss;
	ss << "(" << dx << ", " << dy << ", " << dz << ")";
	hud.pushMessage(green("Moved " + modelnode.name + ss.str()));
}

void Editor::rotate_part(double dx, double dy, double dz)
{
	if(skele)
	{
		Bone& bone = skeletal_model.bones[selected_part];
		bone.rotation_x += dx;
		bone.rotation_y += dy;
		bone.rotation_z += dz;
	}
	else
	{
		if(edited_model.parts.size() <= selected_part)
		{
			hud.pushMessage("Failed to rotate part, no selected part");
			return;
		}
		ModelNode& modelnode = edited_model.parts[selected_part];
		modelnode.rotation_x += dx;
		modelnode.rotation_y += dy;
		modelnode.rotation_z += dz;

		stringstream ss;
		ss << "(" << dx << ", " << dy << ", " << dz << ")";
		hud.pushMessage(green("Rotated " + modelnode.name + ss.str()));
	}
}

void Editor::select_part(const string& part)
{
	if(skele)
	{
		for(size_t i = 0; i < skeletal_model.bones.size(); ++i)
		{
			if(skeletal_model.bones[i].name == part)
			{
				selected_part = i;
				hud.pushMessage(green("Selected part " + part));
				return;
			}
		}
		hud.pushMessage(red("Failed to select part " + part));
		return;
	}
	for(size_t i = 0; i < edited_model.parts.size(); ++i)
	{
		if(edited_model.parts[i].name == part)
		{
			if(selected_part < edited_model.parts.size())
			{
				edited_model.parts[selected_part].hilight = false;
			}

			selected_part = i;
			edited_model.parts[i].hilight = true;
			hud.pushMessage(green("Selected part " + part));
			return;
		}
	}
	hud.pushMessage(red("Failed to select part " + part));
}

void Editor::remove_part()
{
	if(edited_model.parts.size() <= selected_part)
	{
		hud.pushMessage("Failed to remove part, no selected part");
		return;
	}
	queue<size_t> to_be_removed;
	vector<bool> removed;
	vector<size_t> new_indices;
	removed.resize(edited_model.parts.size(), 0);
	new_indices.resize(edited_model.parts.size(), UINT_MAX);

	// Find all removed indices.
	to_be_removed.push(selected_part);
	while(!to_be_removed.empty())
	{
		size_t current = to_be_removed.front();
		to_be_removed.pop();
		removed[current] = true;
		for(size_t i = 0; i < edited_model.parts[current].children.size(); ++i)
		{
			to_be_removed.push(edited_model.parts[current].children[i]);
		}
	}

	if(removed[edited_model.root])
	{
		edited_model.root = -1;
	}

	// Do actual moving and resizing. Store new indices.
	size_t next = 0;
	for(size_t i = 0; i < edited_model.parts.size(); ++i)
	{
		if(!removed[i])
		{
			std::swap(edited_model.parts[i], edited_model.parts[next]);
			new_indices[i] = next;
			++next;
		}
	}
	edited_model.parts.resize(next);

	// Update children's indices.
	for(size_t i = 0; i < edited_model.parts.size(); ++i)
	{
		for(size_t j = 0; j < edited_model.parts[i].children.size(); ++j)
		{
			size_t& child = edited_model.parts[i].children[j];
			child = new_indices[child];
			if(child == UINT_MAX)
			{
				edited_model.parts[i].children.erase(edited_model.parts[i].children.begin() + j);
				--j;
			}
		}
	}
	selected_part = 0;
}

void Editor::add_part(const std::string& part_name, const std::string& type)
{
	if(!edited_model.parts.empty() && selected_part >= edited_model.parts.size())
	{
		hud.pushMessage(red("Add part failed, select part first"));
		return;
	}
	if(!type_exists(type))
	{
		hud.pushMessage(red("Add part failed, type '" + type + "' doesn't exist"));
		return;
	}
	ModelNode new_node;
	new_node.name = part_name;
	new_node.wireframe = type;
	new_node.offset_x = 0.0f;
	new_node.offset_y = 0.0f;
	new_node.offset_z = 0.0f;
	edited_model.parts.push_back(new_node);
	if(edited_model.parts.size() > 1)
	{
		ModelNode& selected_node = edited_model.parts[selected_part];
		selected_node.children.push_back(edited_model.parts.size()-1);
		hud.pushMessage(green("Added new part '" + part_name + "' of type '" + type + "' as child of '" + selected_node.name));
	}
	else
	{
		hud.pushMessage(green("Added new part '" + part_name + "' of type '" + type + "' as root"));
		edited_model.root = 1;
	}

}

void Editor::print_model()
{
	if(skele)
	{
		for(size_t i = 0; i < skeletal_model.bones.size(); ++i)
		{
			const Bone& bone = skeletal_model.bones[i];
			hud.pushMessage(bone.name);

			cerr << bone.name << ":\n";
			cerr << "    x: " << bone.start_x << "\n";
			cerr << "    y: " << bone.start_y << "\n";
			cerr << "    z: " << bone.start_z << "\n";
			cerr << "    rotx: " << bone.rotation_x << "\n";
			cerr << "    roty: " << bone.rotation_y << "\n";
			cerr << "    rotz: " << bone.rotation_z << "\n";
		}
	}
	else
	{
		for(size_t i = 0; i < edited_model.parts.size(); ++i)
		{
			hud.pushMessage(edited_model.parts[i].name);
		}
	}
}

void Editor::print_types()
{
	for(auto it = ApoModel::objects.begin(); it != ApoModel::objects.end(); ++it)
	{
		const std::string& name = it->first;
		hud.pushMessage(name);
	}
}

void Editor::print_animations()
{
	/*
	for(auto it = view.objects.begin(); it != view.objects.end(); ++it)
	{
		const std::string& type_name = it->first;
		for(auto it2 = it->second.animations.begin(); it2 != it->second.animations.end(); ++it2)
		{
			const std::string& animation_name = it2->first;
			hud.pushMessage(type_name + ": '" + animation_name + "'");
		}
	}
	*/
	/*
	for(auto it = edited_model.parts.begin(); it != edited_model.parts.end(); ++it)
	{
		const std::string& part_name = it->name;
		for(auto it2 = it->animations.begin(); it2 != it->animations.end(); ++it2)
		{
			const std::string& animation_name = it2->first;
			hud.pushMessage(part_name + ": '" + animation_name + "'");
		}
	}
	*/
}

void Editor::type_helper(const std::string& type)
{
	edited_type = &ApoModel::objects[type];
	edited_type_name = type;

	edited_type->end_x = 0.0f;
	edited_type->end_y = 0.0f;
	edited_type->end_z = 0.0f;

	stored_model = edited_model;
	ApoModel dummy;
	dummy.root = 0;
	dummy.currentModelPos = Vec3(0.0f, 0.0f, 0.0f);
	dummy.realUnitPos = Vec3(0.0f, 0.0f, 0.0f);
	dummy.animation_time = 0;
	ModelNode node;
	node.name = "dummy";
	node.wireframe = type;
	node.offset_x = 0.0f;
	node.offset_y = 0.0f;
	node.offset_z = 0.0f;
	dummy.parts.push_back(node);
	edited_model = dummy;

	editing_single_part = true;
	hud.pushMessage("Editing parttype '" + type + "'");
	selected_dot = edited_type->triangles.size() * 3 - 1;
}

void Editor::edit_type(const std::string& type)
{
	if(!type_exists(type))
	{
		hud.pushMessage(red("Edit part failed, type '" + type + "' doesn't exist"));
		return;
	}
	type_helper(type);
}

void Editor::add_type(const std::string& type)
{
	if(type_exists(type))
	{
		hud.pushMessage(red("Add part type failed, type '" + type + "' exists already"));
		return;
	}
	type_helper(type);
}

void Editor::edit_model()
{
	if(!editing_single_part)
	{
		hud.pushMessage(red("Already editing model."));
		return;
	}
	editing_single_part = false;
	edited_type = 0;
	edited_type_name = "";

	edited_model = stored_model;
}

void Editor::dot()
{
	if(!editing_single_part)
	{
		hud.pushMessage(red("dot works only when editing part types."));
		return;
	}
	new_dots.push_back(current_dot);

	if(new_dots.size() == 3)
	{
		stringstream ss;

		ObjectTri triangle;
		for(int i = 0; i < 3; ++i)
		{
			triangle.x[i] = new_dots[i].x;
			triangle.y[i] = new_dots[i].y;
			triangle.z[i] = new_dots[i].z;

			ss << fixed << setprecision(2) << "(" << triangle.x[i] << "," << triangle.y[i] << "," << triangle.z[i] << ")";
		}
		edited_type->triangles.push_back(triangle);
		selected_dot = edited_type->triangles.size() * 3 - 1;
		new_dots.clear();

		hud.pushMessage(green("Added new triangle: " + ss.str()));
	}
}

void Editor::next_dot()
{
	if(!editing_single_part)
	{
		hud.pushMessage(red("dot works only when editing part types."));
		return;
	}
	if(selected_dot < edited_type->triangles.size() * 3 - 1)
	{
		++selected_dot;
		size_t index = selected_dot / 3;
		size_t remainder = selected_dot % 3;
		current_dot.x = edited_type->triangles[index].x[remainder];
		current_dot.y = edited_type->triangles[index].y[remainder];
		current_dot.z = edited_type->triangles[index].z[remainder];
	}
}

void Editor::prev_dot()
{
	if(!editing_single_part)
	{
		hud.pushMessage(red("dot works only when editing part types."));
		return;
	}
	if(selected_dot > 0)
	{
		--selected_dot;
		size_t index = selected_dot / 3;
		size_t remainder = selected_dot % 3;
		current_dot.x = edited_type->triangles[index].x[remainder];
		current_dot.y = edited_type->triangles[index].y[remainder];
		current_dot.z = edited_type->triangles[index].z[remainder];
	}
}

void Editor::play_animation(const string& animation)
{
	if(skele)
	{
		skeletal_model.animation_name = animation;
	}
	else
	{
		edited_model.animation_name = animation;
	}
	for(auto it = models.begin(); it != models.end(); ++it)
	{
		it->second->animation_name = animation;
	}
	hud.pushMessage(green("Playing " + animation));
}

void Editor::record_animation(const string& animation)
{
	hud.pushMessage(green("Recording " + animation));
	animation_name = animation;
}

void Editor::record_step(size_t time)
{
	stringstream ss;
	ss << time;
	hud.pushMessage(green("Recorded step of length " + ss.str()));

	if(skele)
	{
		for(size_t i = 0; i < skeletal_model.bones.size(); ++i)
		{
			Bone& bone = skeletal_model.bones[i];
			Animation& animation = Animation::getAnimation(bone.name, animation_name);

			animation.insertAnimationState(time, bone.rotation_x, bone.rotation_y, bone.rotation_z);
	//		cerr << "Inserted animationstate for " << node.name << ": " << node.rotation_x << ", " << node.rotation_y << ", " << node.rotation_z << "\n";
		}
	}
	else
	{
		for(size_t i = 0; i < edited_model.parts.size(); ++i)
		{
			ModelNode& node = edited_model.parts[i];
			Animation& animation = Animation::getAnimation(node.name, animation_name);

			animation.insertAnimationState(time, node.rotation_x, node.rotation_y, node.rotation_z);
	//		cerr << "Inserted animationstate for " << node.name << ": " << node.rotation_x << ", " << node.rotation_y << ", " << node.rotation_z << "\n";
		}
	}
}

void Editor::reorder_triangles()
{
	if(!skele)
	{
		hud.pushMessage(red("Scale works only when editing skeletal models."));
		return;
	}

	for(size_t i = 0; i < skeletal_model.triangles.size(); ++i)
	{
		Triangle& triangle = skeletal_model.triangles[i];
		swap(triangle.vertices[0], triangle.vertices[2]);
	}

	hud.pushMessage(green("Reordered triangles"));
}

void Editor::scale(float scalar)
{
	if(!editing_single_part && !skele)
	{
		hud.pushMessage(red("Scale works only when editing part types or skeletal models."));
		return;
	}
	
	if(skele)
	{
		for(size_t i = 0; i < skeletal_model.vertices.size(); ++i)
		{
			Vec3& v = skeletal_model.vertices[i];
			v *= scalar;
		}

		stringstream ss;
		ss << scalar;
		hud.pushMessage(green("Scaled to " + ss.str()));
		return;
	}

	for(size_t i = 0; i < edited_type->triangles.size(); ++i)
	{
		ObjectTri& triangle = edited_type->triangles[i];
		for(size_t j = 0; j < 3; ++j)
		{
			triangle.x[j] *= scalar;
			triangle.y[j] *= scalar;
			triangle.z[j] *= scalar;
		}
	}
	edited_type->end_x *= scalar;
	edited_type->end_y *= scalar;
	edited_type->end_z *= scalar;

	stringstream ss;
	ss << scalar;
	hud.pushMessage(green("Scaled to " + ss.str()));
}


void Editor::handle_command(const string& command)
{
	hud.pushMessage(command);
	stringstream ss(command);

	string word1;
	string word2;
	string word3;
	string word4;
	ss >> word1;
	ss >> word2;
	ss >> word3;
	ss >> word4;


	if(word1 == "load")
	{
		if(word2 == "objects")
		{
			loadObjects(word3);
		}
		else if(word2 == "model")
		{
			loadModel(word3);
		}
		else if(word2 == "skele")
		{
			loadSkeletalModel(word3);
		}
		else if(word2 == "animations")
		{
			loadAnimations(word3);
		}
	}
	else if(word1 == "save")
	{
		if(word2 == "objects")
		{
			saveObjects(word3);
		}
		else if(word2 == "model")
		{
			saveModel(word3);
		}
		else if(word2 == "skele")
		{
			saveSkeletalModel(word3);
		}
		else if(word2 == "animations")
		{
			saveAnimations(word3);
		}
	}
	else if(word1 == "move")
	{
		stringstream ss(command);
		ss >> word1;
		double dx = 0.0;
		double dy = 0.0;
		double dz = 0.0;
		ss >> dx >> dy >> dz;
		move(dx,dy,dz);
	}
	else if(word1 == "rotate")
	{
		stringstream ss(command);
		ss >> word1;
		double dx = 0.0;
		double dy = 0.0;
		double dz = 0.0;
		ss >> dx >> dy >> dz;
		rotate_part(dx,dy,dz);
	}
	else if(word1 == "select")
	{
		select_part(word2);
	}
	else if(word1 == "add")
	{
		if(word2 == "part")
		{
			add_part(word3, word4);
		}
		else if(word2 == "type")
		{
			add_type(word3);
		}
	}
	else if(word1 == "remove")
	{
		remove_part();
	}
	else if(word1 == "edit")
	{
		if(word2 == "type")
		{
			edit_type(word3);
		}
		else if(word2 == "model")
		{
			edit_model();
		}
	}
	else if(word1 == "print")
	{
		if(word2 == "types")
		{
			print_types();
		}
		else if(word2 == "model")
		{
			print_model();
		}
		else if(word2 == "animations")
		{
			print_animations();
		}
	}
	else if(word1 == "speed")
	{
		stringstream ss1(command);
		ss1 >> word1 >> speed;
		stringstream ss2;
		ss2 << speed;
		hud.pushMessage("Move speed set to " + ss2.str());
	}
	else if(word1 == "rotate_speed")
	{
		stringstream ss1(command);
		ss1 >> word1 >> rotate_speed;
		stringstream ss2;
		ss2 << rotate_speed;
		hud.pushMessage("Rotate speed set to " + ss2.str());
	}
	else if(word1 == "dot")
	{
		dot();
	}
	else if(word1 == "next")
	{
		next_dot();
	}
	else if(word1 == "prev")
	{
		prev_dot();
	}
	else if(word1 == "play")
	{
		play_animation(word2);
	}
	else if(word1 == "record")
	{
		record_animation(word2);
	}
	else if(word1 == "record_step")
	{
		stringstream ss1(command);
		size_t time = 50;
		ss1 >> word1 >> time;
		record_step(time);
	}
	else if(word1 == "reset")
	{
		reset();
	}
	else if(word1 == "scale")
	{
		stringstream ss1(command);
		float scalar = 1.0f;
		ss1 >> word1 >> scalar;
		scale(scalar);
	}
	else if(word1 == "bones")
	{
		calculate_nearest_bones();
	}
	else if(word1 == "reorder")
	{
		reorder_triangles();
	}
	else if(word1 == "swarm")
	{
		int X = 1;
		int Y = 1;
		int Z = 1;
		stringstream ss1(command);
		ss1 >> word1;
		ss1 >> X >> Y >> Z;
		swarm(X, Y, Z);
	}
	else if(word1 == "particles")
	{
		int X = 1;
		int Y = 1;
		int Z = 1;
		stringstream ss1(command);
		ss1 >> word1;
		ss1 >> X >> Y >> Z;
		swarm_particles(X, Y, Z);
	}
	else if(word1 == "meadows")
	{
		int X = 1;
		int Z = 1;
		int y = 0;
		stringstream ss1(command);
		ss1 >> word1 >> X >> Z >> y;
		swarm_meadows(X, Z, y);
	}
	else if(word1 == "wind")
	{
		stringstream ss1(command);
		ss1 >> word1 >> wind.x >> wind.y >> wind.z;
		stringstream ss2;
		ss2 << wind;
		hud.pushMessage("Wind speed set to " + ss2.str());
	}
	else if(word1 == "p")
	{
		swarm_particles(3, 3, 3);
	}
	else if(word1 == "sky")
	{
		skybox = !skybox;
	}
	else if(word1 == "rgb")
	{
		stringstream ss1(command);
		ss1 >> word1;
		ss1 >> r >> g >> b;
	}
	else if(word1 == "MEADOW")
	{
		view.strVals["MEADOW"] = word2;
		hud.pushMessage("MEADOW set to " + word2);
	}
	else if(word1 == "goto")
	{
		Vec3 pos;
		stringstream ss1(command);
		ss1 >> word1;
		ss1 >> pos.x >> pos.y >> pos.z;
		visualworld.camera.setPosition(pos);
		hud.pushMessage("goto");
	}
	else
	{
		hud.pushMessage(red("command " + word1 + " not understood"));
	}

	commands.push_back(command);
	current_command = commands.size();
}

void Editor::reset()
{
	if(skele)
	{
		Bone& bone = skeletal_model.bones[selected_part];
		bone.rotation_x = 0.0f;
		bone.rotation_y = 0.0f;
		bone.rotation_z = 0.0f;
	}
	else
	{
		if(edited_model.parts.size() <= selected_part)
		{
			return;
		}

		ModelNode& modelnode = edited_model.parts[selected_part];
		modelnode.rotation_x = 0.0f;
		modelnode.rotation_y = 0.0f;
		modelnode.rotation_z = 0.0f;
	}
}

bool Editor::handle_input()
{
	string key = userio.getSingleKey();
//	cerr << key << "\n";

	static string clientCommand = "";
	static bool writing = false;
	static bool grabbed = false;

	if(key.size() != 0)
	{
		if(key == "pause")
		{
			window.screenshot();
		}
		if(key == "f3")
		{
			view.reload_shaders();
		}
		else if(key == "f4")
		{
			if(skele)
			{
				loadSkeletalModel(modelFile);
			}
			else
			{
				loadObjects(objectsFile);
				loadModel(modelFile);
			}
			loadAnimations(animationsFile);
		}
		else if(key == "f5")
		{
			reset();
		}
		else if(key == "f6")
		{
			drawing_model = !drawing_model;
		}
		else if(key == "f7")
		{
			drawing_skeleton = !drawing_skeleton;
		}
		else if(key == "f11")
		{
			view.toggleFullscreen();
		}
		else if(key == "f10")
		{
			view.toggleDebugStatus();
		}
		else if(key == "g")
		{
			if(grabbed)
			{
				window.disable_grab();
			}
			else
			{
				window.enable_grab();
			}
			grabbed = !grabbed;
		}
		
		if(writing)
		{
			string nick = ">";
			
			if(key.size() == 1)
			{
				clientCommand.append(key);
			}
			else if(key == "backspace" && clientCommand.size() > 0)
			{
				clientCommand.resize(clientCommand.size()-1);
			}
			else if(key == "space")
			{
				clientCommand.append(" ");
			}
			else if(key == "up")
			{
				if(current_command > 0)
				{
					--current_command;
					clientCommand = commands[current_command];
				}
			}
			else if(key == "down")
			{
				if(current_command < commands.size()-1)
				{
					++current_command;
					clientCommand = commands[current_command];
				}
				else if(current_command == commands.size()-1)
				{
					++current_command;
					clientCommand = "";
				}
			}

			if(key == "return")
			{
				writing = false;
				if(clientCommand.size() > 0)
				{
					handle_command(clientCommand);
				}

				clientCommand = "";
				hud.setCurrentClientCommand(clientCommand);
			}
			else if(key == "escape")
			{
				writing = false;
				clientCommand = "";
				hud.setCurrentClientCommand(clientCommand);
			}
			else
			{
				hud.setCurrentClientCommand("> " + clientCommand);
			}
		}
		else
		{
			if(key == "left")
			{
				prev_dot();
			}
			else if(key == "right")
			{
				next_dot();
			}
			else if(key == "enter")
			{
				dot();
			}
			else if(key == "insert")
			{
				rotate_part(0, 0, rotate_speed);
			}
			else if(key == "delete")
			{
				rotate_part(0, 0, -rotate_speed);
			}
			else if(key == "home")
			{
				rotate_part(rotate_speed, 0, 0);
			}
			else if(key == "end")
			{
				rotate_part(-rotate_speed, 0, 0);
			}
			else if(key == "page up")
			{
				rotate_part(0, rotate_speed, 0);
			}
			else if(key == "page down")
			{
				rotate_part(0, -rotate_speed, 0);
			}
			else if(key == "[8]")
			{
				move(0, 0, speed);
			}
			else if(key == "[5]")
			{
				move(0, 0, -speed);
			}
			else if(key == "[4]")
			{
				move(speed, 0, 0);
			}
			else if(key == "[6]")
			{
				move(-speed, 0, 0);
			}
			else if(key == "[9]")
			{
				move(0, speed, 0);
			}
			else if(key == "[3]")
			{
				move(0, -speed, 0);
			}
			else if(key == "return")
			{
				writing = true;
				hud.setCurrentClientCommand("> " + clientCommand);
			}
			else if(key == "escape")
			{
				cerr << "User pressed ESC, shutting down." << endl;
				return false;
			}
		}
	}

	int keystate = userio.getGameInput();
	int x, y;
	userio.getMouseChange(x, y);
	UserIO::MouseScrollStatus wheel_status = userio.getMouseWheelScrolled();
	if(wheel_status == UserIO::SCROLL_UP)
	{
		view.zoom_in();
	}
	if(wheel_status == UserIO::SCROLL_DOWN)
	{
		view.zoom_out();
	}
	visualworld.camera.updateInput(keystate);
	int sensitivity = 1000;
	x *= sensitivity;
	y *= sensitivity;
	dummy.updateInput(0, x, y, 0);
	return true;
}

struct BoneDistance
{
	float distance;
	size_t index;

	bool operator<(const BoneDistance& rhs) const
	{
		return distance < rhs.distance;
	}
};

void Editor::calculate_nearest_bones()
{
	for(size_t j = 0; j < skeletal_model.vertices.size(); ++j)
	{
		Vec3& vertex = skeletal_model.vertices[j];
		// Find out the closest child bone joint.
		vector<BoneDistance> bone_distances;
		for(size_t k = 0; k < skeletal_model.bones.size(); ++k)
		{
			Bone& bone = skeletal_model.bones[k];
			Vec3 bone_loc1(bone.start_x, bone.start_y, bone.start_z);
			Vec3 bone_loc2(bone.end_x, bone.end_y, bone.end_z);
			float distance1 = (vertex - bone_loc2).lengthSquared();
			float distance2 = (vertex - bone_loc2).lengthSquared();
			float distance = min(distance1, distance2);
			BoneDistance bone_distance = { distance, k };
			bone_distances.push_back(bone_distance);
		}
		sort(bone_distances.begin(), bone_distances.end());

		float dist1 = bone_distances[0].distance;
		float dist2 = bone_distances[1].distance;
		float sum = dist1 + dist2;
		float weight1 = (sum - dist1) / sum;
		float weight2 = (sum - dist2) / sum;
		size_t index1 = bone_distances[0].index;
		size_t index2 = bone_distances[1].index;
		if(weight1 > 0.65)
		{
			weight1 = 1.0;
			weight2 = 0.0;
			cerr << "Separated " << skeletal_model.bones[index1].name << " and " << skeletal_model.bones[index2].name << endl;
		}
	
		WeightedVertex& wv = skeletal_model.weighted_vertices[j];

		wv.bone1 = index1;
		wv.bone2 = index2;
		wv.weight1 = weight1;
		wv.weight2 = weight2;
		cerr << "Nearest: " << skeletal_model.bones[index1].name << " " << weight1
			<< ", second: " << skeletal_model.bones[index2].name << " " << weight2 << endl;
	}
	hud.pushMessage(green("Calculated nearest bones"));
}

void Editor::swarm(int X, int Y, int Z)
{
	release_swarm();
	float x_scalar = 5;
	float y_scalar = 6;
	float z_scalar = 4;
	for(int i = 0; i < X; ++i)
	{
		for(int j = 0; j < Y; ++j)
		{
			for(int k = 0; k < Z; ++k)
			{
				Model* model = (skele ? (Model*)new SkeletalModel(skeletal_model) : (Model*)new ApoModel(edited_model));
				model->realUnitPos.x = x_scalar * (i - X/2);
				model->currentModelPos.x = x_scalar * (i - X/2);
				model->realUnitPos.y = y_scalar * (j - Y/2);
				model->currentModelPos.y = y_scalar * (j - Y/2);
				model->realUnitPos.z = z_scalar * (k - Z/2);
				model->currentModelPos.z = z_scalar * (k - Z/2);
				model->texture_name = "marine";
				model->animation_time = (i+1)*(j+1)*(k+1);
				models[i*Y*Z + j*Z + k + 1] = model;
			}
		}
	}

	stringstream ss;
	ss << X*Y*Z;
	hud.pushMessage(red("SWARM! " + ss.str()));
}

void Editor::swarm_particles(int X, int Y, int Z)
{
	release_particles();
	float x_scalar = 7;
	float y_scalar = 7;
	float z_scalar = 10;
//	int lifetime = 50000;
	for(int i = 0; i < X; ++i)
	{
		for(int j = 0; j < Y; ++j)
		{
			for(int k = 0; k < Z; ++k)
			{
				Location place(x_scalar * (i - X/2), y_scalar * (j - Y/2), z_scalar * (k - Z/2));

				Location zero;
				visualworld.genParticleEmitter(place, zero, 25, 5000, 3500,  "WHITE", "ORANGE", "ORANGE", "DARK_RED", 1500, 10, 80);

//				Location direction(0,0,-1);
//				visualworld.genParticleEmitter(place, direction, lifetime, 20, 20,  r, g, b,    500, 5, 50);
			}
		}
	}

	stringstream ss;
	ss << X*Y*Z;
	hud.pushMessage(red("PARTICLES! " + ss.str()));
}

void Editor::swarm_meadows(int X, int Z, int y)
{
	meadows.bushes.clear();

	for(int i = 0; i < X; ++i)
	{
		for(int j = 0; j < Z; ++j)
		{
			float x = 1*(i-X/2) + 0.7 * rand() / RAND_MAX;
			float z = 1*(j-Z/2) + 0.7 * rand() / RAND_MAX;
			meadows.bushes.push_back(Vec3(x, y, z));
		}
	}
	meadows.unload();
	meadows.preload();
}

