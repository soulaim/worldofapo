#include "texturehandler.h"
#include "graphics.h"
#include "level.h"
#include "shaders.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <algorithm>

using namespace std;

vector<pair<Vec3,Vec3> > LINES;
vector<Vec3> DOTS;

int TRIANGLES_DRAWN_THIS_FRAME = 0;
int QUADS_DRAWN_THIS_FRAME = 0;

std::map<std::string, ObjectPart> Graphics::objects;

float Graphics::modelGround(const Model& model)
{
	return model.height();
}

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
	lightsActive = false;
	GLfloat global_ambient[ 4 ] = {0.f, 0.f,  0.f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	
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
	}
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

void Graphics::setTime(unsigned time)
{
	currentTime = time;
}

void Graphics::drawCrossHair()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	TextureHandler::getSingleton().bindTexture("crosshair");
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 0.f); glVertex3f(-0.03f, +0.02f, -1);
	glTexCoord2f(1.f, 0.f); glVertex3f(+0.03f, +0.02f, -1);
	glTexCoord2f(1.f, 1.f); glVertex3f(+0.03f, +0.08f, -1);
	glTexCoord2f(0.f, 1.f); glVertex3f(-0.03f, +0.08f, -1);
	glEnd();
	
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	if(lightsActive)
		glEnable(GL_LIGHTING);
}

void Graphics::drawString(const string& msg, float pos_x, float pos_y, float scale, bool background)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	TextureHandler::getSingleton().bindTexture("font");
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	float totalWidth = 0.025f;
	for(size_t i = 0; i < msg.size(); ++i)
	{
		if(msg[i] == '^')
		{
			++i;
			continue;
		}
		
		totalWidth += 0.05 * charWidth[msg[i]] * 2 * scale;
	}
	
	float x_now     = 0.0f;
	float x_next    = pos_x + 0.05;
	float y_bot     = pos_y;
	float y_top     = pos_y + 0.05 * scale;
	float edge_size = 1./16.;
	
	// draw a darker background box for the text if that was requested
	if(background)
	{
		glDisable(GL_TEXTURE_2D);
		glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
		glBegin(GL_QUADS);
		glVertex3f(pos_x - 0.01 * scale , y_bot, -1);
		glVertex3f(pos_x + totalWidth    , y_bot, -1);
		glVertex3f(pos_x + totalWidth    , y_top, -1);
		glVertex3f(pos_x - 0.01 * scale , y_top, -1);
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
	
	float currentWidth = 0.f;
	float lastWidth    = 0.f;
	
	// reset default colour to white.
	glColor4f(1.0f, 1.0f, 1.0f, 1.f);
	
	for(size_t i = 0; i < msg.size(); ++i)
	{
		
		if(msg[i] == '^')
		{
			++i;
			if(msg[i] == 'g')
				glColor4f(0.1f, 0.6f, 0.1f, 1.0f);
			else if(msg[i] == 'G')
				glColor4f(0.1f, 1.0f, 0.1f, 1.0f);
			else if(msg[i] == 'r')
				glColor4f(0.6f, 0.1f, 0.1f, 1.0f);
			else if(msg[i] == 'R')
				glColor4f(1.0f, 0.1f, 0.1f, 1.0f);
			else if(msg[i] == 'b')
				glColor4f(0.1f, 0.1f, 0.6f, 1.0f);
			else if(msg[i] == 'B')
				glColor4f(0.1f, 0.1f, 1.0f, 1.0f);
			else if(msg[i] == 'y')
				glColor4f(0.7f, 0.7f, 0.0f, 1.0f);
			else if(msg[i] == 'Y')
				glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
			else if(msg[i] == 'W')
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			else if(msg[i] == 'w')
				glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
			
			continue;
		}
		
		
		currentWidth = 0.05 * charWidth[msg[i]];
		
		x_now = x_next + scale * (currentWidth + lastWidth - 0.05f);
		x_next = x_now + 0.05f * scale;
		
		lastWidth = currentWidth;
		
		int x = msg[i] % 16;
		int y = 15 - (msg[i] / 16);
		
		glBegin(GL_QUADS);
		glTexCoord2f( x    * edge_size, y * edge_size);     glVertex3f(x_now , y_bot, -1);
		glTexCoord2f((x+1) * edge_size, y * edge_size);     glVertex3f(x_next, y_bot, -1);
		glTexCoord2f((x+1) * edge_size, (y+1) * edge_size); glVertex3f(x_next, y_top, -1);
		glTexCoord2f( x    * edge_size, (y+1) * edge_size); glVertex3f(x_now , y_top, -1);
		glEnd();
	}
	
	glDisable(GL_BLEND);
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	if(lightsActive)
		glEnable(GL_LIGHTING);
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
	currentTime = 0;
	zombieCount = 0;
	world_ticks = 0;
	init();
}

void Graphics::init()
{
	
	charWidth.resize(255, 1.f);
	
	for(char symbol = 'A'; symbol <= 'Z'; symbol++)
		charWidth[symbol] = 0.22;
	for(char symbol = 'a'; symbol <= 'z'; symbol++)
		charWidth[symbol] = 0.19;
	for(char symbol = '0'; symbol <= '9'; symbol++)
		charWidth[symbol] = 0.16;
	
	charWidth['9'] = 0.20;
	charWidth['8'] = 0.20;
	charWidth['4'] = 0.20;
	charWidth['0'] = 0.23;
	charWidth['l'] = 0.1;
	charWidth['r'] = 0.1;
	charWidth['f'] = 0.1;
	charWidth['!'] = 0.1;
	charWidth['t'] = 0.15;
	charWidth['>'] = 0.15;
	charWidth['<'] = 0.15;
	charWidth['i'] = 0.1;
	charWidth['w'] = 0.25;
	charWidth['m'] = 0.25;
	charWidth['j'] = 0.12;
	charWidth['o'] = 0.19;
	charWidth['s'] = 0.12;
	charWidth['I'] = 0.1;
	charWidth['J'] = 0.12;
	charWidth['.'] = 0.1;
	charWidth[','] = 0.1;
	charWidth[':'] = 0.1;
	charWidth['?'] = 0.15;
	charWidth[' '] = 0.1;
	charWidth[']'] = 0.1;
	charWidth['['] = 0.1;
	charWidth[')'] = 0.1;
	charWidth['('] = 0.1;
	charWidth['\''] = 0.1;
	charWidth['-'] = 0.1;
	charWidth['|'] = 0.1;
	charWidth['/'] = 0.1;
	charWidth['_'] = 0.1;
	charWidth['%'] = 0.13;
	charWidth['&'] = 0.12;
	charWidth['='] = 0.13;
	charWidth['"'] = 0.1;
	
	charWidth['S'] = 0.17;
	charWidth['T'] = 0.19;
	charWidth['W'] = 0.24;
	charWidth['Q'] = 0.24;
	charWidth['O'] = 0.3;
	charWidth['Z'] = 0.1;
	charWidth['M'] = 0.3;
	
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

	drawDebuglines = false;
}

void Graphics::createWindow()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
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
		exit(0);
	}
	else
	{
		cerr << "SUCCESS: Got a drawContext!" << endl;
	}
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

void setActiveLights(const map<int, LightObject>& lightsContainer, const FixedPoint& fpx, const FixedPoint& fpy, const FixedPoint& fpz)
{
	vector<int> indexes;
	vector<FixedPoint> distances;

	distances.resize(2, FixedPoint(40000));
	indexes.resize(2, -1);

	Location fixMe;
	fixMe.x = fpx;
	fixMe.y = fpy;
	fixMe.z = fpz;

	for(auto iter = lightsContainer.begin(); iter != lightsContainer.end(); iter++)
	{
		const LightObject& light = iter->second;
		FixedPoint lightDistance = (light.position - fixMe).lengthSquared();


		if(lightDistance < distances[0])
		{
			distances[1] = distances[0];
			indexes[1]   = indexes[0];

			distances[0] = lightDistance;
			indexes[0]  = iter->first;
		}
		else if(lightDistance < distances[1])
		{
			distances[1] = lightDistance;
			indexes[1]  = iter->first;
		}
	}

	for(size_t light_i = 0; light_i < indexes.size(); light_i++)
	{
		if(indexes[light_i] >= 0)
		{
			float rgb[4]; rgb[3] = 1.0f;
			int light_key = indexes[light_i];
			lightsContainer.find(light_key)->second.getDiffuse(rgb[0], rgb[1], rgb[2]);

			glLightfv(GL_LIGHT1+light_i, GL_DIFFUSE, rgb);

			Location pos = lightsContainer.find(light_key)->second.getPosition();

			rgb[0] = pos.x.getFloat();
			rgb[1] = pos.y.getFloat();
			rgb[2] = pos.z.getFloat();

			glLightfv(GL_LIGHT1+light_i, GL_POSITION, rgb);
		}
		else
		{
			float rgb[4] = { };
			glLightfv(GL_LIGHT1+light_i, GL_DIFFUSE, rgb);
		}
	}
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

void Graphics::drawLevel(const Level& lvl, const map<int, LightObject>& lightsContainer)
{
	glUseProgram(shaders["level_program"]);
	//glEnable(GL_TEXTURE_2D);
	TextureHandler::getSingleton().bindTexture("grass");
	
	// this should only be done at world ticks! not at draw ticks..
	level_triangles.clear();
	lvl.btt.getTriangles(level_triangles);
	
	int multiplier = 8;
	
	Vec3 semiAverage;
	Vec3 points[3];

	glColor3f(1.0,1.0,1.0);
	for(size_t k=0; k<level_triangles.size(); k++)
	{
		BTT_Triangle& tri = level_triangles[k];
		for(size_t i = 0; i < 3; ++i)
		{
			points[i].x = tri.points[i].x * multiplier;
			points[i].z = tri.points[i].z * multiplier;
			points[i].y = lvl.getVertexHeight(tri.points[i].x, tri.points[i].z).getFloat();
		}
		
		semiAverage = (points[0] + points[1] + points[2]) / 3;
		float r = (semiAverage - points[0]).length();
		
		if(frustum.sphereInFrustum(semiAverage, r) != FrustumR::OUTSIDE)
		{
			Location n;
			
			if(drawDebuglines)
			{
				n = lvl.getNormal(tri.points[0].x, tri.points[0].z) * 10;
				Location start;
				start.x = FixedPoint(int(points[0].x));
				start.y = lvl.getVertexHeight(tri.points[0].x, tri.points[0].z);
				start.z = FixedPoint(int(points[0].z));
				
				Location end = start + n;
				
				glBegin(GL_LINES);
				glColor3f(1.0, 0.0, 0.0); glVertex3f(start.x.getFloat(), start.y.getFloat(), start.z.getFloat());
				glColor3f(0.0, 1.0, 0.0); glVertex3f(end.x.getFloat(), end.y.getFloat(), end.z.getFloat());
				glEnd();
				
				glColor3f(1.0, 1.0, 1.0);
			}
			
			lightsContainer.empty();
			
			
			
			// set active lights
			vector<int> indexes;
			vector<FixedPoint> distances;
			
			distances.resize(2, FixedPoint(40000));
			indexes.resize(2, -1);
			
			Location fixMe;
			fixMe.x = int(semiAverage.x);
			fixMe.y = 10;
			fixMe.z = int(semiAverage.z);
			
			for(auto iter = lightsContainer.begin(); iter != lightsContainer.end(); iter++)
			{
				const LightObject& light = iter->second;
				FixedPoint lightDistance = (light.position - fixMe).lengthSquared();
				
				if(lightDistance < distances[0])
				{
					distances[1] = distances[0];
					indexes[1]   = indexes[0];
					
					distances[0] = lightDistance;
					indexes[0]  = iter->first;
				}
				else if(lightDistance < distances[1])
				{
					distances[1] = lightDistance;
					indexes[1]  = iter->first;
				}
			}
			
			for(size_t light_i = 0; light_i < indexes.size(); light_i++)
			{
				if(indexes[light_i] >= 0)
				{
					float rgb[4]; rgb[3] = 1.0f;
					int light_key = indexes[light_i];
					lightsContainer.find(light_key)->second.getDiffuse(rgb[0], rgb[1], rgb[2]);
					
					glLightfv(GL_LIGHT1+light_i, GL_DIFFUSE, rgb);
					
					Location pos = lightsContainer.find(light_key)->second.getPosition();
					
					rgb[0] = pos.x.getFloat();
					rgb[1] = pos.y.getFloat();
					rgb[2] = pos.z.getFloat();
					
					glLightfv(GL_LIGHT1+light_i, GL_POSITION, rgb);
				}
				else
				{
					float rgb[4] = { };
					glLightfv(GL_LIGHT1+light_i, GL_DIFFUSE, rgb);
				}
			}
			
			
			
			// TODO: Terrain texture
			glBegin(GL_TRIANGLES);
			n=lvl.getNormal(tri.points[0].x, tri.points[0].z); glNormal3f(n.x.getFloat(), n.y.getFloat(), n.z.getFloat());
			glTexCoord2f(0.f, 0.0f); glVertex3f( points[0].x, points[0].y, points[0].z );
			
			n=lvl.getNormal(tri.points[1].x, tri.points[1].z); glNormal3f(n.x.getFloat(), n.y.getFloat(), n.z.getFloat());
			glTexCoord2f(1.f, 0.0f); glVertex3f( points[1].x, points[1].y, points[1].z );
			
			n=lvl.getNormal(tri.points[2].x, tri.points[2].z); glNormal3f(n.x.getFloat(), n.y.getFloat(), n.z.getFloat());
			glTexCoord2f(1.f, 1.0f); glVertex3f( points[2].x, points[2].y, points[2].z );
			glEnd();
			
			++TRIANGLES_DRAWN_THIS_FRAME;
			
		}
	}

	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	glUseProgram(0);
}

void Graphics::drawDebugLines()
{
	glColor3f(1.0f, 1.0f, 0.0f);
	for(size_t i = 0; i < LINES.size(); ++i)
	{
		Vec3& p1 = LINES[i].first;
		Vec3& p2 = LINES[i].second;
		glBegin(GL_LINES);
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
		glEnd();
	}

	glPointSize(5.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	for(size_t i = 0; i < DOTS.size(); ++i)
	{
		Vec3& p1 = DOTS[i];
		glBegin(GL_POINTS);
		glVertex3f(p1.x, p1.y, p1.z);
		glEnd();
	}
}

void Graphics::drawModels(map<int, Model*>& models)
{
	glUseProgram(shaders["unit_program"]);
	for(map<int, Model*>::iterator iter = models.begin(); iter != models.end(); ++iter)
	{
		Model& model = *iter->second;

		glUniform4f(unit_color_location, 0.7, 0.7, 0.7, 0.5);
		
		if(frustum.sphereInFrustum(model.currentModelPos, 5) != FrustumR::OUTSIDE)
		{
			glTranslatef(model.currentModelPos.x, model.currentModelPos.y - modelGround(model), model.currentModelPos.z);
			model.draw();
			glTranslatef(-model.currentModelPos.x, -model.currentModelPos.y + modelGround(model), -model.currentModelPos.z);
		}
	}
	glUseProgram(0);
}

void Graphics::drawParticles()
{
	Vec3 direction_vector = camera.getCurrentTarget() - camera.getPosition();
	depthSortParticles(direction_vector);
	
	TextureHandler::getSingleton().bindTexture("particle");
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glPushMatrix();
	
	for(size_t i = 0; i < viewParticles.size(); ++i)
	{
		float px = viewParticles[i].pos.x.getFloat();
		float py = viewParticles[i].pos.y.getFloat();
		float pz = viewParticles[i].pos.z.getFloat();
		
		glColor4f(viewParticles[i].r, viewParticles[i].g, viewParticles[i].b, viewParticles[i].getAlpha());
		
		float x_angle = camera.getXrot();
		float y_angle = camera.getYrot() + 90.0f;
		
		glTranslatef(px, py, pz);
		
		glRotatef(x_angle, 0.0, 1.0, 0.0);
		glRotatef(y_angle, 1.0, 0.0, 0.0);
		
		float s = viewParticles[i].scale;
		
		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.f); glVertex3f(-1.5f * s, -1.5f * s, 0.0f);
		glTexCoord2f(1.f, 0.f); glVertex3f(+1.5f * s, -1.5f * s, 0.0f);
		glTexCoord2f(1.f, 1.f); glVertex3f(+1.5f * s, +1.5f * s, 0.0f);
		glTexCoord2f(0.f, 1.f); glVertex3f(-1.5f * s, +1.5f * s, 0.0f);
		glEnd();
		++QUADS_DRAWN_THIS_FRAME;
		
		glRotatef(-y_angle, 1.0, 0.0, 0.0);
		glRotatef(-x_angle, 0.0, 1.0, 0.0);
		
		glTranslatef(-px, -py, -pz);
	}
	
	glPopMatrix();
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glColor4f(1.0, 1.0, 1.0, 1.0);

	if(lightsActive)
		glDisable(GL_LIGHTING);
	
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
	camTarget = camera.getCurrentTarget();

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

void Graphics::draw(map<int, Model*>& models, const Level& lvl, const std::map<int,Unit>& units,
	const std::map<int, LightObject>& lights, const std::shared_ptr<Octree> o,
	const std::map<int, Medikit>& medikits)
{
	updateCamera(lvl);

	startDrawing();

	drawLevel(lvl, lights);
//	drawDebugHeightDots(lvl);
	drawDebugLines();
	drawMedikits(medikits);
	drawBoundingBoxes(units);
	drawModels(models);
	drawParticles();
	drawOctree(o);
	drawHUD();

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

void Graphics::draw(std::map<int, Model*>& models, const std::string& status_message)
{
	startDrawing();
	drawDebugLines();
	drawModels(models);
	drawMessages();
	drawString(status_message, -0.9, 0.9, 1.5, true);
	finishDrawing();
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
		viewParticles[i].decrementLife();
		if(!viewParticles[i].alive())
		{
			viewParticles[i] = viewParticles.back();
			viewParticles.pop_back();
			i--;
			continue;
		}
		viewParticles[i].tick();
	}
}

void Graphics::world_tick()
{
	// Don't draw anything here!
	++world_ticks;
	updateParticles();
}

void Graphics::tick()
{
	camera.tick();

	GLfloat lightPos[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	Vec3 camPos = camera.getPosition();

	lightPos[0] = camPos.x; //modelLocation.x.getFloat();
	lightPos[1] = camPos.y + 4; //modelLocation.y.getFloat();
	lightPos[2] = camPos.z; //modelLocation.z.getFloat();
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	
	Location position(FixedPoint(0),FixedPoint(5),FixedPoint(0));
	
	Location velocity;
	velocity.x = FixedPoint(100,1000);
	velocity.y = FixedPoint(900,1000);
	velocity.z = FixedPoint(0);
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

void Graphics::setZombiesLeft(int count)
{
	zombieCount = count;
}

void Graphics::drawMinimap()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	glTranslatef(0.0f, 0.0f, -1.0f);

	glTranslatef(0.78f, -0.78f, 0.0f);
	glRotatef(-camera.getXrot(), 0.0f, 0.0f, 1.0f);
	glTranslatef(-0.78f, 0.78f, 0.0f);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glColor4f(0.3f, 0.3f, 0.3f, 0.5f);
	glBegin(GL_QUADS);
	glVertex3f(0.60f, -0.96f, 0.f);
	glVertex3f(0.96f, -0.96f, 0.f);
	glVertex3f(0.96f, -0.60f, 0.f);
	glVertex3f(0.60f, -0.60f, 0.f);
	glEnd();
	
	glPointSize(4.0f);

	glBegin(GL_POINTS);
	for(std::vector<Location>::const_iterator iter = humanPositions.begin(); iter != humanPositions.end(); ++iter)
	{
		const Location& loc = *iter;
		const Location& unitPos = camera.getUnitPosition();
		if(loc == unitPos)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		else
		{
			glColor3f(0.0f, 0.0f, 1.0f);
		}
		glVertex3f(0.96f - (0.37*loc.x.getFloat())/800.0f, -0.96f + (0.37*loc.z.getFloat())/800.0f, 0.f);
	}
	glEnd();
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	if(lightsActive)
		glEnable(GL_LIGHTING);
}

void Graphics::setHumanPositions(const std::vector<Location>& positions)
{
	humanPositions = positions;
}

void Graphics::drawBox(const Location& top, const Location& bot,
	GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
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
	for(map<int, Unit>::const_iterator iter = units.begin(); iter != units.end(); iter++)
	{
		const Unit& u = iter->second;
		drawBox(u.bb_top(), u.bb_bot());
	}
}

