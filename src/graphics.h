
#include "menubutton.h"
#include "image.h"
#include "location.h"

#include <vector>
#include <map>
#include <string>

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>

#include "model.h"
#include "animation.h"
#include "frustum/FrustumR.h"
#include "camera.h"

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
  
  void init();
  void createWindow();
  float modelGround(Model& model);
  
  SDL_Surface* drawContext;
  Camera camera;
public:

  void bindCamera(Unit* unit);
  void updateInput(int keystate, int mousex, int mousey);
  
  std::map<std::string, ObjectPart> objects;
  void drawPartsRecursive(Model&, int, int, std::string&, int);
  
  void megaFuck(); // this function creates a test animation called "walk" for the test model
  
  void setCamera(const Camera& camera);
  void draw(std::map<int, Model>&, Level& lvl);
  void loadObjects(std::string);
  
  void drawMenu(std::vector<MenuButton>&);
  
  Graphics();
  
  FrustumR frustum;
};

