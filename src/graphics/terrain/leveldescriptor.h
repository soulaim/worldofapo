
#ifndef H_LEVEL_DESCRIPTOR
#define H_LEVEL_DESCRIPTOR

#include "graphics/opengl.h"
#include "graphics/terrain/btt.h"
#include "graphics/texturecoordinate.h"
#include "misc/hasproperties.h"
#include "misc/vec3.h"

#include <vector>

class Level;
class FrustumR;
class Shaders;

class LevelDescriptor: public HasProperties
{
public:
	LevelDescriptor();

	void setLevel(Level* lvl);

	void drawFR(size_t, Shaders&) const;
	void drawDeferred(Shaders&) const;
	void drawDebugHeightDots(const vec3<float>& location) const;
	void drawDebugLevelNormals() const;
    Level* getLevel() const;

	void world_tick(FrustumR& frustum);

	void preload();
	void unload();

private:
	LevelDescriptor(const LevelDescriptor&); // Disabled.
	LevelDescriptor& operator=(const LevelDescriptor&); // Disabled.

	void drawLevelFR(const Level&, int, Shaders&) const;
	// void drawLevelFR_new(const Level&, int, Shaders&);
	void drawLevel(const Level&, size_t, Shaders&) const;
	void drawLevelDeferred(const Level&, Shaders&) const;

	void drawBuffers() const;

	Level* level;

	bool debugMode;

	// Static data.
	std::vector<vec3<float> > vertices;
	std::vector<vec3<float> > normals;
	std::vector<TextureCoordinate> texture_coordinates1;
	std::vector<TextureCoordinate> texture_coordinates2;
//	std::vector<TextureCoordinate> texture_coordinates3; // All texture coordinates are actually same, so we'll let shader handle the third.

	// Dynamic data.
	mutable std::vector<unsigned> indices; // TODO: remove mutable.

	std::vector<BTT_Triangle> level_triangles;
	BinaryTriangleTree btt;

	enum { BUFFERS = 6 };
	GLuint locations[BUFFERS];
	bool buffers_loaded;
};

#endif

