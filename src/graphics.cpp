
#include "texturehandler.h"
#include "graphics.h"
#include "level.h"

#include <iostream>
#include <vector>
#include <utility>

using namespace std;

vector<pair<Location,Location> > LINES;

float Graphics::modelGround(Model& model)
{
	// :G
	
	return -2.f;
}


void Graphics::pushMessage(string msg, float r, float g, float b)
{
	viewMessages.push_back(ViewMessage(msg, currentTime));
}

void Graphics::setTime(unsigned time)
{
	currentTime = time;
}

void Graphics::drawMessages()
{
	for(int i=0; i<viewMessages.size(); i++)
	{
		if(viewMessages[i].endTime < currentTime)
		{
			for(int k=i+1; k<viewMessages.size(); k++)
				viewMessages[k-1] = viewMessages[k];
			viewMessages.pop_back();
			
			i--;
			continue;
		}
		
		float pos_x = -0.9;
		float pos_y = -0.82 + 0.08 * i;
		drawString(viewMessages[i].msgContent, pos_x, pos_y, viewMessages[i].scale, viewMessages[i].hilight);
	}
	
	if(currentClientCommand.size() > 0)
		drawString(currentClientCommand, -0.9, -0.9, 1.3, true); 
}

void Graphics::drawString(const string& msg, float pos_x, float pos_y, float scale, bool background)
{
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
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	
	float totalWidth = 0.025f;
	for(int i=0; i<msg.size(); i++)
		totalWidth += 0.05 * charWidth[msg[i]] * 2 * scale;
	
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
		glVertex3f(pos_x             , y_bot, -1);
		glVertex3f(pos_x + totalWidth, y_bot, -1);
		glVertex3f(pos_x + totalWidth, y_top, -1);
		glVertex3f(pos_x             , y_top, -1);
		glEnd();
		glColor4f(1.0f, 1.0f, 1.0f, 1.f);
		glEnable(GL_TEXTURE_2D);
	}
	
	float currentWidth = 0.f;
	float lastWidth    = 0.f;
	
	for(int i=0; i<msg.size(); i++)
	{
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
	
//	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_BLEND);
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
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
	init();
}

void Graphics::setCurrentClientCommand(string& cmd)
{
	currentClientCommand = cmd;
}

void Graphics::init()
{
	
	charWidth.resize(255, 1.f);
	
	for(char symbol = 'A'; symbol <= 'Z'; symbol++)
		charWidth[symbol] = 0.26;
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
	
	
	createWindow(); // let SDL handle this part..
	
	glEnable(GL_TEXTURE_2D);			// Enable Texture Mapping
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);	// Clear The Background Color To Blue 
	glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);			// The Type Of Depth Test To Do
	glEnable(GL_DEPTH_TEST);			// Enables Depth Testing
	glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				// Reset The Projection Matrix
	
	// do some weird magic i dont understand
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	
	TextureHandler::getSingleton().createTexture("font", "data/fonts/font2.png");
	
	// these could be stored and set somewhere else possibly
	float angle = 45.f;
	float ratio = 800.f / 600.f;
	float nearP = 1.f;
	float farP  = 200.f;
	
	gluPerspective(angle,ratio,nearP,farP);
	frustum.setCamInternals(angle,ratio,nearP,farP);
	
	glMatrixMode(GL_MODELVIEW);
}

void Graphics::createWindow()
{
	SDL_Init(SDL_INIT_EVERYTHING);
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

void Graphics::drawPartsRecursive(Model& model, int current_node, int prev_node, string& animation, int animation_state)
{
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

void Graphics::draw(map<int, Model>& models, Level& lvl)
{
	
	glMatrixMode(GL_MODELVIEW);
	
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
	glLoadIdentity();                                   // Reset The View
	
	camera.tick();
	Vec3 camPos, camTarget, upVector;
	camPos = camera.getPosition();
	
	/*
	camTarget.x = camera.getTargetX();
	camTarget.y = camera.getTargetY();
	camTarget.z = camera.getTargetZ();
	*/
	
	camTarget = camera.getCurrentTarget();
	
	upVector.x = 0.f;
	upVector.y = 1.f;
	upVector.z = 0.f;
	
	glLoadIdentity();
	gluLookAt(camPos.x, camPos.y, camPos.z,
			  camTarget.x, camTarget.y, camTarget.z,
			  upVector.x, upVector.y, upVector.z);
			  
			  frustum.setCamDef(camPos, camTarget, upVector);
			  
			  // this must also be integrated to the camera describing frustum
			  // this must be done with the model of the local player if at all! models[0] is not the hero for all players.
			  //  glRotatef(-models[0].parts[models[0].root].rotation_x + 180.f, 0.f, 1.0f, 0.f); // rotate the camera to players rotation.
	
	glEnable(GL_TEXTURE_2D);
	
	int multiplier = 8;
	
	Vec3 semiAverage;
	for(size_t x=0; x < lvl.pointheight_info.size()-1; x++)
	{
		for(size_t y=0; y < lvl.pointheight_info[x].size()-1; y++)
		{
			FixedPoint fpx; fpx.number = multiplier * (1000 * x + 500);
			FixedPoint fpy; fpy.number = multiplier * (1000 * y + 500);
			
			semiAverage.x = multiplier * (x + 0.5f);
			semiAverage.z = multiplier * (y + 0.5f);
			semiAverage.y = lvl.getHeight(fpx, fpy).getFloat();
			
			// this is bubblegum. could maybe test each corner point of the quad.
			float h_diff = lvl.estimateHeightDifference(x, y); // estimates could be precalculated also.
			if(frustum.sphereInFrustum(semiAverage, h_diff + multiplier * 1.f) != FrustumR::OUTSIDE)
			{
				if(h_diff < 3500)
					TextureHandler::getSingleton().bindTexture("grass");
				else if(h_diff < 10000)
					TextureHandler::getSingleton().bindTexture("highground");
				else
					TextureHandler::getSingleton().bindTexture("mountain");
				
				glBegin(GL_QUADS);
				glTexCoord2f(0.f, 0.0f); glVertex3f( multiplier * (x)   , lvl.pointheight_info[x][y].getFloat()    , multiplier * y);
				glTexCoord2f(1.f, 0.0f); glVertex3f( multiplier * (x+1) , lvl.pointheight_info[x+1][y].getFloat()  , multiplier * y);
				glTexCoord2f(1.f, 1.0f); glVertex3f( multiplier * (x+1) , lvl.pointheight_info[x+1][y+1].getFloat(), multiplier * (y+1) );
				glTexCoord2f(0.f, 1.0f); glVertex3f( multiplier * (x)   , lvl.pointheight_info[x][y+1].getFloat()  , multiplier * (y+1) );
				glEnd();
			}
		}
	}
	
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	

	for(int i = 0; i < LINES.size(); ++i)
	{
		Location& p1 = LINES[i].first;
		Location& p2 = LINES[i].second;
		glBegin(GL_LINES);
		glVertex3f(p1.x.getFloat(), p1.h.getFloat(), p1.y.getFloat());
		glVertex3f(p2.x.getFloat(), p2.h.getFloat(), p2.y.getFloat());
		glEnd();
	}
	
	for(map<int, Model>::iterator iter = models.begin(); iter != models.end(); iter++)
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
	
	
//	drawString("Hello world! :D Random TEXT here. Just to see, if it works at all?", -0.9f, -0.7f, 1.5f, true);
//	drawString("Trolololol. Pessi tekee jotai hyodyllista :]]", -0.9f, -0.6f, 1.5f, false);
//	drawString("<Apodus> eiss voivv.. :D", -0.9f, -0.5f, 1.9f, true);
	
	drawMessages();
	
	SDL_GL_SwapBuffers();
	return;
}

void Graphics::updateInput(int keystate, int mousex, int mousey)
{
	/*
	if(keystate & 1)
	{
		Location location = camera.getLocation();
		location.x += 5;
		camera.setLocation(location);
}
if(keystate & 2)
{
	Location location = camera.getLocation();
	location.x -= 5;
	camera.setLocation(location);
}

if(keystate & 4)
{
	Location location = camera.getLocation();
	location.y += 5;
	camera.setLocation(location);
}
if(keystate & 8)
{
	Location location = camera.getLocation();
	location.y -= 5;
	camera.setLocation(location);
}
*/
	if(keystate & 1 << 11)
	{
		camera.position = Vec3();
	}
	
	if(keystate & 1 << 12)
	{
		camera.position.x += 1;
	}
	if(keystate & 1 << 13)
	{
		camera.position.x -= 1;
	}
	if(keystate & 1 << 14)
	{
		camera.position.y += 1;
	}
	if(keystate & 1 << 15)
	{
		camera.position.y -= 1;
	}
	if(keystate & 1 << 16)
	{
		camera.position.z += 1;
	}
	if(keystate & 1 << 17)
	{
		camera.position.z -= 1;
	}
	if(keystate & 1 << 18)
	{
		camera.setMode(Camera::RELATIVE);
	}
	if(keystate & 1 << 19)
	{
		camera.setMode(Camera::FIRST_PERSON);
	}
}

void Graphics::bindCamera(Unit* unit)
{
	camera.bind(unit, Camera::RELATIVE);
}

