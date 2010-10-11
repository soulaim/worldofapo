
#include "texturehandler.h"
#include "graphics.h"
#include "level.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <algorithm>

using namespace std;

vector<pair<Location,Location> > LINES;

float Graphics::modelGround(const Model& model)
{
	// :G
	
	return -2.f;
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
	lightsActive = true;
	GLfloat	global_ambient[ 4 ]	= {0.05f, 0.05f,  0.05f, 1.0f};
	GLfloat	light0ambient[ 4 ]	= {1.0f, 1.0f,  1.0f, 1.0f};
	GLfloat	light0diffuse[ 4 ]	= {1.0f, 1.0f,  1.0f, 1.0f};
	GLfloat	light0specular[ 4 ]	= {1.0f, 1.0f,  1.0f, 1.0f};
	
	glClearColor(0.0f,0.0f,0.0f,0.5f);
	glClearDepth(1.0f);
	
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.1f);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0specular);
	
	// lights should be defined somewhere else maybe. hard to say.
	glEnable(GL_LIGHT0);
}

void Graphics::toggleLightingStatus()
{
	if(lightsActive)
		glDisable(GL_LIGHTING);
	else
		glEnable(GL_LIGHTING);
	lightsActive = !lightsActive;
}

void Graphics::genParticles(const Location& position, const Location& velocity, int num, float max_rand, float scale, float r, float g, float b)
{
	for(int i=0; i<num; i++)
	{
		Particle p;
		p.pos = position;
		p.vel = velocity;
		
		p.vel.x.number += ((rand() % 1000) - 500) * max_rand;
		p.vel.y.number += ((rand() % 1000) - 500) * max_rand;
		p.vel.z.number += ((rand() % 1000) - 500) * max_rand;
		p.max_life = 70;
		p.cur_life = 70;
		
		p.r = r;
		p.g = g;
		p.b = b;
		
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
		glColor4f(1.0f, 1.0f, 1.0f, 1.f);
		glEnable(GL_TEXTURE_2D);
	}
	
	float currentWidth = 0.f;
	float lastWidth    = 0.f;
	
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
	ObjectPart& leg = objects["LEG"];
	ObjectPart& foot = objects["FOOT"];
	ObjectPart& arm = objects["ARM"];
	ObjectPart& hip = objects["HIP"];
	
	int num_frames = 3;
	
	// walking, step #1
	for(int i=0; i<num_frames; i++)
	{
		leg.animations["walk"].insertAnimationState(0.f, -30.f * i / num_frames, 0.f);
		foot.animations["walk"].insertAnimationState(0.f, 0.f, 0.f);
	}
	
	// walking, step #2
	for(int i=0; i< 2 * num_frames; i++)
	{
		leg.animations["walk"].insertAnimationState(0.f, -30.f + 30.f * i / (2*num_frames), 0.f);
		foot.animations["walk"].insertAnimationState(0.f, +30.f * i / (2*num_frames), 0.f);
	}
	
	// walking, step #3
	for(int i=0; i<num_frames; i++)
	{
		leg.animations["walk"].insertAnimationState(0.f, +40.f * i / num_frames, 0.f);
		foot.animations["walk"].insertAnimationState(0.f, +30.f, 0.f);
	}
	
	// walking, step #4
	for(int i=0; i<num_frames; i++)
	{
		leg.animations["walk"].insertAnimationState(0.f, +40.f - 40.f * i / num_frames, 0.f);
		foot.animations["walk"].insertAnimationState(0.f, +30.f - 30.f * i / num_frames, 0.f);
	}
	
	// there you go! now you know how to walk :D
	
	leg.animations["idle"].insertAnimationState(0.f, 0.f, 0.f);
	foot.animations["idle"].insertAnimationState(0.f, 0.f, 0.f);
	arm.animations["walk"].insertAnimationState(0.f, 0.f, 0.f);
	
	// when idle, SWING YOUR ARMS AROUND WILDLY :DD
	for(int i=0; i<4*num_frames; i++)
		arm.animations["idle"].insertAnimationState(0.f, 360.f * i / (4 * num_frames), 0.f);
	
	for(int i=0; i<4 * num_frames; i++)
		hip.animations["jump"].insertAnimationState(0.f, 360.f * i / (4 * num_frames), 0.f);
	
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
	charWidth['O'] = 0.24;
	
	createWindow(); // let SDL handle this part..
	
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

void Graphics::drawPartsRecursive(Model& model, int current_node, int prev_node, const string& animation, int animation_state)
{
	if(current_node < 0 || size_t(current_node) >= model.parts.size())
	{
		return;
	}
	glTranslatef(model.parts[current_node].offset_x, model.parts[current_node].offset_y, model.parts[current_node].offset_z);
	
	ObjectPart& obj_part = objects[model.parts[current_node].wireframe];
	
	// left and right sides of the body are in polarized animation states
	int ani_addition = 0;
	if(model.parts[current_node].name.substr(0, 4) == "LEFT")
		ani_addition = obj_part.animations[animation].getSize() / 2;
	
	obj_part.animations[animation].getAnimationState(animation_state + ani_addition, model.parts[current_node].rotation_x, model.parts[current_node].rotation_y, model.parts[current_node].rotation_z);
	
	
	glRotatef(model.parts[current_node].rotation_x, 0, 1, 0);
	glRotatef(model.parts[current_node].rotation_y, 1, 0, 0);
	glRotatef(model.parts[current_node].rotation_z, 0, 0, 1);
	
	
	glBegin(GL_TRIANGLES);
	for(size_t i=0; i<obj_part.triangles.size(); i++)
	{
		// how to choose textures??
		ObjectTri& tri = obj_part.triangles[i];
		glColor3f(1.0f, 1.0f, 1.0f); glVertex3f(tri.x[0], tri.y[0], tri.z[0]);
		glColor3f(1.0f, 1.0f, 1.0f); glVertex3f(tri.x[1], tri.y[1], tri.z[1]);
		glColor3f(1.0f, 1.0f, 1.0f); glVertex3f(tri.x[2], tri.y[2], tri.z[2]);
	}
	glEnd();
	
	glTranslatef(obj_part.end_x, obj_part.end_y, obj_part.end_z);
	for(size_t i=0; i<model.parts[current_node].children.size(); i++)
		drawPartsRecursive(model, model.parts[current_node].children[i], current_node, animation, animation_state);
	glTranslatef(-obj_part.end_x, -obj_part.end_y, -obj_part.end_z);
	
	
	// restore rotations
	glRotatef(-model.parts[current_node].rotation_z, 0, 0, 1);
	glRotatef(-model.parts[current_node].rotation_y, 1, 0, 0);
	glRotatef(-model.parts[current_node].rotation_x, 0, 1, 0);
	
	glTranslatef(-model.parts[current_node].offset_x, -model.parts[current_node].offset_y, -model.parts[current_node].offset_z);
}

void Graphics::setCamera(const Camera& cam)
{
	camera = cam;
}

void Graphics::drawLevel(const Level& lvl)
{
	glEnable(GL_TEXTURE_2D);
	// TextureHandler::getSingleton().bindTexture("grass");
	
	int multiplier = 8;
	
	Vec3 semiAverage;
	
	const int textures_size = 3;
	const char* textures[textures_size] = { "grass", "highground", "mountain" };
	for(int i = 0; i < textures_size; ++i)
	{
		// Changing the active texture is avoided by drawing similar coordinates together.
		TextureHandler::getSingleton().bindTexture(textures[i]);

		for(size_t x=0; x < lvl.pointheight_info.size()-1; x++)
		{
			for(size_t y=0; y < lvl.pointheight_info[x].size()-1; y++)
			{
				// These estimates ARE PRECALCULATED! The environment can still be changed dynamically with lvl.updateHeight(x, y, h);
				FixedPoint h_diff = lvl.getHeightDifference(x, y);
				int lololol = -1;
				if(h_diff < FixedPoint(35, 10))
					lololol = 0;
				else if(h_diff < FixedPoint(10))
					lololol = 1;
				else
					lololol = 2;
				
				if(lololol != i)
					continue;
				
				FixedPoint fpx(x);
				FixedPoint fpy(y);
				fpx += FixedPoint(1,2);
				fpy += FixedPoint(1,2);
				fpx *= FixedPoint(multiplier);
				fpy *= FixedPoint(multiplier);

				semiAverage.x = multiplier * (x + 0.5f);
				semiAverage.z = multiplier * (y + 0.5f);
				semiAverage.y = lvl.getHeight(fpx, fpy).getFloat();
				
				Location n = lvl.getNormal(x, y);
				glNormal3f(-n.x.getFloat(), n.y.getFloat(), -n.z.getFloat());
				
				if(frustum.sphereInFrustum(semiAverage, h_diff.getFloat() + multiplier * 1.f) != FrustumR::OUTSIDE)
				{
					Vec3 A(multiplier * x, lvl.pointheight_info[x][y].getFloat(), multiplier * y);
					Vec3 B(multiplier * (x+1), lvl.pointheight_info[x+1][y].getFloat(), multiplier * y);
					Vec3 C(multiplier * (x+1) , lvl.pointheight_info[x+1][y+1].getFloat(), multiplier * (y+1));
					Vec3 D(multiplier * (x)   , lvl.pointheight_info[x][y+1].getFloat()  , multiplier * (y+1));
					
					glBegin(GL_QUADS);
					glTexCoord2f(0.f, 0.0f); glVertex3f( A.x, A.y, A.z );
					glTexCoord2f(1.f, 0.0f); glVertex3f( B.x, B.y, B.z );
					glTexCoord2f(1.f, 1.0f); glVertex3f( C.x, C.y, C.z );
					glTexCoord2f(0.f, 1.0f); glVertex3f( D.x, D.y, D.z );
					glEnd();
				}
			}
		}
	}
	
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
}

void Graphics::drawDebugLines()
{
	for(size_t i = 0; i < LINES.size(); ++i)
	{
		Location& p1 = LINES[i].first;
		Location& p2 = LINES[i].second;
		glBegin(GL_LINES);
		glVertex3f(p1.x.getFloat(), p1.y.getFloat(), p1.z.getFloat());
		glVertex3f(p2.x.getFloat(), p2.y.getFloat(), p2.z.getFloat());
		glEnd();
	}
}

void Graphics::drawModels(map<int, Model>& models)
{
	for(map<int, Model>::iterator iter = models.begin(); iter != models.end(); ++iter)
	{
		if(iter->second.root < 0)
		{
			cerr << "ERROR: There exists a Model descriptor which is empty! (not drawing it)" << endl;
			continue;
		}

		glTranslatef(iter->second.currentModelPos.x, iter->second.currentModelPos.y - modelGround(iter->second), iter->second.currentModelPos.z);
		drawPartsRecursive(iter->second, iter->second.root, -1, iter->second.animation_name, iter->second.animation_time);
		glTranslatef(-iter->second.currentModelPos.x, -iter->second.currentModelPos.y + modelGround(iter->second), -iter->second.currentModelPos.z);		
	}
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
}

void Graphics::draw(map<int, Model>& models, const Level& lvl, const std::map<int,Unit>& units)
{
	updateCamera(lvl);

	startDrawing();

	drawLevel(lvl);
	drawDebugLines();
	drawHitboxes(units);
	drawModels(models);
	drawParticles();
	drawHUD();

	SDL_GL_SwapBuffers();
}

void Graphics::draw(std::map<int, Model>& models, const std::string& status_message)
{
	startDrawing();
	drawModels(models);
	drawMessages();
	drawString(status_message, -0.9, 0.9, 1.5, true);
	SDL_GL_SwapBuffers();
}

void Graphics::updateInput(int keystate, int mousex, int mousey)
{
	camera.updateInput(keystate, mousex, mousey);
}

void Graphics::bindCamera(Unit* unit)
{
	camera.bind(unit, Camera::RELATIVE);
}

void Graphics::world_tick()
{
	// Don't draw anything here!
	++world_ticks;
}

void Graphics::tick()
{
	camera.tick();

	GLfloat lightPos[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	Vec3 camPos = camera.getPosition();
	lightPos[0] = camPos.x;
	lightPos[1] = camPos.y;
	lightPos[2] = camPos.z;
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	
	Location position(FixedPoint(0),FixedPoint(5),FixedPoint(0));
	
	Location velocity;
	velocity.x.number = 100;
	velocity.y.number = 900;
	velocity.z.number = 0;

	
	genParticles(position, velocity, 1, 0.5, 1.0, 0.9, 0.2, 0.2);
	
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

void Graphics::drawHitboxes(const std::map<int,Unit>& units) {
	for(map<int, Unit>::const_iterator iter = units.begin(); iter != units.end(); iter++)
	{
		const Unit& u = iter->second;
		const Location& top = u.hitbox_top();
		const Location& bot = u.hitbox_bot();

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
}

