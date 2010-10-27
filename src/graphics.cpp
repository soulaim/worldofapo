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





void Graphics::setActiveLights(const map<int, LightObject>& lightsContainer, const Location& pos)
{
	vector<int> indexes;
	vector<FixedPoint> distances;
	
	distances.resize(2, FixedPoint(40000));
	indexes.resize(2, -1);
	
	for(auto iter = lightsContainer.begin(); iter != lightsContainer.end(); iter++)
	{
		const LightObject& light = iter->second;
		FixedPoint lightDistance = (light.position - pos).lengthSquared();
		
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

void Graphics::drawLevel(const Level& lvl, const map<int, LightObject>& lightsContainer)
{
	if(lightsActive)
	{
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
	glUseProgram(shaders["level_program"]);
	//glEnable(GL_TEXTURE_2D);
	TextureHandler::getSingleton().bindTexture("grass");
	
	int multiplier = 8;
	
	Vec3 semiAverage;
	Vec3 points[3];

	glBegin(GL_TRIANGLES);
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
		
		Location n;
		
		// set active lights
		Location pos;
		pos.x = int(semiAverage.x);
		pos.y = int(semiAverage.x);
		pos.z = int(semiAverage.x);
		
		setActiveLights(lightsContainer, pos);
		
		
		// TODO: Terrain texture
		n=lvl.getNormal(tri.points[0].x, tri.points[0].z); glNormal3f(n.x.getFloat(), n.y.getFloat(), n.z.getFloat());
		glTexCoord2f(0.f, 0.0f); glVertex3f( points[0].x, points[0].y, points[0].z );
		
		n=lvl.getNormal(tri.points[1].x, tri.points[1].z); glNormal3f(n.x.getFloat(), n.y.getFloat(), n.z.getFloat());
		glTexCoord2f(1.f, 0.0f); glVertex3f( points[1].x, points[1].y, points[1].z );
		
		n=lvl.getNormal(tri.points[2].x, tri.points[2].z); glNormal3f(n.x.getFloat(), n.y.getFloat(), n.z.getFloat());
		glTexCoord2f(1.f, 1.0f); glVertex3f( points[2].x, points[2].y, points[2].z );
		
		++TRIANGLES_DRAWN_THIS_FRAME;
		
	}
	glEnd();

//	glDisable(GL_TEXTURE_2D);
	
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
			glTranslatef(model.currentModelPos.x, model.currentModelPos.y - modelGround(model), model.currentModelPos.z);
			model.draw();
			glTranslatef(-model.currentModelPos.x, -model.currentModelPos.y + modelGround(model), -model.currentModelPos.z);
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
		glTexCoord2f(0.f, 1.f); glVertex3f(-1.5f * s, +1.5f * s, 0.0f);
		glTexCoord2f(1.f, 1.f); glVertex3f(+1.5f * s, +1.5f * s, 0.0f);
		glTexCoord2f(1.f, 0.f); glVertex3f(+1.5f * s, -1.5f * s, 0.0f);
		glTexCoord2f(0.f, 0.f); glVertex3f(-1.5f * s, -1.5f * s, 0.0f);
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

void Graphics::draw(map<int, Model*>& models, const Level& lvl, const std::map<int,Unit>& units, const std::map<int, LightObject>& lights, const std::shared_ptr<Octree> o, Hud* hud)
{
	updateCamera(lvl);

	startDrawing();

	if(drawDebuglines)
	{
		drawDebugLevelNormals(lvl);
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

void Graphics::world_tick(Level& lvl)
{
	// Don't draw anything here!
	updateParticles();
	
	Location pos;
	
	// position and frustum
	lvl.splitBTT(pos, frustum);
	
	level_triangles.clear();
	lvl.btt.getTriangles(level_triangles);
	// cerr << "total triangles: " << level_triangles.size() << endl;
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
	for(map<int, Unit>::const_iterator iter = units.begin(); iter != units.end(); iter++)
	{
		const Unit& u = iter->second;
		drawBox(u.bb_top(), u.bb_bot());
	}
}

