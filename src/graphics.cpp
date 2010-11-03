#include "texturehandler.h"
#include "graphics.h"
#include "level.h"
#include "shaders.h"
#include "hud.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <algorithm>
#include <map>

using namespace std;

vector<pair<Vec3,Vec3> > LINES;
vector<Vec3> DOTS;

int TRIANGLES_DRAWN_THIS_FRAME = 0;
int QUADS_DRAWN_THIS_FRAME = 0;

std::map<std::string, ObjectPart> Graphics::objects;

void Graphics::depthSortParticles(Vec3& d)
{
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		viewParticles[i].updateDepthVal(d);
	}
	sort(viewParticles.begin(), viewParticles.end());
}


void Graphics::initLight()
{
	GLfloat light0ambient[ 4 ] = {0.f, 0.f,  0.f, 1.0f};
	GLfloat light0specular[ 4 ] = {1.0f, .4f,  .4f, 1.0f};
	GLfloat light0diffuse[ 4 ] = {0.8f, .3f,  .3f, 1.0f};
	
	glClearColor(0.0f,0.0f,0.0f,0.5f);
	glClearDepth(1.0f);
	
	for(int i=0; i<3; i++)
	{
		glLightf(GL_LIGHT0+i , GL_LINEAR_ATTENUATION, 0.f);
		glLightf(GL_LIGHT0+i , GL_QUADRATIC_ATTENUATION, 0.0006f);
		glLightfv(GL_LIGHT0+i, GL_AMBIENT, light0ambient);
		glLightfv(GL_LIGHT0+i, GL_DIFFUSE, light0diffuse);
		glLightfv(GL_LIGHT0+i, GL_SPECULAR, light0specular);

//		int POSITION = 0;
		int DIFFUSE = 1;
		stringstream ss;
		ss << "lvl_lights[" << i*2 + DIFFUSE << "]";
		glUniform4f(uniform_locations[ss.str()], light0diffuse[0], light0diffuse[1], light0diffuse[2], light0diffuse[3]);
	}
}

void Graphics::toggleWireframeStatus()
{
	drawDebugWireframe = !drawDebugWireframe;
}

void Graphics::toggleLightingStatus()
{
	if(lightsActive)
	{
		GLfloat global_ambient[ 4 ] = {0.f, 0.f,  0.f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
		drawDebuglines = false;
	}
	else
	{
		GLfloat global_ambient[ 4 ] = {0.6f, 0.6f,  0.6f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
		drawDebuglines = true;
	}

	lightsActive = !lightsActive;
}

void Graphics::genParticles(const Location& position, const Location& velocity, int num, float max_rand, float scale, float r, float g, float b)
{
	for(int i=0; i<num; i++)
	{
		Particle p;
		p.pos = position;
		p.vel = velocity;
		
		p.vel.x += FixedPoint(((rand() % 1000) - 500) * max_rand, 1000);
		p.vel.y += FixedPoint(((rand() % 1000) - 500) * max_rand, 1000);
		p.vel.z += FixedPoint(((rand() % 1000) - 500) * max_rand, 1000);
		
		p.max_life = 40 + (rand() % 30);
		p.cur_life = p.max_life;
		
		p.r = r * ((rand() % 40) + 80) / 100;
		p.g = g * ((rand() % 40) + 80) / 100;
		p.b = b * ((rand() % 40) + 80) / 100;
		
		p.scale = scale;
		
		viewParticles.push_back(p);
	}
	
}

void Graphics::megaFuck()
{
	int num_frames = 3;
	int anim_time = 2;
	// walking, step #1
	for(int i=0; i<num_frames; i++)
	{
		Animation::getAnimation("LEFT_LEG", "walk").insertAnimationState(anim_time, -30.f * i / num_frames, 0.f, 0.f);
		Animation::getAnimation("LEFT_FOOT", "walk").insertAnimationState(anim_time, 0.f, 0.f, 0.f);
		Animation::getAnimation("RIGHT_LEG", "walk").insertAnimationState(anim_time, -30.f * i / num_frames, 0.f, 0.f);
		Animation::getAnimation("RIGHT_FOOT", "walk").insertAnimationState(anim_time, 0.f, 0.f, 0.f);
	}
	
	// walking, step #2
	for(int i=0; i< 2 * num_frames; i++)
	{
		Animation::getAnimation("LEFT_LEG", "walk").insertAnimationState(anim_time, -30.f + 30.f * i / (2*num_frames), 0.f, 0.f);
		Animation::getAnimation("LEFT_FOOT", "walk").insertAnimationState(anim_time, +30.f * i / (2*num_frames), 0.f, 0.f);
		Animation::getAnimation("RIGHT_LEG", "walk").insertAnimationState(anim_time, -30.f + 30.f * i / (2*num_frames), 0.f, 0.f);
		Animation::getAnimation("RIGHT_FOOT", "walk").insertAnimationState(anim_time, +30.f * i / (2*num_frames), 0.f, 0.f);
	}
	
	// walking, step #3
	for(int i=0; i<num_frames; i++)
	{
		Animation::getAnimation("LEFT_LEG", "walk").insertAnimationState(anim_time, +40.f * i / num_frames, 0.f, 0.f);
		Animation::getAnimation("LEFT_FOOT", "walk").insertAnimationState(anim_time, +30.f, 0.f, 0.f);
		Animation::getAnimation("RIGHT_LEG", "walk").insertAnimationState(anim_time, +40.f * i / num_frames, 0.f, 0.f);
		Animation::getAnimation("RIGHT_FOOT", "walk").insertAnimationState(anim_time, +30.f, 0.f, 0.f);
	}
	
	// walking, step #4
	for(int i=0; i<num_frames; i++)
	{
		Animation::getAnimation("LEFT_LEG", "walk").insertAnimationState(anim_time, +40.f - 40.f * i / num_frames, 0.f, 0.f);
		Animation::getAnimation("LEFT_FOOT", "walk").insertAnimationState(anim_time, +30.f - 30.f * i / num_frames, 0.f, 0.f);
		Animation::getAnimation("RIGHT_LEG", "walk").insertAnimationState(anim_time, +40.f - 40.f * i / num_frames, 0.f, 0.f);
		Animation::getAnimation("RIGHT_FOOT", "walk").insertAnimationState(anim_time, +30.f - 30.f * i / num_frames, 0.f, 0.f);
	}
	Animation::getAnimation("LEFT_ARM", "walk").insertAnimationState(anim_time, 0.f, 0.f, 0.f);
	Animation::getAnimation("RIGHT_ARM", "walk").insertAnimationState(anim_time, 0.f, 0.f, 0.f);
	
	// there you go! now you know how to walk :D
	
	Animation::getAnimation("LEFT_LEG", "idle").insertAnimationState(anim_time, 0.f, 0.f, 0.f);
	Animation::getAnimation("LEFT_FOOT", "idle").insertAnimationState(anim_time, 0.f, 0.f, 0.f);
	Animation::getAnimation("RIGHT_LEG", "idle").insertAnimationState(anim_time, 0.f, 0.f, 0.f);
	Animation::getAnimation("RIGHT_FOOT", "idle").insertAnimationState(anim_time, 0.f, 0.f, 0.f);
	
	// when idle, SWING YOUR ARMS AROUND WILDLY :DD
	for(int i=0; i<4*num_frames; i++)
	{
		Animation::getAnimation("LEFT_ARM", "idle").insertAnimationState(4*anim_time, 360.f * i / (4 * num_frames), 0.f, 0.f);
		Animation::getAnimation("RIGHT_ARM", "idle").insertAnimationState(4*anim_time, 360.f * i / (4 * num_frames), 0.f, 0.f);
	}
	
	for(int i=0; i<4 * num_frames; i++)
	{
		Animation::getAnimation("HIP", "jump").insertAnimationState(4*anim_time, 0.f, 360.f * i / (4 * num_frames), 0.f);
	}

}


Graphics::Graphics()
{
	init();
}

Graphics::~Graphics()
{
	releaseShaders();
	destroyWindow();
}

void Graphics::init()
{
	createWindow(); // let SDL handle this part..
	
	initShaders();

	glLineWidth(3.0f);
	glEnable(GL_TEXTURE_2D);			// Enable Texture Mapping
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Clear The Background Color To Blue 
	glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);			// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);			// Enables Depth Testing
	glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				// Reset The Projection Matrix
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	initLight();
	
	// do some weird magic i dont understand
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	
	TextureHandler::getSingleton().createTexture("font", "data/fonts/font2.png");
	TextureHandler::getSingleton().createTexture("crosshair", "data/images/crosshair.png");
	TextureHandler::getSingleton().createTexture("particle", "data/images/particle.png");
	
	// these could be stored and set somewhere else possibly
	float angle = 100.f;
	float ratio = 800.f / 600.f;
	float nearP = 1.f;
	float farP  = 200.f;
	
	gluPerspective(angle,ratio,nearP,farP);
	frustum.setCamInternals(angle,ratio,nearP,farP);
	
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	drawDebuglines = false;
	drawDebugWireframe = false;
}

void Graphics::createWindow()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		cerr << "ERROR: SDL init failed." << endl;
		throw std::string("Unable to init SDL");
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	
	drawContext = SDL_SetVideoMode(800, 600, 0, SDL_OPENGL); // | SDL_FULLSCREEN);
	if(drawContext == 0)
	{
		cerr << "ERROR: drawContext = " << drawContext << endl;
		throw std::string("Unable to set SDL video mode");
	}
	else
	{
		cerr << "SUCCESS: Got a drawContext!" << endl;
	}
}

void Graphics::destroyWindow()
{
//	SDL_VideoQuit();
	SDL_Quit();
}

void Graphics::setCamera(const Camera& cam)
{
	camera = cam;
}

void drawNormal(const Level& lvl, int x, int z, int multiplier)
{
	Location n = lvl.getNormal(x, z) * 10;
	Location start;
	start.x = FixedPoint(int(x * multiplier));
	start.y = lvl.getVertexHeight(x, z);
	start.z = FixedPoint(int(z * multiplier));
	
	Location end = start + n;
	
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0); glVertex3f(start.x.getFloat(), start.y.getFloat(), start.z.getFloat());
	glColor3f(0.0, 1.0, 0.0); glVertex3f(end.x.getFloat(), end.y.getFloat(), end.z.getFloat());
	glEnd();
	
	glColor3f(1.0, 1.0, 1.0);
}


void Graphics::drawDebugHeightDots(const Level& lvl)
{
	int multiplier = 8;
	Vec3 points[3];

	// this should only be done at world ticks! not at draw ticks..
	level_triangles.clear();
	lvl.btt.getTriangles(level_triangles);

	// Draw triangles with lines.
	glPointSize(1.0f);
	glColor3f(0,0,1);
	glBegin(GL_LINES);
	for(size_t k=0; k<level_triangles.size(); k++)
	{
		BTT_Triangle& tri = level_triangles[k];
		for(size_t i = 0; i < 3; ++i)
		{
			points[i].x = tri.points[i].x * multiplier;
			points[i].z = tri.points[i].z * multiplier;
			points[i].y = lvl.getVertexHeight(tri.points[i].x, tri.points[i].z).getFloat();
		}
		
		for(size_t i = 0; i < 3; ++i)
		{
			glVertex3f(points[i].x, points[i].y, points[i].z);
			int next = (i+1+3) % 3;
			glVertex3f(points[next].x, points[next].y, points[next].z);
		}
	}
	glEnd();

	// Draw heights with dots.
	glColor3f(1,0,0);
	glPointSize(2.0f);
	glBegin(GL_POINTS);
	for(size_t x = 0; x < 500; x += 2)
	{
		for(size_t z = 0; z < 1000; z += 2)
		{
			Vec3 v(x, 0, z);
			v.y = lvl.getHeight(x,z).getFloat();
			glVertex3f(v.x, v.y, v.z);
		}
	}
	glEnd();
	glColor3f(0,1,0);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	for(int x = 0; x < lvl.max_x(); x += 8)
	{
		for(int z = 0; z < lvl.max_z(); z += 8)
		{
			Vec3 v(x, 0, z);
			v.y = lvl.getHeight(x,z).getFloat();
			glVertex3f(v.x, v.y, v.z);
		}
	}
	glEnd();
}



void Graphics::updateLights(const std::map<int, LightObject>& lightsContainer)
{
	while(glGetError() != GL_NO_ERROR); // Clear error flags.

	glUseProgram(shaders["level_program"]);
	int i=0;
	for(auto iter = lightsContainer.begin(); iter != lightsContainer.end(); iter++)
	{
		int POSITION = 0;
		int DIFFUSE = 1;

		float rgb[4]; rgb[3] = 1.0f;
		
		iter->second.getDiffuse(rgb[0], rgb[1], rgb[2]);
//		glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, rgb);
		stringstream ss1;
		ss1 << "lvl_lights[" << i*2 + DIFFUSE << "]";
		glUniform4f(uniform_locations[ss1.str()], rgb[0], rgb[1], rgb[2], rgb[3]);
		GLenum error;
		if((error = glGetError()) != GL_NO_ERROR)
		{
			cerr << "glUniform4f failed: " << gluErrorString(error) << " at line " << __LINE__ << "\n";
		}
//		cerr << uniform_locations[ss1.str()] << " is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;
//		cerr << "DIFFUSE is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;

		const Location& pos = iter->second.getPosition();
		rgb[0] = pos.x.getFloat();
		rgb[1] = pos.y.getFloat();
		rgb[2] = pos.z.getFloat();
		
//		glLightfv(GL_LIGHT0 + i, GL_POSITION, rgb);

		stringstream ss2;
		ss2 << "lvl_lights[" << i*2 + POSITION << "]";
		glUniform4f(uniform_locations[ss2.str()], rgb[0], rgb[1], rgb[2], rgb[3]);
		if((error = glGetError()) != GL_NO_ERROR)
		{
			cerr << "glUniform4f failed: " << gluErrorString(error) << " at line " << __LINE__ << "\n";
		}
//		cerr << uniform_locations[ss2.str()]  << " is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;
//		cerr << "POSITION  is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;
		
		++i;
		if(i >= MAX_NUM_LIGHTS)
		{
			// if there are too many lights, just ignore the rest of them
			break;
		}
	}
	glUseProgram(0);
}

void Graphics::drawDebugLevelNormals(const Level& lvl)
{
	int multiplier = 8;
	Vec3 points[3];

	glBegin(GL_LINES);
	for(size_t k=0; k<level_triangles.size(); k++)
	{
		BTT_Triangle& tri = level_triangles[k];
		for(size_t i = 0; i < 3; ++i)
		{
			points[i].x = tri.points[i].x * multiplier;
			points[i].z = tri.points[i].z * multiplier;
			points[i].y = lvl.getVertexHeight(tri.points[i].x, tri.points[i].z).getFloat();
		}
		
		Location n;
		
		n = lvl.getNormal(tri.points[0].x, tri.points[0].z) * 10;
		Location start;
		start.x = FixedPoint(int(points[0].x));
		start.y = lvl.getVertexHeight(tri.points[0].x, tri.points[0].z);
		start.z = FixedPoint(int(points[0].z));
		
		Location end = start + n;
		
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(start.x.getFloat(), start.y.getFloat(), start.z.getFloat());
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(end.x.getFloat(), end.y.getFloat(), end.z.getFloat());
	}
	glEnd();
}

struct LightDistance
{
	int index;
	FixedPoint squaredDistance;

	bool operator<(const LightDistance& rhs) const
	{
		return squaredDistance < rhs.squaredDistance;
	}
};

void Graphics::setActiveLights(const map<int, LightObject>& lightsContainer, const Location& pos)
{
	static vector<LightDistance> distances;
	distances.resize( max(lightsContainer.size(), distances.size()) );
	int i = 0;
	for(auto iter = lightsContainer.begin(); iter != lightsContainer.end(); ++iter, ++i)
	{
		FixedPoint lightDistance = (iter->second.position - pos).lengthSquared();
		distances[i].index = i;
		distances[i].squaredDistance = lightDistance;
	}
//	sort(distances.begin(), distances.end());
	size_t k = min(size_t(MAX_NUM_ACTIVE_LIGHTS), distances.size());
	nth_element(distances.begin(), distances.begin() + k, distances.begin() + lightsContainer.size());
	assert(MAX_NUM_ACTIVE_LIGHTS == 4);
	glVertexAttrib4f(uniform_locations["lvl_activeLights"], distances[0].index, distances[1].index, distances[2].index, distances[3].index);
}

struct ActiveLights
{
	float active_light0;
	float active_light1;
	float active_light2;
	float active_light3;
};

void Graphics::drawLevel(const Level& lvl, const map<int, LightObject>& lightsContainer)
{
	glUseProgram(shaders["level_program"]);
	TextureHandler::getSingleton().bindTexture("grass2048");
	//TextureHandler::getSingleton().bindTexture("chessboard");
	
	// set ambient light
	if(drawDebuglines)
		glUniform4f(uniform_locations["lvl_ambientLight"], 0.4f, 0.4, 0.4f, 1.f);
	else
		glUniform4f(uniform_locations["lvl_ambientLight"], 0.1f, 0.1f, 0.1f, 1.0f);
	
	const int multiplier = 8;

	static vector<Vec3> vertices;
	static vector<Vec3> normals;
	static vector<TextureCoordinate> texture_coordinates;
	static vector<ActiveLights> active_lights;

	size_t height = lvl.pointheight_info.size();
	size_t width = lvl.pointheight_info[0].size();
	const int BUFFERS = 5;
	static GLuint locations[BUFFERS];
	static bool level_buffers_loaded = false;

	// Load static buffers.
	if(!level_buffers_loaded) // TODO: Move initialization somewhere else?
	{
		level_buffers_loaded = true;
		assert(height*width > 0);

		vertices.reserve(height * width);
		normals.reserve(height * width);
		for(size_t x = 0; x < height; ++x)
		{
			for(size_t z = 0; z < width; ++z)
			{
				Vec3 point(x*8, lvl.getVertexHeight(x, z).getFloat(), z*8);
				vertices.push_back(point);

				Location normal = lvl.getNormal(x, z);
				normals.push_back(Vec3(normal.x.getFloat(), normal.y.getFloat(), normal.z.getFloat()));

				 // TODO: These coordinates are like :G
				const int divisions = 10;
				TextureCoordinate tc1 = { float(x % (height/divisions)) / (height/divisions), float(z % (width/divisions)) / (width/divisions) };
				texture_coordinates.push_back(tc1);

				ActiveLights ac = { 0, 0, 0, 0};
				active_lights.push_back(ac);
			}
		}

		glGenBuffers(BUFFERS, locations);

		glBindBuffer(GL_ARRAY_BUFFER, locations[0]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[1]);
		glBufferData(GL_ARRAY_BUFFER, texture_coordinates.size() * sizeof(TextureCoordinate), &texture_coordinates[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[2]);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(Vec3), &normals[0], GL_STATIC_DRAW);
	}

	// Load dynamic indices.
	vector<unsigned> indices;
	for(size_t k=0; k<level_triangles.size(); k++)
	{
		Vec3 points[3];
		BTT_Triangle& tri = level_triangles[k];
		for(size_t i = 0; i < 3; ++i)
		{
			points[i].x = tri.points[i].x * multiplier;
			points[i].z = tri.points[i].z * multiplier;
			points[i].y = lvl.getVertexHeight(tri.points[i].x, tri.points[i].z).getFloat();

			indices.push_back( tri.points[i].x * width + tri.points[i].z  );
		}
		
		Vec3 semiAverage = (points[0] + points[1] + points[2]) / 3;
		
		// Set active lights
		Location pos(int(semiAverage.x), int(semiAverage.y), int(semiAverage.z));

		static vector<LightDistance> distances;
		distances.resize( max(lightsContainer.size(), distances.size()) );
		int i = 0;
		for(auto iter = lightsContainer.begin(); iter != lightsContainer.end(); ++iter, ++i)
		{
			FixedPoint lightDistance = (iter->second.position - pos).lengthSquared();
			distances[i].index = i;
			distances[i].squaredDistance = lightDistance;
		}

		assert(MAX_NUM_ACTIVE_LIGHTS <= 4);
	//	sort(distances.begin(), distances.end());
		size_t k = min(size_t(MAX_NUM_ACTIVE_LIGHTS), distances.size());
		nth_element(distances.begin(), distances.begin() + k, distances.begin() + lightsContainer.size());
		ActiveLights ac = { float(distances[0].index), float(distances[1].index), float(distances[2].index), float(distances[3].index) };

		for(size_t i = 0; i < 3; ++i)
		{
			size_t index = tri.points[i].x * width + tri.points[i].z;
			active_lights[index] = ac; // TODO: now every vertex gets active lights only from a single face.
		}
	}


	assert(active_lights.size() == vertices.size());
	assert(texture_coordinates.size() == vertices.size());
	assert(normals.size() == vertices.size());
/*
	// Draw data.
	glBegin(GL_TRIANGLES);
	for(size_t k = 0; k < indices.size(); k += 3)
	{
		Vec3 points[3];
		points[0] = vertices[indices[k+0]];
		points[1] = vertices[indices[k+1]];
		points[2] = vertices[indices[k+2]];

		ActiveLights& lol = active_lights[indices[k+0]];
		glVertexAttrib4f(uniform_locations["lvl_activeLights"], lol.active_light0, lol.active_light1, lol.active_light2, lol.active_light3);

		// TODO: Terrain texture
		for(size_t i = 0; i < 3; ++i)
		{
			Vec3 n = normals[indices[k+i]];
			glNormal3f(n.x, n.y, n.z);
			glTexCoord2f(texture_coordinates[indices[k+i]].x, texture_coordinates[indices[k+i]].y);
			glVertex3f( points[i].x, points[i].y, points[i].z );
		}
		
		++TRIANGLES_DRAWN_THIS_FRAME;
	}
	glEnd();
*/

	// Bind static data and send dynamic data to graphics card.
	glBindBuffer(GL_ARRAY_BUFFER, locations[0]);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, locations[1]);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, locations[2]);
	glNormalPointer(GL_FLOAT, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, locations[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ActiveLights) * active_lights.size(), &active_lights[0], GL_STREAM_DRAW);
	glVertexAttribPointer(uniform_locations["lvl_activeLights"], 4, GL_FLOAT, GL_FALSE, sizeof(ActiveLights), 0);

	// Draw sent data.
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableVertexAttribArray(uniform_locations["lvl_activeLights"]);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glDisableVertexAttribArray(uniform_locations["lvl_activeLights"]);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	TRIANGLES_DRAWN_THIS_FRAME += level_triangles.size();

	glUseProgram(0);
}

void Graphics::drawDebugLines()
{
	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	for(size_t i = 0; i < LINES.size(); ++i)
	{
		Vec3& p1 = LINES[i].first;
		Vec3& p2 = LINES[i].second;
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
	}
	glEnd();

	glPointSize(5.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	for(size_t i = 0; i < DOTS.size(); ++i)
	{
		Vec3& p1 = DOTS[i];
		glVertex3f(p1.x, p1.y, p1.z);
	}
	glEnd();
}

void Graphics::drawModels(map<int, Model*>& models)
{
	if(lightsActive)
	{
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
	glUseProgram(shaders["unit_program"]);
	for(map<int, Model*>::iterator iter = models.begin(); iter != models.end(); ++iter)
	{
		Model& model = *iter->second;

		glUniform4f(unit_color_location, 0.7, 0.7, 0.7, 0.5);
		
		if(frustum.sphereInFrustum(model.currentModelPos, 5) != FrustumR::OUTSIDE)
		{
			glTranslatef(model.currentModelPos.x, model.currentModelPos.y, model.currentModelPos.z);
			model.draw();
			glTranslatef(-model.currentModelPos.x, -model.currentModelPos.y, -model.currentModelPos.z);
		}
	}
	glUseProgram(0);
}

void Graphics::drawParticles()
{
	Vec3 direction_vector = camera.getTarget() - camera.getPosition();
	depthSortParticles(direction_vector);
	
	TextureHandler::getSingleton().bindTexture("particle");
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glDepthMask(GL_FALSE); // dont write to depth buffer.
	
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		float px = viewParticles[i].pos.x.getFloat();
		float py = viewParticles[i].pos.y.getFloat();
		float pz = viewParticles[i].pos.z.getFloat();
		
		glColor4f(viewParticles[i].r, viewParticles[i].g, viewParticles[i].b, viewParticles[i].getAlpha());
		
		float x_angle = camera.getXrot();
		float y_angle = -camera.getYrot() - 90.f;
		
		glPushMatrix();
		
		glTranslatef(px, py, pz);
		glRotatef(x_angle, 0.0, 1.0, 0.0);
		glRotatef(y_angle, 1.0, 0.0, 0.0);
		
		float s = viewParticles[i].scale;
		
		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 1.f); glVertex3f(-1.5f * s, +1.5f * s, 0.0f);
		glTexCoord2f(1.f, 1.f); glVertex3f(+1.5f * s, +1.5f * s, 0.0f);
		glTexCoord2f(1.f, 0.f); glVertex3f(+1.5f * s, -1.5f * s, 0.0f);
		glTexCoord2f(0.f, 0.f); glVertex3f(-1.5f * s, -1.5f * s, 0.0f);
		glEnd();
		++QUADS_DRAWN_THIS_FRAME;
		
		glPopMatrix();
	}
	
	glDepthMask(GL_TRUE); // re-enable depth writing.
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

void Graphics::updateCamera(const Level& lvl)
{
	Vec3 camStartPos = camera.getPosition();
	FixedPoint camX = FixedPoint(camStartPos.x);
	FixedPoint camZ = FixedPoint(camStartPos.z);
	
	float cam_min_y = lvl.getHeight(camX, camZ).getFloat() + 3.f;
	camera.setAboveGround(cam_min_y);
}

void Graphics::startDrawing()
{
	glMatrixMode(GL_MODELVIEW);
	
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
	
	Vec3 camPos, camTarget, upVector;
	camPos = camera.getPosition();
	camTarget = camera.getTarget();

	upVector.x = 0.f;
	upVector.y = 1.f;
	upVector.z = 0.f;
	
	glLoadIdentity();
	gluLookAt(camPos.x, camPos.y, camPos.z,
			  camTarget.x, camTarget.y, camTarget.z,
			  upVector.x, upVector.y, upVector.z);
			  
	frustum.setCamDef(camPos, camTarget, upVector);

	TRIANGLES_DRAWN_THIS_FRAME = 0;
	QUADS_DRAWN_THIS_FRAME = 0;
}

void Graphics::draw(
	map<int, Model*>& models,
	const Level& lvl,
	const std::map<int,Unit>& units,
	const std::map<int, LightObject>& lights,
	const std::shared_ptr<Octree> o, Hud* hud,
	const std::map<int, Medikit>& medikits,
	const std::map<int, Projectile>& projectiles
	)
{
	updateCamera(lvl);

	startDrawing();

	if(drawDebuglines)
	{
		drawDebugLevelNormals(lvl);
		drawDebugProjectiles(projectiles);
	}

	if(drawDebugWireframe)
	{
		drawDebugHeightDots(lvl);
	}
	else
	{
		drawLevel(lvl, lights);
	}
	drawDebugLines();
	drawMedikits(medikits);
	drawBoundingBoxes(units);
	drawModels(models);
	drawParticles();
	drawOctree(o);
	
	if(hud)
	{
		hud->setMinimap(-camera.getXrot(), camera.getUnitLocation());
		hud->draw(camera.isFirstPerson());
	}

	finishDrawing();
}

void Graphics::finishDrawing()
{
	SDL_GL_SwapBuffers();
}

void Graphics::drawMedikits(const std::map<int, Medikit>& medikits) {
	for (auto it = medikits.begin(); it != medikits.end(); ++it) {
		Medikit kit = it->second;
		drawBox(kit.bb_top(), kit.bb_bot());
	}
}

void Graphics::updateInput(int keystate)
{
	camera.updateInput(keystate);
}

void Graphics::bindCamera(Unit* unit)
{
	camera.bind(unit, Camera::RELATIVE);
}

void Graphics::updateParticles()
{
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		viewParticles[i].tick();
		if(!viewParticles[i].alive())
		{
			viewParticles[i] = viewParticles.back();
			viewParticles.pop_back();
			--i;
		}
	}
}

void Graphics::world_tick(Level& lvl, const std::map<int, LightObject>& lights)
{
	// Don't draw anything here!
	updateParticles();
	
	Location pos;
	
	// position and frustum
	lvl.splitBTT(pos, frustum);
	
	level_triangles.clear();
	lvl.btt.getTriangles(level_triangles);
	// cerr << "total triangles: " << level_triangles.size() << endl;
	
	updateLights(lights);
}

void Graphics::tick()
{
	camera.tick();
}

void Graphics::toggleFullscreen()
{
	SDL_WM_ToggleFullScreen(drawContext);
}

void Graphics::mouseUp()
{
	camera.zoomIn();
}

void Graphics::mouseDown()
{
	camera.zoomOut();
}

void Graphics::drawBox(const Location& top, const Location& bot,
	GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	glColor4f(r, g, b, a);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	glVertex3f(top.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), top.z.getFloat());

	glVertex3f(top.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), top.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());

	glVertex3f(top.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(bot.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), bot.z.getFloat());
	glVertex3f(top.x.getFloat(), bot.y.getFloat(), top.z.getFloat());
	glEnd();
}

void Graphics::drawOctree(const std::shared_ptr<Octree>& o) {
	if (!drawDebuglines)
		return;
	if (o->n > 0)
		drawBox(o->top, o->bot, 1.0f);
	if (o->hasChildren) {
		for (int i = 0; i < 2; ++i)
			for (int j = 0; j < 2; ++j)
				for (int k = 0; k < 2; ++k)
					drawOctree(o->children[i][j][k]);
	}
}

void Graphics::drawBoundingBoxes(const std::map<int,Unit>& units)
{
	if (!drawDebuglines)
		return;
	for(auto iter = units.begin(); iter != units.end(); iter++)
	{
		drawBox(iter->second.bb_top(), iter->second.bb_bot());
	}
}

void Graphics::drawDebugProjectiles(const std::map<int, Projectile>& projectiles)
{
	glColor3f(0.3, 0.7, 0.5);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	for(auto it = projectiles.begin(); it != projectiles.end(); ++it)
	{
		const MovableObject& proj = it->second;
		glVertex3f(proj.position.x.getFloat(), proj.position.y.getFloat(), proj.position.z.getFloat());
	}
	glEnd();
}

