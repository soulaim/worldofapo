
#include "graphics.h"
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
  
  // walking, step #1
  for(int i=0; i<50; i++)
  {
    leg.animations["walk"].insertAnimationState(0.f, -30.f * i / 50.f, 0.f);
    foot.animations["walk"].insertAnimationState(0.f, 0.f, 0.f);
  }
  
  // walking, step #2
  for(int i=0; i<50; i++)
  {
    leg.animations["walk"].insertAnimationState(0.f, -30.f + 30.f * i / 50.f, 0.f);
    foot.animations["walk"].insertAnimationState(0.f, +30.f * i / 50.f, 0.f);
  }
  
  // walking, step #3
  for(int i=0; i<50; i++)
  {
    leg.animations["walk"].insertAnimationState(0.f, +40.f * i / 50.f, 0.f);
    foot.animations["walk"].insertAnimationState(0.f, +30.f, 0.f);
  }

  // walking, step #4
  for(int i=0; i<50; i++)
  {
    leg.animations["walk"].insertAnimationState(0.f, +40.f - 40.f * i / 50.f, 0.f);
    foot.animations["walk"].insertAnimationState(0.f, +30.f - 30.f * i / 50.f, 0.f);
  }

  // there you go! now you know how to walk :D
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
    
    gluPerspective(45.0f,(GLfloat) 800/(GLfloat) 600,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window
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
  cerr << "drawContext = " << drawContext << endl;
}

void Graphics::buildTexture(Image& img)
{
  if(img.data == 0)
  {
    cerr << "ERROR: Trying to build texture of image pointer -> 0" << endl;
    return;
  }
  
  glGenTextures(1, &(img.id));
  glBindTexture(GL_TEXTURE_2D, img.id);   // 2d texture (x and y size)

  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture

  // 2d texture, level of detail 0 (normal), 3 components (red, green, blue), x size from image, y size from image, 
  // border 0 (normal), rgb color data, unsigned byte data, and finally the data itself.
  glTexImage2D(GL_TEXTURE_2D, 0, 3, img.sizeX, img.sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img.data);
  
  free(img.data);
  img.data = 0;
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
  for(int i=0; i<obj_part.triangles.size(); i++)
  {
    // how to choose textures??
    ObjectTri& tri = obj_part.triangles[i];
    glColor3f(1.0f, 1.0f, 1.0f); glVertex3f(tri.x[0], tri.y[0], tri.z[0]);
    glColor3f(1.0f, 1.0f, 1.0f); glVertex3f(tri.x[1], tri.y[1], tri.z[1]);
    glColor3f(1.0f, 1.0f, 1.0f); glVertex3f(tri.x[2], tri.y[2], tri.z[2]);
  }
  glEnd();
  
  glTranslatef(obj_part.end_x, obj_part.end_y, obj_part.end_z);
  for(int i=0; i<model.parts[current_node].children.size(); i++)
    drawPartsRecursive(model, model.parts[current_node].children[i], current_node, animation, animation_state);
  glTranslatef(-obj_part.end_x, -obj_part.end_y, -obj_part.end_z);
  
  // restore rotations
  glRotatef(-model.parts[current_node].rotation_z, 0, 0, 1);
  glRotatef(-model.parts[current_node].rotation_y, 1, 0, 0);
  glRotatef(-model.parts[current_node].rotation_x, 0, 1, 0);
  
  glTranslatef(-model.parts[current_node].offset_x, -model.parts[current_node].offset_y, -model.parts[current_node].offset_z);
}

void Graphics::setCamera(Location cam)
{
  camera = cam;
}

void Graphics::draw(vector<Model>& models)
{
    
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
    glLoadIdentity();                                   // Reset The View
    
    glTranslatef(-0.0f, -10.0f, -50.0f); // move some units into the screen.    
    glRotatef(60.f, 1, 0, 0);
    glTranslatef(.0f, .0f, -10.0f); // move some units into the screen.    
    glTranslatef(-camera.x.getFloat(), 0, -camera.y.getFloat());
    
    glBegin(GL_TRIANGLES);
    glColor3f(1.0f, 0.0f, 0.0f); glVertex3f(20. , 0, 20);
    glColor3f(0.0f, 0.0f, 1.0f); glVertex3f(-20., 0, 20);
    glColor3f(0.0f, 1.0f, 0.0f); glVertex3f(0.  , 0, -20);
    glEnd();
    
    for(int i=0; i<models.size(); i++)
    {
      if(models[i].root < 0)
	continue;
      drawPartsRecursive(models[i], models[i].root, -1, models[i].animation_name, models[i].animation_time);
    }
    
    SDL_GL_SwapBuffers();
    return;
}
