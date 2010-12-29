
#ifndef H_LEVEL_DESCRIPTOR
#define H_LEVEL_DESCRIPTOR

#include "btt.h"
#include "hasproperties.h"

class Level;
class FrustumR;

class LevelDescriptor : public HasProperties
{
	public:
	
	LevelDescriptor();
	
	void setLevel(Level* lvl);
	
	void drawFR(size_t, Shaders&) const;
	void drawDeferred(Shaders&) const;
	
	void drawLevelFR(const Level&, int, Shaders&) const;
	// void drawLevelFR_new(const Level&, int, Shaders&);
	void drawLevel(const Level&, size_t, Shaders&) const;
	void drawLevelDeferred(const Level&, Shaders&) const;
	
	void drawDebugHeightDots(const Level& lvl) const;
	void drawDebugLevelNormals(const Level& lvl) const;
	
	void world_tick(FrustumR& frustum);
	
	Level* level;
	
	bool debugMode;
	
	std::vector<BTT_Triangle> level_triangles;
};

#endif

