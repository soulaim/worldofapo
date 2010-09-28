
#include "image.h"
#include "location.h"

#include <vector>
#include <map>
#include <string>

#include <GL/glut.h>    // Header File For The GLUT Library 
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#include <SDL/SDL.h>

#include "model.h"
#include "animation.h"

struct Level;


struct ObjectTri
{
  float x[3];
  float y[3];
  float z[3];
};

struct ObjectQuad
{
  float x[4];
  float y[4];
  float z[4];
};

struct ObjectPart
{
  std::vector<ObjectQuad> quads;
  std::vector<ObjectTri> triangles;
  std::map<std::string, Animation> animations;
  
  float end_x;
  float end_y;
  float end_z;
};

class Graphics
{
  GLfloat m[16]; // storage for OGL ModelView matrix
  std::vector<GLuint> textures;
  
  void init();
  void createWindow();
  float modelGround(Model& model);
  
  SDL_Surface* drawContext;
  Location camera;
public:

  
  std::map<std::string, ObjectPart> objects;
  void drawPartsRecursive(Model&, int, int, std::string&, int);
  
  void megaFuck(); // this function creates a test animation called "walk" for the test model
  
  void setCamera(Location);
  void draw(std::vector<Model>&, Level& lvl);
  void buildTexture(Image&);
  void loadObjects(std::string);
  Graphics();
};

