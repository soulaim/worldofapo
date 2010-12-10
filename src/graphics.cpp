#include "graphics.h"

#include "texturehandler.h"
#include "level.h"
#include "shaders.h"
#include "hud.h"
#include "frustum/matrix4.h"
#include "octree.h"
#include "texturecoordinate.h"
#include "window.h"
#include "menubutton.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <algorithm>
#include <numeric>
#include <map>

using namespace std;

vector<pair<Vec3,Vec3> > LINES;
vector<Vec3> DOTS;
vector<pair<Vec3,string> > STRINGS;

int TRIANGLES_DRAWN_THIS_FRAME = 0;
int QUADS_DRAWN_THIS_FRAME = 0;

bool near(const Camera* camera, const Vec3& position)
{
	float dist = 100.0;
	return (camera->getPosition() - position).lengthSquared() < dist * dist;
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

float isLightUsed(LightDistance& light)
{
	if(light.squaredDistance > 1000)
		return -1.f;
	else
		return light.index;
}

void Graphics::depthSortParticles(Vec3& d, vector<Particle>& viewParticles)
{
	return;
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		viewParticles[i].updateDepthVal(d);
	}
	sort(viewParticles.begin(), viewParticles.end());
}


void Graphics::initLight()
{
	glDisable(GL_LIGHTING);

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
		glUniform4f(shaders.uniform(ss.str()), light0diffuse[0], light0diffuse[1], light0diffuse[2], light0diffuse[3]);
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

	cerr << "Lightsactive: " << lightsActive << endl;
}

Graphics::Graphics(Window& w, Hud& h):
	window(w),
	hud(h)
{
	cerr << "Loading config file for Graphics.." << endl;
	load("graphics.conf");
	
	window.createWindow(intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
}

Graphics::~Graphics()
{
}

void Graphics::init(Camera& camera)
{
	cerr << "Graphics::init()" << endl;

	shaders.init();
	
	camera_p = &camera;

	glLineWidth(3.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Set the background color.
	glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);			// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				// Reset The Projection Matrix
	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	MAX_NUM_LIGHTS = 71;
	MAX_NUM_ACTIVE_LIGHTS = 1; // Make sure this is the same number as in the shaders.

	glUseProgram(shaders["particle_program"]);
	glUniform1f(shaders.uniform("particle_screen_width"),  intVals["RESOLUTION_X"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
	glUniform1f(shaders.uniform("particle_screen_height"), intVals["RESOLUTION_Y"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
	glUseProgram(0);

	// TODO: This is completely obsolete?
	initLight();
	
	
	// Init screen framebuffer objects
	glGenFramebuffers(1, &screenFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
	TextureHandler::getSingleton().createTexture("screenFBO_texture", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	TextureHandler::getSingleton().createDepthTexture("screenFBO_depth_texture", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("screenFBO_texture"), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("screenFBO_depth_texture"), 0);
	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("particlesFBO_texture"), 0);

//	TextureHandler::getSingleton().createTexture("screenFBO_texture2", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("screenFBO_texture2"), 0);

	cerr << "screenFBO status: " << gluErrorString(glCheckFramebufferStatus(screenFBO)) << endl;

	glGenFramebuffers(1, &postFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
	TextureHandler::getSingleton().createTexture("postFBO_texture", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("postFBO_texture"), 0);
	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("screenFBO_depth_texture"), 0);
	
	glGenFramebuffers(1, &particlesFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, particlesFBO);
	TextureHandler::getSingleton().createTexture("particlesFBO_texture", intVals["RESOLUTION_X"] / intVals["PARTICLE_RESOLUTION_DIVISOR"], intVals["RESOLUTION_Y"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("particlesFBO_texture"), 0);
	
	glGenFramebuffers(1, &particlesUpScaledFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, particlesUpScaledFBO);
	TextureHandler::getSingleton().createTexture("particlesUpScaledFBO_texture", intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandler::getSingleton().getTextureID("particlesUpScaledFBO_texture"), 0);
	
	/*
	glGenRenderbuffers(1, &screenRB);
	glBindRenderbuffer(GL_RENDERBUFFER, screenRB);
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 800, 600);
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, screenRB);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	*/
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// ------------------------------
	
	// do some weird magic i dont understand
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	
	TextureHandler::getSingleton().createTexture("font", "data/fonts/font2.png");
	TextureHandler::getSingleton().createTexture("particle", "data/images/particle.png");
	
	camera_p->aspect_ratio = float(intVals["RESOLUTION_X"]) / float(intVals["RESOLUTION_Y"]);
	gluPerspective(camera_p->fov, camera_p->aspect_ratio, camera_p->nearP, camera_p->farP);
	frustum.setCamInternals(camera_p->fov, camera_p->aspect_ratio, camera_p->nearP, camera_p->farP);
	
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	
	lightsActive = false;
	drawDebuglines = false;
	drawDebugWireframe = false;
}

void drawNormal(const Level& lvl, int x, int z)
{
	Location n = lvl.getNormal(x, z) * 10;
	Location start;
	start.x = FixedPoint(x * Level::BLOCK_SIZE);
	start.y = lvl.getVertexHeight(x, z);
	start.z = FixedPoint(z * Level::BLOCK_SIZE);
	
	Location end = start + n;
	
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0); glVertex3f(start.x.getFloat(), start.y.getFloat(), start.z.getFloat());
	glColor3f(0.0, 1.0, 0.0); glVertex3f(end.x.getFloat(), end.y.getFloat(), end.z.getFloat());
	glEnd();
	
	glColor3f(1.0, 1.0, 1.0);
}


void Graphics::drawDebugHeightDots(const Level& lvl)
{
	TextureHandler::getSingleton().bindTexture(0, "");

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
			points[i].x = tri.points[i].x * Level::BLOCK_SIZE;
			points[i].z = tri.points[i].z * Level::BLOCK_SIZE;
			points[i].y = lvl.getVertexHeight(tri.points[i].x, tri.points[i].z).getFloat();
		}
		
		// TODO: this could also be done by loading a geometry shader that transforms a polygon into three lines.
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
	for(FixedPoint x; x < lvl.max_x(); x += 2)
	{
		for(FixedPoint z; z < lvl.max_z(); z += 2)
		{
			Vec3 v(x.getFloat(), 0, z.getFloat());
			if(near(camera_p, v))
			{
				v.y = lvl.getHeight(x,z).getFloat();
				glVertex3f(v.x, v.y, v.z);
			}
		}
	}
	glEnd();
	glColor3f(0,1,0);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	for(int x = 0; x < lvl.max_block_x(); ++x)
	{
		for(int z = 0; z < lvl.max_block_z(); ++z)
		{
			Vec3 v(x * Level::BLOCK_SIZE, 0, z * Level::BLOCK_SIZE);
			if(near(camera_p, v))
			{
				v.y = lvl.getVertexHeight(x,z).getFloat();
				glVertex3f(v.x, v.y, v.z);
			}
		}
	}
	glEnd();
}



void Graphics::updateLights(const std::map<int, LightObject>& lightsContainer)
{
	while(glGetError() != GL_NO_ERROR); // Clear error flags.

	glUseProgram(shaders["level_program"]);
	int i=0;
	
	int POSITION = 0;
	int DIFFUSE = 1;
	
	for(auto iter = lightsContainer.begin(); iter != lightsContainer.end(); iter++)
	{
		float rgb[4]; rgb[3] = 1.0f;
		
		iter->second.getDiffuse(rgb[0], rgb[1], rgb[2]);
//		glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, rgb);
		stringstream ss1;
		ss1 << "lvl_lights[" << i*2 + DIFFUSE << "]";
		glUniform4f(shaders.uniform(ss1.str()), rgb[0], rgb[1], rgb[2], rgb[3]);
		GLenum error;
		if((error = glGetError()) != GL_NO_ERROR)
		{
			cerr << "glUniform4f failed: " << gluErrorString(error) << " at line " << __LINE__ << "\n";
		}
//		cerr << shaders.uniform(ss1.str()) << " is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;
//		cerr << "DIFFUSE is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;

		const Location& pos = iter->second.getPosition();
		rgb[0] = pos.x.getFloat();
		rgb[1] = pos.y.getFloat();
		rgb[2] = pos.z.getFloat();
		
//		glLightfv(GL_LIGHT0 + i, GL_POSITION, rgb);

		stringstream ss2;
		ss2 << "lvl_lights[" << i*2 + POSITION << "]";
		glUniform4f(shaders.uniform(ss2.str()), rgb[0], rgb[1], rgb[2], rgb[3]);
		if((error = glGetError()) != GL_NO_ERROR)
		{
			cerr << "glUniform4f failed: " << gluErrorString(error) << " at line " << __LINE__ << "\n";
		}
//		cerr << shaders.uniform(ss2.str()) << " is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;
//		cerr << "POSITION of light " << i << " is now " << rgb[0] << " " << rgb[1] << " "<< rgb[2] << " " << rgb[3] << endl;
		
		++i;
		if(i >= MAX_NUM_LIGHTS)
		{
			// if there are too many lights, just ignore the rest of them
			break;
		}
	}
	
	// always write multiples of four
	for(int k=i % 4; (k % 4) != 0; k++)
	{
		if(i >= MAX_NUM_LIGHTS)
		{
			// if there are too many lights, just ignore the rest of them
			break;
		}
		
		float rgb[4]; rgb[0] = rgb[1] = rgb[2] = rgb[3] = 0.0f;
		stringstream ss1;
		ss1 << "lvl_lights[" << i*2 + DIFFUSE << "]";
		glUniform4f(shaders.uniform(ss1.str()), rgb[0], rgb[1], rgb[2], rgb[3]);
		
		stringstream ss2;
		ss2 << "lvl_lights[" << i*2 + POSITION << "]";
		glUniform4f(shaders.uniform(ss2.str()), rgb[0], rgb[1], rgb[2], rgb[3]);
		
		++i;
	}
	
	glUseProgram(0);
}

void Graphics::drawDebugLevelNormals(const Level& lvl)
{
	Vec3 points[3];

	glBegin(GL_LINES);
	for(size_t k=0; k<level_triangles.size(); k++)
	{
		BTT_Triangle& tri = level_triangles[k];
		for(size_t i = 0; i < 3; ++i)
		{
			points[i].x = tri.points[i].x * Level::BLOCK_SIZE;
			points[i].z = tri.points[i].z * Level::BLOCK_SIZE;
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

/*
void Graphics::setActiveLights(const map<int, LightObject>& lightsContainer, const Location& pos)
{
	static vector<LightDistance> distances;
	distances.resize( max(lightsContainer.size(), distances.size()) );
	assert(lightsContainer.size() >= size_t(MAX_NUM_ACTIVE_LIGHTS));

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
	glVertexAttrib4f(shaders.uniform("lvl_activeLights"), distances[0].index, distances[1].index, distances[2].index, distances[3].index);
}
*/

struct ActiveLights
{
	float active_light0;
	float active_light1;
	float active_light2;
	float active_light3;
};

int pass;

void Graphics::drawLevelFR(const Level& lvl, const map<int, LightObject>& lightsContainer)
{
	glUseProgram(shaders["level_program"]);
	assert(lightsContainer.size() >= size_t(MAX_NUM_ACTIVE_LIGHTS));

	static vector<Vec3> vertices;
	static vector<Vec3> normals;
	static vector<TextureCoordinate> texture_coordinates1;
	static vector<TextureCoordinate> texture_coordinates2;
//	static vector<TextureCoordinate> texture_coordinates3; // All texture coordinates are actually same, so we'll let shader handle the third.
//	static vector<ActiveLights> active_lights;

	size_t height = lvl.pointheight_info.size();
	size_t width = lvl.pointheight_info[0].size();
	const int BUFFERS = 6;
	static GLuint locations[BUFFERS];
	int buffer = 0;
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
				const int divisions = 25;
//				TextureCoordinate tc1 = { float(x % (height/divisions)) / (height/divisions), float(z % (width/divisions)) / (width/divisions) };
				TextureCoordinate tc1 = TextureCoordinate( float(x) / (height/divisions), float(z) / (width/divisions) );
				texture_coordinates1.push_back(tc1);
				texture_coordinates2.push_back(tc1);
				
				//ActiveLights ac = {0, 0, 0, 0};
				//active_lights.push_back(ac);
			}
		}

		glGenBuffers(BUFFERS, locations);

		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, texture_coordinates1.size() * sizeof(TextureCoordinate), &texture_coordinates1[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, texture_coordinates2.size() * sizeof(TextureCoordinate), &texture_coordinates2[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(Vec3), &normals[0], GL_STATIC_DRAW);

		assert(buffer <= BUFFERS);
	}

	// Load dynamic indices.
	static vector<unsigned> indices;

if(pass == 0)
{
	indices.clear();
	for(size_t k=0; k<level_triangles.size(); k++)
	{
		Vec3 points[3];
		BTT_Triangle& tri = level_triangles[k];
		for(size_t i = 0; i < 3; ++i)
		{
			points[2-i].x = tri.points[i].x * Level::BLOCK_SIZE;
			points[2-i].z = tri.points[i].z * Level::BLOCK_SIZE;
			points[2-i].y = lvl.getVertexHeight(tri.points[i].x, tri.points[i].z).getFloat();

			indices.push_back( tri.points[2-i].x * width + tri.points[2-i].z  );
		}
	}
	
	if(drawDebuglines)
	{
		TextureHandler::getSingleton().bindTexture(0, "chessboard");
		TextureHandler::getSingleton().bindTexture(1, "chessboard");
		TextureHandler::getSingleton().bindTexture(2, "chessboard");
	}
	else
	{
		TextureHandler::getSingleton().bindTexture(0, "grass");
		TextureHandler::getSingleton().bindTexture(1, "hill");
		TextureHandler::getSingleton().bindTexture(2, "highground");
	}
	
	if(drawDebuglines)
	{
		glUniform4f(shaders.uniform("lvl_ambientLight"), 0.4f, 0.4f, 0.4f, 1.f);
	}
	else
	{
		float r = intVals["AMBIENT_RED"]   / 255.0f;
		float g = intVals["AMBIENT_GREEN"] / 255.0f;
		float b = intVals["AMBIENT_BLUE"]  / 255.0f;
		glUniform4f(shaders.uniform("lvl_ambientLight"), r, g, b, 1.0f);
	}
	
}
else
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthFunc(GL_EQUAL);
	glDepthMask(GL_FALSE);
	
	glUniform4f(shaders.uniform("lvl_ambientLight"), 0.f, 0.f, 0.f, 1.0f);
}
	
	
	glUniform4f(shaders.uniform("lvl_activeLights"), float(pass), float(pass+1), float(pass+2), float(pass+3));
	
	assert(texture_coordinates1.size() == vertices.size());
	assert(texture_coordinates2.size() == vertices.size());
	assert(normals.size() == vertices.size());
	
	buffer = 0;
	
	// Bind static data and send dynamic data to graphics card.
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	
	glClientActiveTexture(GL_TEXTURE1);
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glClientActiveTexture(GL_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glNormalPointer(GL_FLOAT, 0, 0);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	//glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(ActiveLights) * active_lights.size(), &active_lights[0], GL_STREAM_DRAW);
	//glVertexAttribPointer(shaders.uniform("lvl_activeLights"), 4, GL_FLOAT, GL_FALSE, sizeof(ActiveLights), 0);
	buffer++;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STREAM_DRAW);
	
	assert(buffer == BUFFERS);
	
	// Draw sent data.
	//glEnableVertexAttribArray(shaders.uniform("lvl_activeLights"));
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	//glDisableVertexAttribArray(shaders.uniform("lvl_activeLights"));
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	TRIANGLES_DRAWN_THIS_FRAME += level_triangles.size();
	
	glUseProgram(0);
}

struct VisualLevelPart
{
	Vec3 bounding_sphere_center;
	float bounding_sphere_radius;
	
	static const int BUFFERS = 5;
	GLuint locations[BUFFERS];
	bool buffers_loaded;

	vector<unsigned> indices;
	vector<Vec3> vertices;
	vector<Vec3> normals;
	vector<TextureCoordinate> texture_coordinates1;
	vector<TextureCoordinate> texture_coordinates2;
//	vector<TextureCoordinate> texture_coordinates3; // All texture coordinates are actually same, so we'll let shader handle the third.

	VisualLevelPart():
		bounding_sphere_radius(99999999999.9f),
		buffers_loaded(false)
	{
	}

	void preload()
	{
		if(buffers_loaded)
			return;

		cerr << "PRELOADING LEVEL PART ";

		buffers_loaded = true;
		
		glGenBuffers(BUFFERS, locations);

		int buffer = 0;
		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, texture_coordinates1.size() * sizeof(TextureCoordinate), &texture_coordinates1[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, texture_coordinates2.size() * sizeof(TextureCoordinate), &texture_coordinates2[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(Vec3), &normals[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[buffer++]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

		assert(buffer == BUFFERS);

		calculate_bounding_sphere();
		cerr << "CENTER: " << bounding_sphere_center << ", RADIUS: " << bounding_sphere_radius << endl;
	}

	void calculate_bounding_sphere()
	{
		bounding_sphere_center = accumulate(vertices.begin(), vertices.end(), Vec3());
		bounding_sphere_center /= vertices.size();

		bounding_sphere_radius = 0.0f;
		for(size_t i = 0; i < vertices.size(); ++i)
		{
			bounding_sphere_radius = max(bounding_sphere_radius, (bounding_sphere_center - vertices[i]).length());
		}
	}

	void draw() const
	{
		assert(texture_coordinates1.size() == vertices.size());
		assert(texture_coordinates2.size() == vertices.size());
		assert(normals.size() == vertices.size());
		assert(indices.size() % 3 == 0);
		
		int buffer = 0;
		
		// Bind static data and send dynamic data to graphics card.
		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		glEnableClientState(GL_VERTEX_ARRAY);
		
		glClientActiveTexture(GL_TEXTURE1);
		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
		glClientActiveTexture(GL_TEXTURE0);
		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glTexCoordPointer(2, GL_FLOAT, 0, 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
		glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
		glNormalPointer(GL_FLOAT, 0, 0);
		glEnableClientState(GL_NORMAL_ARRAY);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, locations[buffer++]);

		assert(buffer == BUFFERS);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		TRIANGLES_DRAWN_THIS_FRAME += indices.size() / 3;
	}
};

void fill_level_part(VisualLevelPart& part, size_t part_min_x, size_t part_max_x, size_t part_min_z, size_t part_max_z, size_t max_x, size_t max_z, const Level& lvl)
{
	assert(part.vertices.empty());
	assert(part.indices.empty());
	for(size_t x = part_min_x; x < part_max_x; ++x)
	{
		for(size_t z = part_min_z; z < part_max_z; ++z)
		{
			Vec3 point(x * Level::BLOCK_SIZE, lvl.getVertexHeight(x, z).getFloat(), z * Level::BLOCK_SIZE);
			part.vertices.push_back(point);
			
			Location normal = lvl.getNormal(x, z);
			part.normals.push_back(Vec3(normal.x.getFloat(), normal.y.getFloat(), normal.z.getFloat()));
			
			 // TODO: These coordinates are like :G
			const int divisions = 25;
			TextureCoordinate tc1 = TextureCoordinate( float(x) / (max_x/divisions), float(z) / (max_z/divisions) );
			part.texture_coordinates1.push_back(tc1);
			part.texture_coordinates2.push_back(tc1);

			const size_t row_length = part_max_z - part_min_z;
			if(x < part_max_x - 1 && z < part_max_z - 1)
			{
				size_t index = part.vertices.size() - 1;
				assert(index == (x - part_min_x) * row_length + (z - part_min_z));

				if((x + z) % 2 == 1)
				{
					part.indices.push_back(index);
					part.indices.push_back(index+1);
					part.indices.push_back(index + row_length);

					part.indices.push_back(index+1);
					part.indices.push_back(index+1 + row_length);
					part.indices.push_back(index   + row_length);
				}
				else
				{
					part.indices.push_back(index);
					part.indices.push_back(index+1 + row_length);
					part.indices.push_back(index   + row_length);

					part.indices.push_back(index);
					part.indices.push_back(index+1);
					part.indices.push_back(index+1 + row_length);
				}
			}
		}
	}
	cerr << "Filled from: " << part_min_x << " - " << part_max_x << " to " << part_min_z << " - " << part_max_z << " with " << part.indices.size()/3 << " triangles" << endl;
}


void Graphics::drawLevelFR_new(const Level& lvl, const map<int, LightObject>& lightsContainer)
{
	glUseProgram(shaders["level_program"]);
	assert(lightsContainer.size() >= size_t(MAX_NUM_ACTIVE_LIGHTS));

	size_t max_x = lvl.max_block_x();
	size_t max_z = lvl.max_block_z();
	assert(max_x > 0);
	assert(max_z > 0);

	static bool level_buffers_loaded = false;
	static vector<VisualLevelPart> parts;

	// Load static buffers.
	if(!level_buffers_loaded) // TODO: Move initialization somewhere else?
	{
		cerr << "CREATING LEVEL PARTS: " << max_x << " - " << max_z << endl;

		level_buffers_loaded = true;

		int divisions = 20;
		for(int i = 0; i < divisions + 1; ++i)
		{
			for(int j = 0; j < divisions + 1; ++j)
			{
				size_t part_min_x = max_x * i / divisions;
				size_t part_min_z = max_z * j / divisions;
				size_t part_max_x = max_x * (i + 1) / divisions + 1;
				size_t part_max_z = max_z * (j + 1) / divisions + 1;
				part_max_x = min(part_max_x, max_x);
				part_max_z = min(part_max_z, max_z);
				if(part_min_x >= part_max_x || part_min_z >= part_max_z)
				{
					continue;
				}

				parts.push_back(VisualLevelPart());
				fill_level_part(parts.back(), part_min_x, part_max_x, part_min_z, part_max_z, max_x, max_z, lvl);
			}
		}
	}

	if(pass == 0)
	{
		if(drawDebuglines)
		{
			TextureHandler::getSingleton().bindTexture(0, "chessboard");
			TextureHandler::getSingleton().bindTexture(1, "chessboard");
			TextureHandler::getSingleton().bindTexture(2, "chessboard");
		}
		else
		{
			TextureHandler::getSingleton().bindTexture(0, "grass");
			TextureHandler::getSingleton().bindTexture(1, "hill");
			TextureHandler::getSingleton().bindTexture(2, "highground");
		}
		
		if(drawDebuglines)
		{
			glUniform4f(shaders.uniform("lvl_ambientLight"), 0.4f, 0.4f, 0.4f, 1.f);
		}
		else
		{
			float r = intVals["AMBIENT_RED"]   / 255.0f;
			float g = intVals["AMBIENT_GREEN"] / 255.0f;
			float b = intVals["AMBIENT_BLUE"]  / 255.0f;
			glUniform4f(shaders.uniform("lvl_ambientLight"), r, g, b, 1.0f);
		}
		
	}
	else
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthFunc(GL_EQUAL);
		glDepthMask(GL_FALSE);
		
		glUniform4f(shaders.uniform("lvl_ambientLight"), 0.f, 0.f, 0.f, 1.0f);
	}
	
	
	glUniform4f(shaders.uniform("lvl_activeLights"), float(pass), float(pass+1), float(pass+2), float(pass+3));

	for(size_t i = 0; i < parts.size(); ++i)
	{
		if(frustum.sphereInFrustum(parts[i].bounding_sphere_center, parts[i].bounding_sphere_radius) != FrustumR::OUTSIDE)
		{
			parts[i].preload();
			parts[i].draw();
		}
	}
	
	glUseProgram(0);
}

void Graphics::drawDebugLines()
{
	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	for(size_t i = 0; i < LINES.size(); ++i)
	{
		const Vec3& p1 = LINES[i].first;
		const Vec3& p2 = LINES[i].second;
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
	}
	glEnd();

	glPointSize(5.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_POINTS);
	for(size_t i = 0; i < DOTS.size(); ++i)
	{
		const Vec3& p1 = DOTS[i];
		glVertex3f(p1.x, p1.y, p1.z);
	}
	glEnd();

}

void Graphics::drawDebugStrings()
{
	for(size_t i = 0; i < STRINGS.size(); ++i)
	{
		hud.draw3Dstring(STRINGS[i].second, STRINGS[i].first, camera_p->getXrot(), camera_p->getYrot());
	}
}

void Graphics::drawSkybox()
{
	if(drawDebuglines)
	{
		TextureHandler::getSingleton().bindTexture(0, "chessboard");
	}
	else
	{
		TextureHandler::getSingleton().bindTexture(0, strVals["skybox"]);
	}

	glPushMatrix();
	glLoadIdentity();

	Vec3 ans = camera_p->getTarget() - camera_p->getPosition();
	gluLookAt(
			0,0,0,
			ans.x, ans.y, ans.z,
			0,1,0);
	glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	glColor3f(1.0f, 1.0f, 1.0f);

	double minus = -20.0;
	double plus  =  20.0;
	double epsilon = 0.001;
//	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_QUADS); // Front
	glTexCoord2f(1.0/4+epsilon, 2.0/3-epsilon); glVertex3f( plus,  plus, plus-epsilon);
	glTexCoord2f(1.0/4+epsilon, 1.0/3+epsilon); glVertex3f( plus, minus, plus-epsilon);
	glTexCoord2f(2.0/4-epsilon, 1.0/3+epsilon); glVertex3f(minus, minus, plus-epsilon);
	glTexCoord2f(2.0/4-epsilon, 2.0/3-epsilon); glVertex3f(minus,  plus, plus-epsilon);
	glEnd();
//	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_QUADS); // Right
	glTexCoord2f(2.0/4+epsilon, 2.0/3-epsilon); glVertex3f(minus+epsilon,  plus, plus);
	glTexCoord2f(2.0/4+epsilon, 1.0/3+epsilon); glVertex3f(minus+epsilon, minus, plus);
	glTexCoord2f(3.0/4-epsilon, 1.0/3+epsilon); glVertex3f(minus+epsilon, minus, minus);
	glTexCoord2f(3.0/4-epsilon, 2.0/3-epsilon); glVertex3f(minus+epsilon,  plus, minus);
	glEnd();
//	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS); // Left
	glTexCoord2f(0.0/4+epsilon, 2.0/3-epsilon); glVertex3f(plus-epsilon,  plus, minus);
	glTexCoord2f(0.0/4+epsilon, 1.0/3+epsilon); glVertex3f(plus-epsilon, minus, minus);
	glTexCoord2f(1.0/4-epsilon, 1.0/3+epsilon); glVertex3f(plus-epsilon, minus, plus);
	glTexCoord2f(1.0/4-epsilon, 2.0/3-epsilon); glVertex3f(plus-epsilon,  plus, plus);
	glEnd();
//	glColor3f(1.0f, 0.0f, 1.0f);
	glBegin(GL_QUADS); // Top
	glTexCoord2f(1.0/4+epsilon, 2.0/3+epsilon); glVertex3f( plus, plus-epsilon, plus);
	glTexCoord2f(2.0/4-epsilon, 2.0/3+epsilon); glVertex3f(minus, plus-epsilon, plus);
	glTexCoord2f(2.0/4-epsilon, 3.0/3-epsilon); glVertex3f(minus, plus-epsilon, minus);
	glTexCoord2f(1.0/4+epsilon, 3.0/3-epsilon); glVertex3f( plus, plus-epsilon, minus);
	glEnd();
//	glColor3f(0.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS); // Bottom
	glTexCoord2f(1.0/4+epsilon, 1.0/3-epsilon); glVertex3f( plus, minus+epsilon,  plus);
	glTexCoord2f(1.0/4+epsilon, 0.0/3+epsilon); glVertex3f( plus, minus+epsilon, minus);
	glTexCoord2f(2.0/4-epsilon, 0.0/3+epsilon); glVertex3f(minus, minus+epsilon, minus);
	glTexCoord2f(2.0/4-epsilon, 1.0/3-epsilon); glVertex3f(minus, minus+epsilon,  plus);
	glEnd();
//	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_QUADS); // Back
	glTexCoord2f(3.0/4+epsilon, 2.0/3-epsilon); glVertex3f(minus,  plus, minus+epsilon);
	glTexCoord2f(3.0/4+epsilon, 1.0/3+epsilon); glVertex3f(minus, minus, minus+epsilon);
	glTexCoord2f(4.0/4-epsilon, 1.0/3+epsilon); glVertex3f( plus, minus, minus+epsilon);
	glTexCoord2f(4.0/4-epsilon, 2.0/3-epsilon); glVertex3f( plus,  plus, minus+epsilon);
	glEnd();

	glPopAttrib();
	glPopMatrix();
}

void Graphics::drawModels(const map<int, Model*>& models)
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
	for(map<int, Model*>::const_iterator iter = models.begin(); iter != models.end(); ++iter)
	{
		const Model& model = *iter->second;

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

// TODO: make this function faster than the current implementation by having the data already in the VBO format.
// TODO: Now it has to do extra copying.
void Graphics::drawParticles_vbo(std::vector<Particle>& viewParticles)
{
	glUseProgram(shaders["particle_program"]);
	GLint scale_location = shaders.uniform("particle_particleScale");

	Vec3 direction_vector = camera_p->getTarget() - camera_p->getPosition();
	depthSortParticles(direction_vector, viewParticles);
	
	TextureHandler::getSingleton().bindTexture(1, "screenFBO_depth_texture");
	TextureHandler::getSingleton().bindTexture(0, "particle");
//	TextureHandler::getSingleton().bindTexture(0, "smoke");

	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glDepthMask(GL_FALSE); // dont write to depth buffer.

	static vector<Vec3> particles;
	static vector<float> colors;
	static vector<float> scales;
	static bool buffers_loaded = false;
	static const int BUFFERS = 3;
	static GLuint locations[BUFFERS];
	
	if(!buffers_loaded)
	{
		buffers_loaded = true;
		glGenBuffers(BUFFERS, locations);
	}

	size_t n = viewParticles.size();
	if(particles.size() < n)
	{
		particles.resize(n);
		colors.resize(4*n);
		scales.resize(n);
	}

	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		particles[i] = Vec3(viewParticles[i].pos.x.getFloat(), viewParticles[i].pos.y.getFloat(), viewParticles[i].pos.z.getFloat());

		viewParticles[i].getColor(&colors[i*4]);
		colors[i*4 + 3] = viewParticles[i].getAlpha();
		scales[i] = viewParticles[i].getScale();
		++QUADS_DRAWN_THIS_FRAME;
	}

	int buffer = 0;
	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(Vec3), &particles[0], GL_STREAM_DRAW);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, n * 4 * sizeof(float), &colors[0], GL_STREAM_DRAW);
	glColorPointer(4, GL_FLOAT, 0, 0);
	glEnableClientState(GL_COLOR_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, locations[buffer++]);
	glBufferData(GL_ARRAY_BUFFER, n * sizeof(float), &scales[0], GL_STREAM_DRAW);
	glVertexAttribPointer(scale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);

	assert(buffer == BUFFERS);

	// Draw sent data.
	glEnableVertexAttribArray(scale_location);
	glDrawArrays(GL_POINTS, 0, viewParticles.size());
	glDisableVertexAttribArray(scale_location);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDepthMask(GL_TRUE); // re-enable depth writing.
	glDisable(GL_BLEND);

	TextureHandler::getSingleton().bindTexture(1, "");
	TextureHandler::getSingleton().bindTexture(0, "");

	glUseProgram(0);
}

void Graphics::prepareForParticleRendering()
{
	glBindFramebuffer(GL_FRAMEBUFFER, particlesFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, intVals["RESOLUTION_X"] / intVals["PARTICLE_RESOLUTION_DIVISOR"], intVals["RESOLUTION_Y"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
	
	TextureHandler::getSingleton().bindTexture(1, "screenFBO_depth_texture");
	TextureHandler::getSingleton().bindTexture(0, "particle");
	
	glUseProgram(shaders["particle_program"]);
	
	// Vec3 direction_vector = camera_p->getTarget() - camera_p->getPosition();
	// depthSortParticles(direction_vector, viewParticles);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE); // dont write to depth buffer.
	glDisable(GL_DEPTH_TEST);
}

void Graphics::renderParticles(std::vector<Particle>& viewParticles)
{
	GLint scale_location = shaders.uniform("particle_particleScale");
	
	// The geometry shader transforms the points into quads.
	glBegin(GL_POINTS);
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		float px = viewParticles[i].pos.x.getFloat();
		float py = viewParticles[i].pos.y.getFloat();
		float pz = viewParticles[i].pos.z.getFloat();
		
		float color[4];
		viewParticles[i].getColor(color);
		color[3] = viewParticles[i].getAlpha();
		
		// cerr << color[0] << " " << color[1] << " " << color[2] << " " << color[3] << "\n";
		
		float scale = viewParticles[i].getScale();
		
		glVertexAttrib1f(scale_location, scale);
		glColor4fv(color);
		glVertex3f(px, py, pz);
		
		++QUADS_DRAWN_THIS_FRAME;
	}
	glEnd();
	
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	TextureHandler::getSingleton().bindTexture(1, "");
}

void Graphics::drawParticles(std::vector<Particle>& viewParticles)
{
	
	prepareForParticleRendering();
	
	// create a downscaled depth texture for particle rendering
	
	renderParticles(viewParticles);
	
	// upscale particle rendering result
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, particlesUpScaledFBO);
	glViewport(0, 0, intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	TextureHandler::getSingleton().bindTexture(0, "particlesFBO_texture");
	glUseProgram(0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(+1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(+1.0f, +1.0f, -1.0f);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f, +1.0f, -1.0f);
	glEnd();
	
	// blur upscaled particle texture
	if(intVals["PARTICLE_BLUR"])
	{
		applyBlur(intVals["PARTICLE_BLUR_AMOUNT"], "particlesUpScaledFBO_texture", particlesUpScaledFBO);
		applyBlur(intVals["PARTICLE_BLUR_AMOUNT"], "particlesUpScaledFBO_texture", particlesUpScaledFBO);
	}
	
	// render to screen.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
	
	glUseProgram(0);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(+1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(+1.0f, +1.0f, -1.0f);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f, +1.0f, -1.0f);
	glEnd();
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	glUseProgram(0);
	TextureHandler::getSingleton().bindTexture(0, "");
	
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

// This function is not in use anymore, would be useful only if geometry shader is not available. Remove if you feel like it.
void Graphics::drawParticles_old(std::vector<Particle>& viewParticles)
{
	
	Vec3 direction_vector = camera_p->getTarget() - camera_p->getPosition();
	// depthSortParticles(direction_vector, viewParticles);
	
	glBindFramebuffer(GL_FRAMEBUFFER, particlesFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, intVals["RESOLUTION_X"] / intVals["PARTICLE_RESOLUTION_DIVISOR"], intVals["RESOLUTION_Y"] / intVals["PARTICLE_RESOLUTION_DIVISOR"]);
	
	//TextureHandler::getSingleton().bindTexture(1, "screenFBO_depth_texture");
	TextureHandler::getSingleton().bindTexture(0, "particle");
	
	// glUseProgram(shaders["particle_program"]);
	// GLint scale_location = shaders.uniform("particle_particleScale");
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE); // dont write to depth buffer.

	glBegin(GL_QUADS);
	
	float x_angle =  camera_p->getXrot();
	float y_angle = -camera_p->getYrot() - 90.f;
	Matrix4 m(y_angle, x_angle, 0.f, 0.f, 0.f, 0.f);
	
	Vec3 s1(-1.0f, -1.0f, 0.0f);
	Vec3 s2(+1.0f, -1.0f, 0.0f);
	Vec3 s3(+1.0f, +1.0f, 0.0f);
	Vec3 s4(-1.0f, +1.0f, 0.0f);
	
	s1 = m * s1;
	s2 = m * s2;
	s3 = m * s3;
	s4 = m * s4;
	
	float color[4];
	
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		
		float px = viewParticles[i].pos.x.getFloat();
		float py = viewParticles[i].pos.y.getFloat();
		float pz = viewParticles[i].pos.z.getFloat();
		
		viewParticles[i].getColor(color);
		color[3] = viewParticles[i].getAlpha();
		glColor4fv(color);
		
		float s = viewParticles[i].getScale();
		glTexCoord2f(0.f, 0.f); glVertex3f(px+s1.x*s, py+s1.y*s, pz+s1.z*s);
		glTexCoord2f(1.f, 0.f); glVertex3f(px+s2.x*s, py+s2.y*s, pz+s2.z*s);
		glTexCoord2f(1.f, 1.f); glVertex3f(px+s3.x*s, py+s3.y*s, pz+s3.z*s);
		glTexCoord2f(0.f, 1.f); glVertex3f(px+s4.x*s, py+s4.y*s, pz+s4.z*s);
		
		++QUADS_DRAWN_THIS_FRAME;
	}
	
	glEnd();
	
	
	
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	TextureHandler::getSingleton().bindTexture(1, "");
	
	// /*
	glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
	glViewport(0, 0, intVals["RESOLUTION_X"], intVals["RESOLUTION_Y"]);
	
	
	TextureHandler::getSingleton().bindTexture(0, "particlesFBO_texture");
	
	glUseProgram(0);
	
	//glBlendFunc(GL_ONE, GL_ONE);
	glBlendFunc(GL_SRC_COLOR, GL_ONE);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(+1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(+1.0f, +1.0f, -1.0f);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.0f, +1.0f, -1.0f);
	glEnd();
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	// */
	
	glUseProgram(0);
	TextureHandler::getSingleton().bindTexture(0, "");
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	glUseProgram(0);
}

void Graphics::updateCamera(const Level& lvl)
{
	Vec3 camStartPos = camera_p->getPosition();
	FixedPoint camX = FixedPoint(camStartPos.x);
	FixedPoint camZ = FixedPoint(camStartPos.z);
	
	float cam_min_y = lvl.getHeight(camX, camZ).getFloat() + 3.f;
	camera_p->setAboveGround(cam_min_y);
}

void Graphics::startDrawing()
{
	glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(camera_p->fov, camera_p->aspect_ratio, camera_p->nearP, camera_p->farP);
	glMatrixMode(GL_MODELVIEW);
	
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_DEPTH_BUFFER_BIT); // Dont clear color buffer, since we're going to rewrite the color everywhere anyway.

	glClear(GL_COLOR_BUFFER_BIT);
	
	Vec3 camPos = camera_p->getPosition();
	Vec3 camTarget = camera_p->getTarget();
	Vec3 upVector(0.0, 1.0, 0.0);

	glLoadIdentity();
	gluLookAt(camPos.x, camPos.y, camPos.z,
			  camTarget.x, camTarget.y, camTarget.z,
			  upVector.x, upVector.y, upVector.z);
			  
	frustum.setCamDef(camPos, camTarget, upVector);

	TRIANGLES_DRAWN_THIS_FRAME = 0;
	QUADS_DRAWN_THIS_FRAME = 0;
}



void Graphics::applySSAO(int power, string inputImg, string depthImage, GLuint renderTarget)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, renderTarget);
	TextureHandler::getSingleton().bindTexture(0, inputImg);
	TextureHandler::getSingleton().bindTexture(1, depthImage);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glUseProgram(shaders["ssao"]);
	glUniform1f(shaders.uniform("ssao_power"), float(power));
	
	glColor3f(1.0, 1.0, 1.0);
	
	//	TextureHandler::getSingleton().bindTexture(1, "screenFBO_depth_texture");
	//	glUseProgram(shaders["debug_program"]);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.f, +1.f, -1.0f);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.f, -1.f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(+1.f, -1.f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(+1.f, +1.f, -1.0f);
	glEnd();
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	
	TextureHandler::getSingleton().bindTexture(1, "");
	glUseProgram(0);
}


void Graphics::applyBlur(int blur, string inputImg, GLuint renderTarget)
{
	glDisable(GL_DEPTH_TEST);
	//glDepthMask(GL_FALSE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
	TextureHandler::getSingleton().bindTexture(0, inputImg);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glUseProgram(shaders["blur_program1"]);
	glUniform1f(shaders.uniform("blur_amount1"), float(blur));
	
	glColor3f(1.0, 1.0, 1.0);
	
	//	TextureHandler::getSingleton().bindTexture(1, "screenFBO_depth_texture");
	//	glUseProgram(shaders["debug_program"]);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.f, +1.f, -1.0f);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.f, -1.f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(+1.f, -1.f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(+1.f, +1.f, -1.0f);
	glEnd();
	
	glBindFramebuffer(GL_FRAMEBUFFER, renderTarget);
	TextureHandler::getSingleton().bindTexture(0, "postFBO_texture");
	
	glUseProgram(shaders["blur_program2"]);
	glUniform1f(shaders.uniform("blur_amount2"), float(blur));
	
	glColor3f(1.0, 1.0, 1.0);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.f, +1.f, -1.0f);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.f, -1.f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(+1.f, -1.f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(+1.f, +1.f, -1.0f);
	glEnd();
	
	glEnable(GL_DEPTH_TEST);
	//glDepthMask(GL_TRUE);
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glUseProgram(0);
	// TextureHandler::getSingleton().bindTexture(0, "");
}


void Graphics::draw(
	const Level& lvl,
	const VisualWorld& visualworld,
	const std::shared_ptr<Octree> o, // For debug.
	const std::map<int, Projectile>& projectiles, // For debug.
	const std::map<int, Unit>& units // For debug and names.
	)
{
	updateCamera(lvl);
	
	startDrawing();
	
	if(intVals["DRAW_SKYBOX"])
	{
		drawSkybox();
	}
	
	if(drawDebuglines)
	{
		drawDebugLevelNormals(lvl);
		drawDebugProjectiles(projectiles);
	}

	if(drawDebugWireframe)
	{
		drawDebugHeightDots(lvl);
	}
	else if(intVals["DRAW_LEVEL"])
	{
		// draw terrain with forward rendering, applying lights there
		for(size_t i = 0; i < visualworld.lights.size(); i += 4)
		{
			pass = int(i);
			if(intVals["DRAW_LEVEL"] == 2)
			{
				drawLevelFR_new(lvl, visualworld.lights);
			}
			else
			{
				drawLevelFR(lvl, visualworld.lights);
			}
		}
		
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);
		
		TextureHandler::getSingleton().bindTexture(2, "");
		TextureHandler::getSingleton().bindTexture(1, "");
		TextureHandler::getSingleton().bindTexture(0, "");
	}
	
	if(drawDebuglines)
	{
		drawDebugLines();
	}

	if(intVals["DRAW_MODELS"])
	{
		drawModels(visualworld.models);
	}

	if(intVals["DRAW_GRASS"])
	{
		drawGrass(visualworld.meadows);
	}
	
	if(intVals["SSAO"])
	{
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		applySSAO(intVals["SSAO_DISTANCE"], "screenFBO_texture", "screenFBO_depth_texture", screenFBO);
	}
	
	if(intVals["DRAW_PARTICLES"])
	{
		drawParticles(visualworld.particles);
	}
	
	if(drawDebuglines)
	{
		drawBoundingBoxes(units);
		drawOctree(o);
	}
	
	if(intVals["DRAW_NAMES"])
	{
		drawPlayerNames(units, visualworld.models);
	}
	
	drawDebugStrings();
	
	int blur = units.find(hud.myID)->second["D"];
	if(camera_p->mode() == Camera::STATIC)
		blur = 0;
	else if( blur > 13 )
		blur = 13;
	
	if(intVals["DAMAGE_BLUR"])
		applyBlur(blur, "screenFBO_texture", screenFBO);
	
	hud.draw(camera_p->mode() == Camera::FIRST_PERSON);
	
	finishDrawing();
}

void Graphics::drawPlayerNames(const std::map<int, Unit>& units, const map<int, Model*>& models)
{
	for(auto iter = units.begin(); iter != units.end(); ++iter)
	{
		/*
		if(!iter->second.human())
		{
			continue;
		}
		*/
		const Model& model = *models.find(iter->first)->second;
		Vec3 pos = model.currentModelPos;
		pos.y += 5.0;

		hud.draw3Dstring(iter->second.name, pos, camera_p->getXrot(), camera_p->getYrot(), iter->second["TEAM"]);
	}
}

void Graphics::finishDrawing()
{
	STRINGS.clear();
	
	glUseProgram(0);
	
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	TextureHandler::getSingleton().bindTexture(0, "screenFBO_texture");
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glUseProgram(0);
	
	glColor3f(1.0, 1.0, 1.0);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 1.f); glVertex3f(-1.f, +1.f, -1.0f);
	glTexCoord2f(0.f, 0.f); glVertex3f(-1.f, -1.f, -1.0f);
	glTexCoord2f(1.f, 0.f); glVertex3f(+1.f, -1.f, -1.0f);
	glTexCoord2f(1.f, 1.f); glVertex3f(+1.f, +1.f, -1.0f);
	glEnd();
	glEnable(GL_DEPTH_TEST);
	
//	TextureHandler::getSingleton().bindTexture(1, "");
//	TextureHandler::getSingleton().bindTexture(0, "");
	
	window.swap_buffers();
}

// TODO: This could be in visualworld
void Graphics::updateInput(int keystate)
{
	camera_p->updateInput(keystate);
}

void Graphics::world_tick(Level& lvl, const std::map<int, LightObject>& lights)
{
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
}

void Graphics::toggleFullscreen()
{
	window.toggle_fullscreen();
}

void Graphics::zoom_in()
{
	camera_p->zoomIn();
}

void Graphics::zoom_out()
{
	camera_p->zoomOut();
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

void Graphics::drawOctree(const std::shared_ptr<Octree>& o)
{
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

void Graphics::drawGrass(const std::vector<GrassCluster>& meadows)
{
	glUseProgram(shaders["grass_program"]);
	
	if(drawDebuglines)
	{
		TextureHandler::getSingleton().bindTexture(0, "chessboard");
	}
	else
	{
		TextureHandler::getSingleton().bindTexture(0, "meadow1");
	}
	
	for(size_t i = 0; i < meadows.size(); ++i)
	{
		if(frustum.sphereInFrustum(meadows[i].center, meadows[i].radius) != FrustumR::OUTSIDE)
		{
			meadows[i].draw(&shaders);
		}
	}

	glUseProgram(0);

}

void Graphics::drawMenu(const vector<MenuButton>& buttons) const
{
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	
	
	// render a menu background scene! Falling particles would be awesome.
	
	
	
	// render menu buttons on top of the scene.
	
	float menu_height = 0.3f;
	float menu_y_offset  = 0.2f;
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	
	for(size_t i = 0; i < buttons.size(); ++i)
	{
		stringstream msg;
		stringstream info;
		if(buttons[i].selected == 1)
		{
			msg << "^G";
			info << "^G";
		}
		else
		{
			msg << "^W";
			info << "^G";
		}
		
		float minus = 2.f * (i+0.f) / buttons.size() - 1.f;
		// float plus  = 2.f * (i+1.f) / buttons.size() - 1.f;
		
		// TextureHandler::getSingleton().bindTexture(0, buttons[i].name);
		
		// void drawString(const std::string&, float pos_x = -1.0f, float pos_y = -1.0f, float scale = 1.0f, bool background = false, float alpha = 1.0f) const;
		
		msg << buttons[i].name;
		info << buttons[i].info;
		hud.drawString(msg.str(), -0.7f, minus * menu_height + menu_y_offset, 3.5f);
		hud.drawString(info.str(), 0.0f, minus * menu_height + menu_y_offset, 3.5f);
		
		/*
		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.0f); glVertex3f(-1, minus * menu_height + menu_y_offset, -1);
		glTexCoord2f(1.f, 0.0f); glVertex3f(+0, minus * menu_height + menu_y_offset, -1);
		glTexCoord2f(1.f, 1.0f); glVertex3f(+0, plus  * menu_height + menu_y_offset, -1);
		glTexCoord2f(0.f, 1.0f); glVertex3f(-1, plus  * menu_height + menu_y_offset, -1);
		glEnd();
		*/
	}
	
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	
	window.swap_buffers();
}

void Graphics::reload_shaders()
{
	shaders.release();
	shaders.init();
}

