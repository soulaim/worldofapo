#include "editor/editor.h"
#include "graphics/texturehandler.h"
#include "graphics/grasscluster.h"
#include "misc/timer.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <climits>
#include <locale>
#include <iomanip>
#include <set>

using namespace std;

extern vector<pair<vec3<float>,vec3<float> > > LINES;
extern vector<vec3<float> > DOTS;
GrassCluster meadows;
vec3<float> wind;

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

LINES.push_back(make_pair(vec3<float>(100,0,0),vec3<float>(-100,0,0)));
LINES.push_back(make_pair(vec3<float>(0,100,0),vec3<float>(0,-100,0)));
LINES.push_back(make_pair(vec3<float>(0,0,100),vec3<float>(0,0,-100)));

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
	view.intVals["HELP"] = 0;
	
	if(!handle_input())
	{
		return false;
	}

	string message;

	if(editing_single_part)
	{
		/*
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
		*/
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
			view.drawModels(models, visualworld.camera);
		}
	}

	view.drawDebugLines();
	
	/*
	static int counter = 0;
	++counter;
	winds.resize(meadows.size());
	for(size_t i = 0; i < winds.size(); ++i)
	{
		vec3<float>& w = winds[i];
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
	long long ticks = Timer::time_now();
	static long long last_tick = -999999;
	static long long world_ticks = 0;

	long long time_since_last = ticks - last_tick;
	long long time_between_ticks = 1000/50;
	if(time_since_last >= time_between_ticks)
	{
		++world_ticks;
		last_tick = ticks;

		hud.setTime( ticks );
		view.tick();
		
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
		Timer::sleep(time_between_ticks - time_since_last);
		return true;
	}
}

bool Editor::type_exists(const std::string&)
{
	return false;
}

void Editor::saveModel(const std::string&)
{
	/*
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
	*/
	
	hud.pushMessage(red("Fail"));
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

void Editor::saveObjects(const std::string&)
{
	hud.pushMessage("^rApoModels are obsolete. Don't try to save one plz.");
	
	/*
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
	*/
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

void Editor::loadObjects(const string&)
{
	hud.pushMessage(red("Fail"));
}

void Editor::loadModel(const string&)
{
	hud.pushMessage(red("Fail"));
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





void Editor::play_animation(const string& animation)
{
	if(skele)
	{
		skeletal_model.animation_name = animation;
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
			vec3<float>& v = skeletal_model.vertices[i];
			v *= scalar;
		}

		stringstream ss;
		ss << scalar;
		hud.pushMessage(green("Scaled to " + ss.str()));
		return;
	}
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
	else if(word1 == "print")
	{
		if(word2 == "model")
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
		vec3<float> pos;
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
		if(key == "f1")
		{
			visualworld.camera.setMode(Camera::FollowMode::STATIC);
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
			if(key == "return")
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
	visualworld.camera.updateInput(keystate, x, y);
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
		vec3<float>& vertex = skeletal_model.vertices[j];
		// Find out the closest child bone joint.
		vector<BoneDistance> bone_distances;
		for(size_t k = 0; k < skeletal_model.bones.size(); ++k)
		{
			Bone& bone = skeletal_model.bones[k];
			vec3<float> bone_loc1(bone.start_x, bone.start_y, bone.start_z);
			vec3<float> bone_loc2(bone.end_x, bone.end_y, bone.end_z);
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
				Model* model = (Model*)new SkeletalModel(skeletal_model);
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
			meadows.bushes.push_back(vec3<float>(x, y, z));
		}
	}
	meadows.unload();
	meadows.preload();
}

