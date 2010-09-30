
#include "graphics.h"
#include "level.h"

#include <iostream>

using namespace std;


float Graphics::modelGround(Model& model)
{
  // :G
  
  return -2.f;
}



void Graphics::megaFuck()
{
  ObjectPart& leg = objects["LEG"];
  ObjectPart& foot = objects["FOOT"];
  ObjectPart& arm = objects["ARM"];
  
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

}


Graphics::Graphics()
{
  init();
}

void Graphics::init()
{
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
    
    // these could be stored and set somewhere else possibly
    float angle = 45.f;
    float ratio = 800.f / 600.f;
    float nearP = 1.f;
    float farP  = 100.f;
    
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
}

GLuint Graphics::buildTexture(Image& img)
{
  if(img.data == 0)
  {
    cerr << "ERROR: Trying to build texture of image pointer -> 0" << endl;
    return -1;
  }
  
  textures.push_back(0);
  
  glGenTextures(1, &(textures.back()));
  glBindTexture(GL_TEXTURE_2D, textures.back());   // 2d texture (x and y size)

  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture

  // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image, 
  // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
  glTexImage2D(GL_TEXTURE_2D, 0, 3, img.sizeX, img.sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
  
  free(img.data);
  img.data = 0;
  
  return textures.back();
}

void Graphics::deleteTexture(unsigned texture)
{
  for(size_t i=0; i<textures.size(); i++)
  {
    if(textures[i] == texture)
    {
      // this changes the order they are stored in the vector, is it ok? probably yes.
      textures[i] = textures.back();
      textures.pop_back();
    }
  }
  
  glDeleteTextures(1, &texture);
  return;
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

#include <cmath>

void rotateCamera(const Camera& camera)
{
	double yaw = camera.getYaw();
//	cout << "YAW: " << yaw << "\n";
	GLdouble yaw_matrix[16] = {
		cos(yaw), -sin(yaw), 0, 0,
		sin(yaw), cos(yaw), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1 };

	double pitch = camera.getPitch();
//	cout << "PITCH: " << pitch << "\n";
	GLdouble pitch_matrix[16] = {
		cos(pitch), 0, sin(pitch), 0,
		0, 1, 0, 0,
		-sin(pitch), 0, cos(pitch), 0,
		0, 0, 0, 1 };

	double roll = camera.getRoll();
//	cout << "ROLL: " << roll << "\n";
	GLdouble roll_matrix[16] = {
		1, 0, 0, 0,
		0, cos(roll), -sin(roll), 0,
		0, sin(roll), cos(roll), 0,
		0, 0, 0, 1 };

	glMultTransposeMatrixd(yaw_matrix);
	glMultTransposeMatrixd(pitch_matrix);
	glMultTransposeMatrixd(roll_matrix);
}

void Graphics::draw(vector<Model>& models, Level& lvl)
{
    
    glMatrixMode(GL_MODELVIEW);
    
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
    glLoadIdentity();                                   // Reset The View
    
    Vec3 camPos, camTarget, upVector;
	camPos = camera.getPosition();
    
    camTarget.x = camera.getTargetX();
    camTarget.y = camera.getTargetY();
    camTarget.z = camera.getTargetZ();
    
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
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    
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
	float h_diff = lvl.estimateHeightDifference(x, y);
	if(frustum.sphereInFrustum(semiAverage, h_diff + multiplier * 1.f) != FrustumR::OUTSIDE)
	{
	  if(h_diff < 3500)
	    glBindTexture(GL_TEXTURE_2D, textures[0]);
	  else if(h_diff < 10000)
	    glBindTexture(GL_TEXTURE_2D, textures[1]);
	  else
	    glBindTexture(GL_TEXTURE_2D, textures[2]);
	    
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

    
    for(size_t i=0; i<models.size(); i++)
    {
      if(models[i].root < 0)
	continue;      
      
      glTranslatef(0.0f, -modelGround(models[i]), 0.0f);
      drawPartsRecursive(models[i], models[i].root, -1, models[i].animation_name, models[i].animation_time);
      glTranslatef(0.0f, +modelGround(models[i]), 0.0f);
      
    }
    
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
	if(keystate & 32)
	{
		camera.setYaw(camera.getYaw() + 0.05);
	}
	if(keystate & 64)
	{
		camera.setYaw(camera.getYaw() - 0.05);
	}
	if(keystate & 128)
	{
		camera.setPitch(camera.pitch + 0.05);
	}
	if(keystate & 256)
	{
		camera.setPitch(camera.pitch - 0.05);
	}
	if(keystate & 512)
	{
		camera.setRoll(camera.getRoll() + 0.05);
	}
	if(keystate & 1024)
	{
		camera.setRoll(camera.getRoll() - 0.05);
	}
	if(keystate & 1 << 11)
	{
		camera.position = Vec3();
		camera.setYaw(0.0);
		camera.setPitch(0.0);
		camera.setRoll(0.0);
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
		camera.setMode(Camera::STATIC);
	}
}

void Graphics::bindCamera(Unit* unit)
{
	camera.bind(unit, Camera::RELATIVE);
}

